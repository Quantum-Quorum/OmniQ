/*
 * OmniQ-QOS: Quantum Operating System Daemon (omniqd)
 * src/daemon/omniqd.cpp
 *
 * Copyright (c) 2026 Quantum Quorum
 */

#include "../../include/omniq/DaemonServer.h"
#include "../../include/omniq/qpu.h"
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <string>

static omniq::qos::DaemonServer *g_daemon = nullptr;

static void signalHandler(int signum) {
    std::cout << "\n[SIGNAL] Received signal " << signum << ". Initiating graceful shutdown...\n";
    if (g_daemon) {
        g_daemon->stop();
    }
}

int main(int argc, char *argv[]) {
    // Install POSIX signal handlers for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    int numQubits = 32;
    std::string socketPath = OMNIQ_DEFAULT_SOCKET_PATH;

    if (argc > 1) {
        std::string arg1 = argv[1];
        if (arg1 == "--help" || arg1 == "-h") {
            std::cout << "Usage: omniqd [num_qubits] [socket_path]\n";
            std::cout << "Defaults:\n";
            std::cout << "  num_qubits:  32\n";
            std::cout << "  socket_path: /tmp/omniq_qpu0.sock\n";
            return 0;
        }
        numQubits = std::atoi(argv[1]);
        if (numQubits < 4) numQubits = 32;
    }
    if (argc > 2) {
        socketPath = argv[2];
    }

    std::cout << "========================================================\n";
    std::cout << " OmniQ-QOS: Quantum Operating System Daemon (omniqd)   \n";
    std::cout << " Linux Compute Accelerator Subsystem for QPUs          \n";
    std::cout << "========================================================\n";

    omniq::qos::DaemonServer daemon(numQubits, QPU_TECH_SUPERCONDUCTING_TRANSMON, socketPath);
    g_daemon = &daemon;

    int ret = daemon.start();
    if (ret != 0) {
        std::cerr << "[-] Failed to start daemon on socket " << socketPath
                  << " (error code: " << ret << ")\n";
        return 1;
    }

    std::cout << "[INFO] Initialized QPU Controller with " << numQubits << " physical transmons.\n";
    std::cout << "[INFO] Exported virtual sysfs tree to: /tmp/omniq_sys/class/accel/qpu0\n";
    std::cout << "[INFO] Exported virtual procfs node to: /tmp/omniq_proc/qpu/status\n";
    std::cout << "[INFO] QMMU ready for dynamic spatial partition allocation.\n";
    std::cout << "[INFO] Listening for UNIX domain socket IPC on: " << socketPath << "\n";
    std::cout << "[INFO] Daemon running persistently. Press Ctrl+C or kill to stop.\n\n";

    // Run the persistent event loop
    daemon.run();

    std::cout << "[DAEMON] Socket unlinked and resources released cleanly. Exiting.\n";
    g_daemon = nullptr;
    return 0;
}
