/*
 * OmniQ-QOS: Quantum Operating System Daemon Server Implementation
 * src/daemon/DaemonServer.cpp
 *
 * Copyright (c) 2026 Quantum Quorum
 */

#include "../../include/omniq/DaemonServer.h"
#include "../../include/omniq/qpu.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

namespace fs = std::filesystem;

namespace omniq {
namespace qos {

DaemonServer::DaemonServer(int numQubits,
                           uint32_t technology,
                           const std::string &socketPath,
                           const std::string &sysfsPath,
                           const std::string &procfsPath)
    : numQubits_(numQubits),
      technology_(technology),
      socketPath_(socketPath),
      sysfsPath_(sysfsPath),
      procfsPath_(procfsPath),
      driver_(numQubits, technology),
      serverFd_(-1),
      running_(false),
      started_(false),
      totalJobsSubmitted_(0),
      totalJobsCompleted_(0) {}

DaemonServer::~DaemonServer() {
    stop();
}

int DaemonServer::start() {
    if (started_.load()) return 0;

    // Unlink old socket file if it exists
    ::unlink(socketPath_.c_str());

    serverFd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverFd_ < 0) {
        return -errno;
    }

    struct sockaddr_un addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, socketPath_.c_str(), sizeof(addr.sun_path) - 1);

    if (::bind(serverFd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        int err = -errno;
        ::close(serverFd_);
        serverFd_ = -1;
        return err;
    }

    if (::listen(serverFd_, 32) < 0) {
        int err = -errno;
        ::close(serverFd_);
        serverFd_ = -1;
        ::unlink(socketPath_.c_str());
        return err;
    }

    ::chmod(socketPath_.c_str(), 0666);

    exportSysfsAndProcfs();

    running_.store(true);
    started_.store(true);
    return 0;
}

void DaemonServer::run() {
    while (running_.load()) {
        struct pollfd pfd;
        pfd.fd = serverFd_;
        pfd.events = POLLIN;
        pfd.revents = 0;

        int ret = ::poll(&pfd, 1, 100); // 100ms timeout
        if (ret < 0) {
            if (errno == EINTR) continue;
            break;
        }
        if (ret == 0) continue;

        if (pfd.revents & POLLIN) {
            struct sockaddr_un clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            int clientFd = ::accept(serverFd_, reinterpret_cast<struct sockaddr*>(&clientAddr), &clientLen);
            if (clientFd >= 0) {
                std::thread([this, clientFd]() {
                    this->handleClient(clientFd);
                }).detach();
            }
        }
    }
}

void DaemonServer::stop() {
    bool expected = true;
    if (running_.compare_exchange_strong(expected, false)) {
        if (serverFd_ >= 0) {
            ::shutdown(serverFd_, SHUT_RDWR);
            ::close(serverFd_);
            serverFd_ = -1;
        }
        ::unlink(socketPath_.c_str());
        started_.store(false);
    }
}

int DaemonServer::createClientSocket() {
    int sv[2];
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
        return -errno;
    }
    std::thread([this, fd = sv[0]]() {
        this->handleClient(fd);
    }).detach();
    return sv[1];
}

void DaemonServer::handleClient(int clientFd) {
    while (running_.load()) {
        struct pollfd pfd;
        pfd.fd = clientFd;
        pfd.events = POLLIN;
        pfd.revents = 0;

        int ret = ::poll(&pfd, 1, 250);
        if (ret < 0) {
            if (errno == EINTR) continue;
            break;
        }
        if (ret == 0) continue; // timeout: re-check running_

        if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
            break;
        }

        if (pfd.revents & POLLIN) {
            struct omniq_ipc_msg_hdr hdr;
            if (!omniq_ipc_recv_all(clientFd, &hdr, sizeof(hdr))) {
                break;
            }
            if (hdr.magic != OMNIQ_IPC_MAGIC) {
                break;
            }
            processClientMessage(clientFd, hdr);
        }
    }
    ::close(clientFd);
}

void DaemonServer::processClientMessage(int clientFd, const struct omniq_ipc_msg_hdr &hdr) {
    struct omniq_ipc_msg_hdr respHdr;
    std::memset(&respHdr, 0, sizeof(respHdr));
    respHdr.magic = OMNIQ_IPC_MAGIC;
    respHdr.msg_type = hdr.msg_type;

    switch (hdr.msg_type) {
    case OMNIQ_MSG_QUERY_TOPOLOGY: {
        struct qpu_topology_query topo;
        if (!omniq_ipc_recv_all(clientFd, &topo, sizeof(topo))) return;

        std::vector<struct qpu_coupler_edge> edges(topo.max_edges_capacity);
        int ret = driver_.ioctlQueryTopology(&topo, edges.empty() ? nullptr : edges.data());
        uint32_t num_edges = (ret == 0) ? std::min(topo.num_edges, topo.max_edges_capacity) : 0;

        respHdr.status = ret;
        respHdr.payload_len = sizeof(topo) + num_edges * sizeof(struct qpu_coupler_edge);

        if (!omniq_ipc_send_all(clientFd, &respHdr, sizeof(respHdr))) return;
        if (!omniq_ipc_send_all(clientFd, &topo, sizeof(topo))) return;
        if (num_edges > 0) {
            if (!omniq_ipc_send_all(clientFd, edges.data(), num_edges * sizeof(struct qpu_coupler_edge))) return;
        }
        break;
    }

    case OMNIQ_MSG_QUERY_CALIBRATION: {
        struct qpu_calibration_query query;
        if (!omniq_ipc_recv_all(clientFd, &query, sizeof(query))) return;

        std::vector<struct qpu_qubit_calibration> cals(query.num_qubits);
        int ret = driver_.ioctlQueryCalibration(&query, cals.empty() ? nullptr : cals.data());
        uint32_t numNodes = static_cast<uint32_t>(driver_.getQMMU().getNumPhysicalQubits());
        uint32_t numCals = (ret == 0) ? std::min(numNodes, query.num_qubits) : 0;

        respHdr.status = ret;
        respHdr.payload_len = sizeof(query) + numCals * sizeof(struct qpu_qubit_calibration);

        if (!omniq_ipc_send_all(clientFd, &respHdr, sizeof(respHdr))) return;
        if (!omniq_ipc_send_all(clientFd, &query, sizeof(query))) return;
        if (numCals > 0) {
            if (!omniq_ipc_send_all(clientFd, cals.data(), numCals * sizeof(struct qpu_qubit_calibration))) return;
        }
        break;
    }

    case OMNIQ_MSG_ALLOC_QUBITS: {
        struct qpu_alloc_req req;
        if (!omniq_ipc_recv_all(clientFd, &req, sizeof(req))) return;

        int ret = driver_.ioctlAllocQubits(&req, hdr.client_pid);
        if (ret == 0) {
            exportSysfsAndProcfs();
            respHdr.status = 0;
            respHdr.payload_len = sizeof(req);
            if (!omniq_ipc_send_all(clientFd, &respHdr, sizeof(respHdr))) return;
            if (!omniq_ipc_send_all(clientFd, &req, sizeof(req))) return;
        } else {
            respHdr.status = ret;
            respHdr.payload_len = 0;
            if (!omniq_ipc_send_all(clientFd, &respHdr, sizeof(respHdr))) return;
        }
        break;
    }

    case OMNIQ_MSG_FREE_QUBITS: {
        struct qpu_free_req req;
        if (!omniq_ipc_recv_all(clientFd, &req, sizeof(req))) return;

        int ret = driver_.ioctlFreeQubits(&req);
        if (ret == 0) {
            exportSysfsAndProcfs();
        }

        respHdr.status = ret;
        respHdr.payload_len = 0;

        if (!omniq_ipc_send_all(clientFd, &respHdr, sizeof(respHdr))) return;
        break;
    }

    case OMNIQ_MSG_SUBMIT_JOB: {
        struct qpu_submit_job job;
        if (!omniq_ipc_recv_all(clientFd, &job, sizeof(job))) return;

        std::string code;
        if (job.bytecode_len > 0) {
            code.resize(job.bytecode_len);
            if (!omniq_ipc_recv_all(clientFd, &code[0], job.bytecode_len)) return;
        }

        int ret = driver_.ioctlSubmitJob(&job, code);
        if (ret == 0) {
            totalJobsSubmitted_++;
            exportSysfsAndProcfs();
            respHdr.status = 0;
            respHdr.payload_len = sizeof(job);
            if (!omniq_ipc_send_all(clientFd, &respHdr, sizeof(respHdr))) return;
            if (!omniq_ipc_send_all(clientFd, &job, sizeof(job))) return;
        } else {
            respHdr.status = ret;
            respHdr.payload_len = 0;
            if (!omniq_ipc_send_all(clientFd, &respHdr, sizeof(respHdr))) return;
        }
        break;
    }

    case OMNIQ_MSG_READ_RESULTS: {
        struct qpu_result_req res;
        if (!omniq_ipc_recv_all(clientFd, &res, sizeof(res))) return;

        std::vector<struct qpu_readout_entry> entries(res.max_entries_capacity);
        int ret = driver_.ioctlReadResults(&res, entries.empty() ? nullptr : entries.data());
        if (ret == 0) {
            if (res.is_completed) {
                totalJobsCompleted_++;
                exportSysfsAndProcfs();
            }
            uint32_t numEntries = std::min(res.num_unique_outcomes, res.max_entries_capacity);
            respHdr.status = 0;
            respHdr.payload_len = sizeof(res) + numEntries * sizeof(struct qpu_readout_entry);

            if (!omniq_ipc_send_all(clientFd, &respHdr, sizeof(respHdr))) return;
            if (!omniq_ipc_send_all(clientFd, &res, sizeof(res))) return;
            if (numEntries > 0) {
                if (!omniq_ipc_send_all(clientFd, entries.data(), numEntries * sizeof(struct qpu_readout_entry))) return;
            }
        } else {
            respHdr.status = ret;
            respHdr.payload_len = 0;
            if (!omniq_ipc_send_all(clientFd, &respHdr, sizeof(respHdr))) return;
        }
        break;
    }

    case OMNIQ_MSG_QUERY_PARTITIONS: {
        struct qpu_partition_query query;
        if (!omniq_ipc_recv_all(clientFd, &query, sizeof(query))) return;

        std::vector<struct qpu_partition_entry> parts(query.max_partitions_capacity);
        int ret = driver_.ioctlQueryPartitions(&query, parts.empty() ? nullptr : parts.data());
        uint32_t numParts = (ret == 0) ? std::min(query.num_partitions, query.max_partitions_capacity) : 0;

        respHdr.status = ret;
        respHdr.payload_len = (ret == 0) ? (sizeof(query) + numParts * sizeof(struct qpu_partition_entry)) : 0;

        if (!omniq_ipc_send_all(clientFd, &respHdr, sizeof(respHdr))) return;
        if (ret == 0) {
            if (!omniq_ipc_send_all(clientFd, &query, sizeof(query))) return;
            if (numParts > 0) {
                if (!omniq_ipc_send_all(clientFd, parts.data(), numParts * sizeof(struct qpu_partition_entry))) return;
            }
        }
        break;
    }

    case OMNIQ_MSG_QUERY_CRYO: {
        struct qpu_cryo_telemetry cryo;
        std::memset(&cryo, 0, sizeof(cryo));
        int ret = driver_.ioctlQueryCryo(&cryo);

        respHdr.status = ret;
        respHdr.payload_len = (ret == 0) ? sizeof(cryo) : 0;

        if (!omniq_ipc_send_all(clientFd, &respHdr, sizeof(respHdr))) return;
        if (ret == 0) {
            if (!omniq_ipc_send_all(clientFd, &cryo, sizeof(cryo))) return;
        }
        break;
    }

    case OMNIQ_MSG_RESET_QUBIT: {
        struct qpu_reset_qubit_req req;
        if (!omniq_ipc_recv_all(clientFd, &req, sizeof(req))) return;

        int ret = driver_.ioctlResetQubit(&req);
        if (ret == 0 && req.success == 1) {
            exportSysfsAndProcfs();
            respHdr.status = 0;
            respHdr.payload_len = sizeof(req);
            if (!omniq_ipc_send_all(clientFd, &respHdr, sizeof(respHdr))) return;
            if (!omniq_ipc_send_all(clientFd, &req, sizeof(req))) return;
        } else {
            respHdr.status = (ret != 0) ? ret : -22;
            respHdr.payload_len = 0;
            if (!omniq_ipc_send_all(clientFd, &respHdr, sizeof(respHdr))) return;
        }
        break;
    }

    case OMNIQ_MSG_PING: {
        respHdr.status = 0;
        respHdr.payload_len = 0;
        omniq_ipc_send_all(clientFd, &respHdr, sizeof(respHdr));
        break;
    }

    default: {
        respHdr.status = -38; // -ENOSYS
        respHdr.payload_len = 0;
        omniq_ipc_send_all(clientFd, &respHdr, sizeof(respHdr));
        break;
    }
    }
}

void DaemonServer::exportSysfsAndProcfs() {
    std::lock_guard<std::mutex> lock(exportMutex_);

    try {
        fs::create_directories(sysfsPath_ + "/topology");
        fs::create_directories(sysfsPath_ + "/calibration");
        fs::create_directories(sysfsPath_ + "/thermal");
        fs::create_directories(procfsPath_);
    } catch (...) {}

    const auto &qmmu = driver_.getQMMU();

    // 1. Static attributes
    {
        std::ofstream f(sysfsPath_ + "/num_physical_qubits");
        f << qmmu.getNumPhysicalQubits() << "\n";
    }
    {
        std::ofstream f(sysfsPath_ + "/technology");
        f << qpu_tech_name(qmmu.getTechnology()) << "\n";
    }

    // 2. Topology
    {
        std::ofstream f(sysfsPath_ + "/topology/coupling_matrix");
        for (const auto &e : qmmu.getCouplerEdges()) {
            f << e.q1 << " <-> " << e.q2 << " (fidelity: " << std::fixed
              << std::setprecision(4) << e.fidelity * 100.0 << "%)\n";
        }
    }

    // 3. Calibration
    {
        std::ofstream ft1(sysfsPath_ + "/calibration/t1_relaxation_us");
        std::ofstream ft2(sysfsPath_ + "/calibration/t2_dephasing_us");
        std::ofstream ferr(sysfsPath_ + "/calibration/single_qubit_error");
        for (const auto &n : qmmu.getPhysicalNodes()) {
            ft1 << n.id << ":" << std::fixed << std::setprecision(1) << n.t1_us << " ";
            ft2 << n.id << ":" << std::fixed << std::setprecision(1) << n.t2_us << " ";
            ferr << n.id << ":" << std::fixed << std::setprecision(4) << n.singleQubitError * 100.0 << "% ";
        }
        ft1 << "\n";
        ft2 << "\n";
        ferr << "\n";
    }

    // 4. Cryogenic Telemetry
    struct qpu_cryo_telemetry cryo;
    std::memset(&cryo, 0, sizeof(cryo));
    driver_.ioctlQueryCryo(&cryo);
    {
        std::ofstream f(sysfsPath_ + "/thermal/mixing_chamber_mk");
        f << std::fixed << std::setprecision(3) << (cryo.stage_mxc_uk / 1000.0) << " mK\n";
    }
    {
        std::ofstream f(sysfsPath_ + "/thermal/50k_stage_k");
        f << std::fixed << std::setprecision(3) << (cryo.stage_50k_mk / 1000.0) << " K\n";
    }
    {
        std::ofstream f(sysfsPath_ + "/thermal/4k_stage_k");
        f << std::fixed << std::setprecision(3) << (cryo.stage_4k_mk / 1000.0) << " K\n";
    }
    {
        std::ofstream f(sysfsPath_ + "/thermal/still_stage_mk");
        f << cryo.stage_still_mk << " mK\n";
    }
    {
        std::ofstream f(sysfsPath_ + "/thermal/cold_plate_mk");
        f << cryo.stage_cold_plate_mk << " mK\n";
    }
    {
        std::ofstream f(sysfsPath_ + "/thermal/vacuum_pressure_ubar");
        f << cryo.vacuum_pressure_ubar << " ubar\n";
    }
    {
        std::ofstream f(sysfsPath_ + "/thermal/cryo_ready");
        f << cryo.is_cryo_ready << "\n";
    }

    // 5. Procfs live status
    auto partitions = qmmu.getAllActivePartitions();
    {
        std::ofstream f(procfsPath_ + "/status");
        f << "OmniQ QPU Accelerator Subsystem Status\n";
        f << "=====================================\n";
        f << "Driver: OmniQ-QOS v0.1.0-alpha (drivers/accel/qpu)\n";
        f << "Technology: " << qpu_tech_name(qmmu.getTechnology()) << "\n";
        f << "Physical Qubits: " << qmmu.getNumPhysicalQubits() << "\n";
        f << "Allocated Qubits: " << qmmu.getNumAllocatedQubits() << "\n";
        f << "Guard Band Qubits: " << qmmu.getNumGuardBandQubits() << "\n";
        f << "Free Qubits: " << qmmu.getNumFreeQubits() << "\n";
        f << "Active Partitions: " << partitions.size() << "\n";
        f << "Total Jobs Submitted: " << totalJobsSubmitted_ << "\n";
        f << "Total Jobs Completed: " << totalJobsCompleted_ << "\n";

        if (!partitions.empty()) {
            f << "\nActive Spatial Partitions:\n";
            f << "-------------------------------------\n";
            for (const auto &p : partitions) {
                f << "Partition #" << p.partitionId << " (PID " << p.processId << "): "
                  << p.virtualToPhysical.size() << " virtual -> [";
                for (size_t i = 0; i < p.physicalQubits.size(); ++i) {
                    f << p.physicalQubits[i] << (i + 1 < p.physicalQubits.size() ? ", " : "");
                }
                f << "], " << p.guardQubits.size() << " guards\n";
            }
        }
    }
}

} // namespace qos
} // namespace omniq
