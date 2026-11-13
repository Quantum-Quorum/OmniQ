/*
 * OmniQ-QOS: User-space C POSIX Interface Implementation
 * src/lib/libomniq_qpu.cpp
 *
 * Copyright (c) 2026 Quantum Quorum
 */

#include "../../include/omniq/qpu.h"
#include "../driver/VirtualQPUDriver.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#ifdef __linux__
#include <sys/ioctl.h>
#endif

struct qpu_device {
    int fd;
    char path[256];
    bool isDirectMode;
    std::unique_ptr<omniq::qos::VirtualQPUDriver> directDriver;
};

extern "C" {

qpu_device_t *qpu_open(const char *dev_path) {
    auto *dev = new qpu_device();
    dev->fd = -1;
    dev->isDirectMode = false;

    if (!dev_path || std::strcmp(dev_path, ":direct:") == 0 ||
        std::strcmp(dev_path, "default") == 0) {
        dev->isDirectMode = true;
        dev->directDriver = std::make_unique<omniq::qos::VirtualQPUDriver>(32);
        std::strncpy(dev->path, ":direct:", sizeof(dev->path) - 1);
        return dev;
    }

    std::strncpy(dev->path, dev_path, sizeof(dev->path) - 1);

    // Try opening real device or fallback to direct mode
    int fd = open(dev_path, O_RDWR);
    if (fd >= 0) {
        dev->fd = fd;
        dev->isDirectMode = false;
    } else {
        // Fallback to high-performance direct virtual driver
        dev->isDirectMode = true;
        dev->directDriver = std::make_unique<omniq::qos::VirtualQPUDriver>(32);
    }
    return dev;
}

void qpu_close(qpu_device_t *dev) {
    if (!dev) return;
    if (dev->fd >= 0) {
        close(dev->fd);
        dev->fd = -1;
    }
    delete dev;
}

int qpu_get_topology(qpu_device_t *dev, struct qpu_topology_query *topo,
                     struct qpu_coupler_edge *edges, uint32_t max_edges) {
    if (!dev || !topo) return -22;
    topo->max_edges_capacity = max_edges;
    topo->edges_buffer_ptr = reinterpret_cast<uint64_t>(edges);

    if (dev->isDirectMode && dev->directDriver) {
        return dev->directDriver->ioctlQueryTopology(topo, edges);
    }

#ifdef __linux__
    if (dev->fd >= 0) {
        return ioctl(dev->fd, QPU_IOCTL_QUERY_TOPOLOGY, topo);
    }
#endif
    return -9; // -EBADF
}

int qpu_get_calibration(qpu_device_t *dev, struct qpu_qubit_calibration *cals,
                        uint32_t max_qubits, uint64_t *timestamp_out) {
    if (!dev || !cals) return -22;

    struct qpu_calibration_query query;
    std::memset(&query, 0, sizeof(query));
    query.num_qubits = max_qubits;
    query.calibration_buffer_ptr = reinterpret_cast<uint64_t>(cals);

    int ret = 0;
    if (dev->isDirectMode && dev->directDriver) {
        ret = dev->directDriver->ioctlQueryCalibration(&query, cals);
    } else {
#ifdef __linux__
        if (dev->fd >= 0) {
            ret = ioctl(dev->fd, QPU_IOCTL_QUERY_CALIBRATION, &query);
        } else {
            ret = -9;
        }
#else
        ret = -38; // -ENOSYS
#endif
    }

    if (ret == 0 && timestamp_out) {
        *timestamp_out = query.timestamp_ns;
    }
    return ret;
}

int qpu_alloc(qpu_device_t *dev, uint32_t num_qubits, uint32_t flags,
              uint32_t *out_partition_id) {
    if (!dev || !out_partition_id) return -22;

    struct qpu_alloc_req req;
    std::memset(&req, 0, sizeof(req));
    req.num_virtual_qubits = num_qubits;
    req.flags = flags;

    int ret = 0;
    if (dev->isDirectMode && dev->directDriver) {
        ret = dev->directDriver->ioctlAllocQubits(&req, static_cast<uint32_t>(getpid()));
    } else {
#ifdef __linux__
        if (dev->fd >= 0) {
            ret = ioctl(dev->fd, QPU_IOCTL_ALLOC_QUBITS, &req);
        } else {
            ret = -9;
        }
#else
        ret = -38;
#endif
    }

    if (ret == 0) {
        *out_partition_id = req.partition_id;
    }
    return ret;
}

int qpu_free(qpu_device_t *dev, uint32_t partition_id) {
    if (!dev) return -22;

    struct qpu_free_req req;
    std::memset(&req, 0, sizeof(req));
    req.partition_id = partition_id;
    req.reset_mode = QPU_JOB_FLAG_ACTIVE_RESET;

    if (dev->isDirectMode && dev->directDriver) {
        return dev->directDriver->ioctlFreeQubits(&req);
    }
#ifdef __linux__
    if (dev->fd >= 0) {
        return ioctl(dev->fd, QPU_IOCTL_FREE_QUBITS, &req);
    }
#endif
    return -9;
}

int qpu_submit(qpu_device_t *dev, uint32_t partition_id, const char *qasm_or_code,
               uint32_t shots, uint32_t flags, uint64_t *out_job_id) {
    if (!dev || !out_job_id) return -22;

    struct qpu_submit_job job;
    std::memset(&job, 0, sizeof(job));
    job.partition_id = partition_id;
    job.shots = shots;
    job.flags = flags;
    std::string code = qasm_or_code ? qasm_or_code : "";
    job.qasm_or_bytecode_ptr = reinterpret_cast<uint64_t>(code.c_str());
    job.bytecode_len = static_cast<uint32_t>(code.length());

    int ret = 0;
    if (dev->isDirectMode && dev->directDriver) {
        ret = dev->directDriver->ioctlSubmitJob(&job, code);
    } else {
#ifdef __linux__
        if (dev->fd >= 0) {
            ret = ioctl(dev->fd, QPU_IOCTL_SUBMIT_JOB, &job);
        } else {
            ret = -9;
        }
#else
        ret = -38;
#endif
    }

    if (ret == 0) {
        *out_job_id = job.job_id;
    }
    return ret;
}

int qpu_wait(qpu_device_t *dev, uint64_t job_id, uint32_t timeout_ms,
             struct qpu_readout_entry *entries, uint32_t max_entries,
             uint32_t *out_num_entries, uint32_t *out_total_shots) {
    if (!dev) return -22;

    struct qpu_result_req res;
    std::memset(&res, 0, sizeof(res));
    res.job_id = job_id;
    res.timeout_ms = timeout_ms;
    res.max_entries_capacity = max_entries;
    res.entries_buffer_ptr = reinterpret_cast<uint64_t>(entries);

    int ret = 0;
    if (dev->isDirectMode && dev->directDriver) {
        ret = dev->directDriver->ioctlReadResults(&res, entries);
    } else {
#ifdef __linux__
        if (dev->fd >= 0) {
            ret = ioctl(dev->fd, QPU_IOCTL_READ_RESULTS, &res);
        } else {
            ret = -9;
        }
#else
        ret = -38;
#endif
    }

    if (ret == 0) {
        if (out_num_entries) *out_num_entries = res.num_unique_outcomes;
        if (out_total_shots) *out_total_shots = res.total_shots_collected;
    }
    return ret;
}

int qpu_run_sync(qpu_device_t *dev, uint32_t num_qubits, const char *qasm_str,
                 uint32_t shots, struct qpu_readout_entry *entries,
                 uint32_t max_entries, uint32_t *out_num_entries) {
    uint32_t partId = 0;
    int ret = qpu_alloc(dev, num_qubits, QPU_ALLOC_FLAG_HIGH_FIDELITY, &partId);
    if (ret != 0) return ret;

    uint64_t jobId = 0;
    ret = qpu_submit(dev, partId, qasm_str, shots, QPU_JOB_FLAG_DYNAMIC_DECOUPLING, &jobId);
    if (ret != 0) {
        qpu_free(dev, partId);
        return ret;
    }

    uint32_t totalShots = 0;
    ret = qpu_wait(dev, jobId, 1000, entries, max_entries, out_num_entries, &totalShots);
    qpu_free(dev, partId);
    return ret;
}

const char *qpu_tech_name(uint32_t tech) {
    switch (tech) {
    case QPU_TECH_SUPERCONDUCTING_TRANSMON:
        return "Superconducting Transmon";
    case QPU_TECH_TRAPPED_ION:
        return "Trapped Ion";
    case QPU_TECH_NEUTRAL_ATOM:
        return "Rydberg Neutral Atom";
    case QPU_TECH_PHOTONIC:
        return "Continuous-Variable / Photonic";
    case QPU_TECH_MAGNON_DUAL_RAIL:
        return "Magnonic Dual-Rail Hybrid";
    default:
        return "Generic QPU Accelerator";
    }
}

} // extern "C"

