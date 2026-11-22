/*
 * OmniQ-QOS: Quantum Operating System Daemon Server
 * include/omniq/DaemonServer.h
 *
 * Copyright (c) 2026 Quantum Quorum
 */

#ifndef _OMNIQ_DAEMON_SERVER_H
#define _OMNIQ_DAEMON_SERVER_H

#include "../../src/driver/VirtualQPUDriver.h"
#include "omniqd_ipc.h"
#include <string>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>

namespace omniq {
namespace qos {

class DaemonServer {
public:
    explicit DaemonServer(int numQubits = 32,
                          uint32_t technology = QPU_TECH_SUPERCONDUCTING_TRANSMON,
                          const std::string &socketPath = OMNIQ_DEFAULT_SOCKET_PATH,
                          const std::string &sysfsPath = "/tmp/omniq_sys/class/accel/qpu0",
                          const std::string &procfsPath = "/tmp/omniq_proc/qpu");
    ~DaemonServer();

    // Start server listening socket and export initial sysfs tree
    int start();

    // Run the persistent event loop (blocks until stop() is called)
    void run();

    // Stop the server, unbind and unlink the socket
    void stop();

    // Create an anonymous IPC client socket connected to this server (useful for testing and sandbox environments)
    int createClientSocket();

    // Check if server is running
    bool isRunning() const { return running_.load(); }

    // Export live sysfs and procfs nodes
    void exportSysfsAndProcfs();

    VirtualQPUDriver &getDriver() { return driver_; }
    const VirtualQPUDriver &getDriver() const { return driver_; }
    const std::string &getSocketPath() const { return socketPath_; }

private:
    void handleClient(int clientFd);
    void processClientMessage(int clientFd, const struct omniq_ipc_msg_hdr &hdr);

    int numQubits_;
    uint32_t technology_;
    std::string socketPath_;
    std::string sysfsPath_;
    std::string procfsPath_;

    VirtualQPUDriver driver_;
    int serverFd_;
    std::atomic<bool> running_;
    std::atomic<bool> started_;

    std::mutex exportMutex_;
    uint64_t totalJobsSubmitted_;
    uint64_t totalJobsCompleted_;
};

} // namespace qos
} // namespace omniq

#endif /* _OMNIQ_DAEMON_SERVER_H */
