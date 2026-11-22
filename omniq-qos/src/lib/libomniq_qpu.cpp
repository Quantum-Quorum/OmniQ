/*
 * OmniQ-QOS: User-space C POSIX Interface Implementation
 * src/lib/libomniq_qpu.cpp
 *
 * Copyright (c) 2026 Quantum Quorum
 */

#include "../../include/omniq/qpu.h"
#include "../../include/omniq/omniqd_ipc.h"
#include "../driver/VirtualQPUDriver.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#ifdef __linux__
#include <sys/ioctl.h>
#endif

struct qpu_device {
    int fd;
    int socket_fd;
    char path[256];
    bool isDirectMode;
    bool isIpcMode;
    std::unique_ptr<omniq::qos::VirtualQPUDriver> directDriver;
};

static inline int omniq_ipc_check_status(int sockFd, const struct omniq_ipc_msg_hdr &respHdr) {
    if (respHdr.status != 0) {
        if (respHdr.payload_len > 0) {
            std::vector<uint8_t> discard(respHdr.payload_len);
            omniq_ipc_recv_all(sockFd, discard.data(), discard.size());
        }
        return respHdr.status;
    }
    return 0;
}

extern "C" {

qpu_device_t *qpu_open(const char *dev_path) {
    auto *dev = new qpu_device();
    dev->fd = -1;
    dev->socket_fd = -1;
    dev->isDirectMode = false;
    dev->isIpcMode = false;

    // 0. Support pre-connected file descriptor (e.g. "fd:3")
    if (dev_path && std::strncmp(dev_path, "fd:", 3) == 0) {
        int sockFd = std::atoi(dev_path + 3);
        if (sockFd >= 0) {
            dev->socket_fd = sockFd;
            dev->isIpcMode = true;
            dev->isDirectMode = false;
            std::strncpy(dev->path, dev_path, sizeof(dev->path) - 1);
            return dev;
        }
    }

    // 1. Explicit force-direct mode (bypasses daemon check)
    if (dev_path && std::strcmp(dev_path, ":force-direct:") == 0) {
        dev->isDirectMode = true;
        dev->directDriver = std::make_unique<omniq::qos::VirtualQPUDriver>(32);
        std::strncpy(dev->path, ":direct:", sizeof(dev->path) - 1);
        return dev;
    }

    // Determine target socket path:
    // If dev_path contains ".sock", use that custom socket path.
    // Otherwise use default OMNIQ_DEFAULT_SOCKET_PATH (/tmp/omniq_qpu0.sock).
    const char *targetSock = OMNIQ_DEFAULT_SOCKET_PATH;
    if (dev_path && std::strstr(dev_path, ".sock") != nullptr) {
        targetSock = dev_path;
    }

    // 2. Try connecting to UNIX domain socket (omniqd)
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock >= 0) {
        struct sockaddr_un addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        std::strncpy(addr.sun_path, targetSock, sizeof(addr.sun_path) - 1);

        if (connect(sock, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) == 0) {
            dev->socket_fd = sock;
            dev->isIpcMode = true;
            dev->isDirectMode = false;
            std::strncpy(dev->path, targetSock, sizeof(dev->path) - 1);
            return dev;
        }
        close(sock);
    }

    // 3. If socket absent, try opening real kernel device node if specified
    if (dev_path && std::strcmp(dev_path, ":direct:") != 0 &&
        std::strcmp(dev_path, "default") != 0 &&
        std::strstr(dev_path, ".sock") == nullptr) {
        int fd = open(dev_path, O_RDWR);
        if (fd >= 0) {
            dev->fd = fd;
            dev->isDirectMode = false;
            dev->isIpcMode = false;
            std::strncpy(dev->path, dev_path, sizeof(dev->path) - 1);
            return dev;
        }
    }

    // 4. Fallback to standalone direct virtual driver (:direct:)
    dev->isDirectMode = true;
    dev->isIpcMode = false;
    dev->directDriver = std::make_unique<omniq::qos::VirtualQPUDriver>(32);
    std::strncpy(dev->path, ":direct:", sizeof(dev->path) - 1);
    return dev;
}

void qpu_close(qpu_device_t *dev) {
    if (!dev) return;
    if (dev->socket_fd >= 0) {
        close(dev->socket_fd);
        dev->socket_fd = -1;
    }
    if (dev->fd >= 0) {
        close(dev->fd);
        dev->fd = -1;
    }
    delete dev;
}

bool qpu_is_ipc(qpu_device_t *dev) {
    return (dev != nullptr) && dev->isIpcMode;
}

int qpu_get_topology(qpu_device_t *dev, struct qpu_topology_query *topo,
                     struct qpu_coupler_edge *edges, uint32_t max_edges) {
    if (!dev || !topo) return -22;
    topo->max_edges_capacity = max_edges;
    topo->edges_buffer_ptr = reinterpret_cast<uint64_t>(edges);

    if (dev->isIpcMode && dev->socket_fd >= 0) {
        struct omniq_ipc_msg_hdr hdr;
        std::memset(&hdr, 0, sizeof(hdr));
        hdr.magic = OMNIQ_IPC_MAGIC;
        hdr.msg_type = OMNIQ_MSG_QUERY_TOPOLOGY;
        hdr.payload_len = sizeof(struct qpu_topology_query);

        if (!omniq_ipc_send_all(dev->socket_fd, &hdr, sizeof(hdr))) return -32;
        if (!omniq_ipc_send_all(dev->socket_fd, topo, sizeof(*topo))) return -32;

        struct omniq_ipc_msg_hdr respHdr;
        if (!omniq_ipc_recv_all(dev->socket_fd, &respHdr, sizeof(respHdr))) return -32;
        int statusErr = omniq_ipc_check_status(dev->socket_fd, respHdr);
        if (statusErr != 0) return statusErr;

        if (respHdr.payload_len < sizeof(struct qpu_topology_query)) return -71;
        if (!omniq_ipc_recv_all(dev->socket_fd, topo, sizeof(*topo))) return -32;

        uint32_t edgeBytes = respHdr.payload_len - sizeof(struct qpu_topology_query);
        uint32_t numReturned = edgeBytes / sizeof(struct qpu_coupler_edge);
        if (numReturned > 0) {
            uint32_t toCopy = std::min(numReturned, max_edges);
            if (edges && toCopy > 0) {
                if (!omniq_ipc_recv_all(dev->socket_fd, edges, toCopy * sizeof(struct qpu_coupler_edge))) return -32;
            }
            if (numReturned > toCopy) {
                std::vector<uint8_t> discard((numReturned - toCopy) * sizeof(struct qpu_coupler_edge));
                if (!omniq_ipc_recv_all(dev->socket_fd, discard.data(), discard.size())) return -32;
            }
        }
        return 0;
    }

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

    if (dev->isIpcMode && dev->socket_fd >= 0) {
        struct omniq_ipc_msg_hdr hdr;
        std::memset(&hdr, 0, sizeof(hdr));
        hdr.magic = OMNIQ_IPC_MAGIC;
        hdr.msg_type = OMNIQ_MSG_QUERY_CALIBRATION;
        hdr.payload_len = sizeof(query);

        if (!omniq_ipc_send_all(dev->socket_fd, &hdr, sizeof(hdr))) return -32;
        if (!omniq_ipc_send_all(dev->socket_fd, &query, sizeof(query))) return -32;

        struct omniq_ipc_msg_hdr respHdr;
        if (!omniq_ipc_recv_all(dev->socket_fd, &respHdr, sizeof(respHdr))) return -32;
        int statusErr = omniq_ipc_check_status(dev->socket_fd, respHdr);
        if (statusErr != 0) return statusErr;

        if (respHdr.payload_len < sizeof(query)) return -71;
        if (!omniq_ipc_recv_all(dev->socket_fd, &query, sizeof(query))) return -32;

        uint32_t calBytes = respHdr.payload_len - sizeof(query);
        uint32_t numReturned = calBytes / sizeof(struct qpu_qubit_calibration);
        if (numReturned > 0) {
            uint32_t toCopy = std::min(numReturned, max_qubits);
            if (toCopy > 0) {
                if (!omniq_ipc_recv_all(dev->socket_fd, cals, toCopy * sizeof(struct qpu_qubit_calibration))) return -32;
            }
            if (numReturned > toCopy) {
                std::vector<uint8_t> discard((numReturned - toCopy) * sizeof(struct qpu_qubit_calibration));
                if (!omniq_ipc_recv_all(dev->socket_fd, discard.data(), discard.size())) return -32;
            }
        }
        if (timestamp_out) {
            *timestamp_out = query.timestamp_ns;
        }
        return 0;
    }

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

    if (dev->isIpcMode && dev->socket_fd >= 0) {
        struct omniq_ipc_msg_hdr hdr;
        std::memset(&hdr, 0, sizeof(hdr));
        hdr.magic = OMNIQ_IPC_MAGIC;
        hdr.msg_type = OMNIQ_MSG_ALLOC_QUBITS;
        hdr.client_pid = static_cast<uint32_t>(getpid());
        hdr.payload_len = sizeof(req);

        if (!omniq_ipc_send_all(dev->socket_fd, &hdr, sizeof(hdr))) return -32;
        if (!omniq_ipc_send_all(dev->socket_fd, &req, sizeof(req))) return -32;

        struct omniq_ipc_msg_hdr respHdr;
        if (!omniq_ipc_recv_all(dev->socket_fd, &respHdr, sizeof(respHdr))) return -32;
        int statusErr = omniq_ipc_check_status(dev->socket_fd, respHdr);
        if (statusErr != 0) return statusErr;

        if (respHdr.payload_len < sizeof(req)) return -71;
        if (!omniq_ipc_recv_all(dev->socket_fd, &req, sizeof(req))) return -32;

        *out_partition_id = req.partition_id;
        return 0;
    }

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

    if (dev->isIpcMode && dev->socket_fd >= 0) {
        struct omniq_ipc_msg_hdr hdr;
        std::memset(&hdr, 0, sizeof(hdr));
        hdr.magic = OMNIQ_IPC_MAGIC;
        hdr.msg_type = OMNIQ_MSG_FREE_QUBITS;
        hdr.payload_len = sizeof(req);

        if (!omniq_ipc_send_all(dev->socket_fd, &hdr, sizeof(hdr))) return -32;
        if (!omniq_ipc_send_all(dev->socket_fd, &req, sizeof(req))) return -32;

        struct omniq_ipc_msg_hdr respHdr;
        if (!omniq_ipc_recv_all(dev->socket_fd, &respHdr, sizeof(respHdr))) return -32;
        return respHdr.status;
    }

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

    if (dev->isIpcMode && dev->socket_fd >= 0) {
        struct omniq_ipc_msg_hdr hdr;
        std::memset(&hdr, 0, sizeof(hdr));
        hdr.magic = OMNIQ_IPC_MAGIC;
        hdr.msg_type = OMNIQ_MSG_SUBMIT_JOB;
        hdr.payload_len = sizeof(job) + job.bytecode_len;

        if (!omniq_ipc_send_all(dev->socket_fd, &hdr, sizeof(hdr))) return -32;
        if (!omniq_ipc_send_all(dev->socket_fd, &job, sizeof(job))) return -32;
        if (job.bytecode_len > 0) {
            if (!omniq_ipc_send_all(dev->socket_fd, code.data(), job.bytecode_len)) return -32;
        }

        struct omniq_ipc_msg_hdr respHdr;
        if (!omniq_ipc_recv_all(dev->socket_fd, &respHdr, sizeof(respHdr))) return -32;
        int statusErr = omniq_ipc_check_status(dev->socket_fd, respHdr);
        if (statusErr != 0) return statusErr;

        if (respHdr.payload_len < sizeof(job)) return -71;
        if (!omniq_ipc_recv_all(dev->socket_fd, &job, sizeof(job))) return -32;

        *out_job_id = job.job_id;
        return 0;
    }

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

    if (dev->isIpcMode && dev->socket_fd >= 0) {
        struct omniq_ipc_msg_hdr hdr;
        std::memset(&hdr, 0, sizeof(hdr));
        hdr.magic = OMNIQ_IPC_MAGIC;
        hdr.msg_type = OMNIQ_MSG_READ_RESULTS;
        hdr.payload_len = sizeof(res);

        if (!omniq_ipc_send_all(dev->socket_fd, &hdr, sizeof(hdr))) return -32;
        if (!omniq_ipc_send_all(dev->socket_fd, &res, sizeof(res))) return -32;

        struct omniq_ipc_msg_hdr respHdr;
        if (!omniq_ipc_recv_all(dev->socket_fd, &respHdr, sizeof(respHdr))) return -32;
        int statusErr = omniq_ipc_check_status(dev->socket_fd, respHdr);
        if (statusErr != 0) return statusErr;

        if (respHdr.payload_len < sizeof(res)) return -71;
        if (!omniq_ipc_recv_all(dev->socket_fd, &res, sizeof(res))) return -32;

        uint32_t resBytes = respHdr.payload_len - sizeof(res);
        uint32_t numReturned = resBytes / sizeof(struct qpu_readout_entry);
        if (numReturned > 0) {
            uint32_t toCopy = std::min(numReturned, max_entries);
            if (entries && toCopy > 0) {
                if (!omniq_ipc_recv_all(dev->socket_fd, entries, toCopy * sizeof(struct qpu_readout_entry))) return -32;
            }
            if (numReturned > toCopy) {
                std::vector<uint8_t> discard((numReturned - toCopy) * sizeof(struct qpu_readout_entry));
                if (!omniq_ipc_recv_all(dev->socket_fd, discard.data(), discard.size())) return -32;
            }
        }
        if (out_num_entries) *out_num_entries = res.num_unique_outcomes;
        if (out_total_shots) *out_total_shots = res.total_shots_collected;
        return 0;
    }

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

const char *qpu_tech_name(uint32_t technology) {
    switch (technology) {
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

int qpu_query_partitions(qpu_device_t *dev, struct qpu_partition_entry *partitions,
                         uint32_t max_partitions, uint32_t *out_num_partitions) {
    if (!dev) return -22;
    if (max_partitions > 0 && !partitions) return -22;

    struct qpu_partition_query query;
    std::memset(&query, 0, sizeof(query));
    query.max_partitions_capacity = max_partitions;
    query.partitions_buffer_ptr = reinterpret_cast<uint64_t>(partitions);

    if (dev->isIpcMode && dev->socket_fd >= 0) {
        struct omniq_ipc_msg_hdr hdr;
        std::memset(&hdr, 0, sizeof(hdr));
        hdr.magic = OMNIQ_IPC_MAGIC;
        hdr.msg_type = OMNIQ_MSG_QUERY_PARTITIONS;
        hdr.payload_len = sizeof(query);

        if (!omniq_ipc_send_all(dev->socket_fd, &hdr, sizeof(hdr))) return -32;
        if (!omniq_ipc_send_all(dev->socket_fd, &query, sizeof(query))) return -32;

        struct omniq_ipc_msg_hdr respHdr;
        if (!omniq_ipc_recv_all(dev->socket_fd, &respHdr, sizeof(respHdr))) return -32;
        int statusErr = omniq_ipc_check_status(dev->socket_fd, respHdr);
        if (statusErr != 0) return statusErr;

        if (respHdr.payload_len < sizeof(query)) return -71;
        if (!omniq_ipc_recv_all(dev->socket_fd, &query, sizeof(query))) return -32;

        uint32_t partBytes = respHdr.payload_len - sizeof(query);
        uint32_t numReturned = partBytes / sizeof(struct qpu_partition_entry);
        if (numReturned > 0) {
            uint32_t toCopy = std::min(numReturned, max_partitions);
            if (partitions && toCopy > 0) {
                if (!omniq_ipc_recv_all(dev->socket_fd, partitions, toCopy * sizeof(struct qpu_partition_entry))) return -32;
            }
            if (numReturned > toCopy) {
                std::vector<uint8_t> discard((numReturned - toCopy) * sizeof(struct qpu_partition_entry));
                if (!omniq_ipc_recv_all(dev->socket_fd, discard.data(), discard.size())) return -32;
            }
        }
        if (out_num_partitions) *out_num_partitions = query.num_partitions;
        return 0;
    }

    int ret = 0;
    if (dev->isDirectMode && dev->directDriver) {
        ret = dev->directDriver->ioctlQueryPartitions(&query, partitions);
    } else {
#ifdef __linux__
        if (dev->fd >= 0) {
            ret = ioctl(dev->fd, QPU_IOCTL_QUERY_PARTITIONS, &query);
        } else {
            ret = -9;
        }
#else
        ret = -38;
#endif
    }

    if (ret == 0 && out_num_partitions) {
        *out_num_partitions = query.num_partitions;
    }
    return ret;
}

int qpu_query_cryo(qpu_device_t *dev, struct qpu_cryo_telemetry *cryo) {
    if (!dev || !cryo) return -22;

    if (dev->isIpcMode && dev->socket_fd >= 0) {
        struct omniq_ipc_msg_hdr hdr;
        std::memset(&hdr, 0, sizeof(hdr));
        hdr.magic = OMNIQ_IPC_MAGIC;
        hdr.msg_type = OMNIQ_MSG_QUERY_CRYO;
        hdr.payload_len = 0;

        if (!omniq_ipc_send_all(dev->socket_fd, &hdr, sizeof(hdr))) return -32;

        struct omniq_ipc_msg_hdr respHdr;
        if (!omniq_ipc_recv_all(dev->socket_fd, &respHdr, sizeof(respHdr))) return -32;
        int statusErr = omniq_ipc_check_status(dev->socket_fd, respHdr);
        if (statusErr != 0) return statusErr;

        if (respHdr.payload_len < sizeof(*cryo)) return -71;
        if (!omniq_ipc_recv_all(dev->socket_fd, cryo, sizeof(*cryo))) return -32;
        return 0;
    }

    if (dev->isDirectMode && dev->directDriver) {
        return dev->directDriver->ioctlQueryCryo(cryo);
    }
#ifdef __linux__
    if (dev->fd >= 0) {
        return ioctl(dev->fd, QPU_IOCTL_QUERY_CRYO, cryo);
    }
    return -9;
#else
    return -38;
#endif
}

int qpu_reset_qubit(qpu_device_t *dev, uint32_t physical_id, uint32_t reset_mode) {
    if (!dev) return -22;

    struct qpu_reset_qubit_req req;
    std::memset(&req, 0, sizeof(req));
    req.physical_id = physical_id;
    req.reset_mode = reset_mode;

    if (dev->isIpcMode && dev->socket_fd >= 0) {
        struct omniq_ipc_msg_hdr hdr;
        std::memset(&hdr, 0, sizeof(hdr));
        hdr.magic = OMNIQ_IPC_MAGIC;
        hdr.msg_type = OMNIQ_MSG_RESET_QUBIT;
        hdr.payload_len = sizeof(req);

        if (!omniq_ipc_send_all(dev->socket_fd, &hdr, sizeof(hdr))) return -32;
        if (!omniq_ipc_send_all(dev->socket_fd, &req, sizeof(req))) return -32;

        struct omniq_ipc_msg_hdr respHdr;
        if (!omniq_ipc_recv_all(dev->socket_fd, &respHdr, sizeof(respHdr))) return -32;
        int statusErr = omniq_ipc_check_status(dev->socket_fd, respHdr);
        if (statusErr != 0) return statusErr;

        if (respHdr.payload_len < sizeof(req)) return -71;
        if (!omniq_ipc_recv_all(dev->socket_fd, &req, sizeof(req))) return -32;
        return (req.success == 1) ? 0 : -22;
    }

    if (dev->isDirectMode && dev->directDriver) {
        return dev->directDriver->ioctlResetQubit(&req);
    }
#ifdef __linux__
    if (dev->fd >= 0) {
        return ioctl(dev->fd, QPU_IOCTL_RESET_QUBIT, &req);
    }
    return -9;
#else
    return -38;
#endif
}

} // extern "C"
