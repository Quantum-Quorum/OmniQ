/*
 * OmniQ-QOS: Quantum Operating System Daemon (omniqd)
 * src/daemon/omniqd.cpp
 *
 * Copyright (c) 2026 Quantum Quorum
 */

#include "../driver/VirtualQPUDriver.h"
#include "../../include/omniq/qpu.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <csignal>
#include <iomanip>

namespace fs = std::filesystem;

static volatile bool g_running = true;

void signalHandler(int) {
    g_running = false;
}

void exportSysfsTree(const omniq::qos::VirtualQPUDriver &driver, const std::string &baseDir) {
    fs::create_directories(baseDir + "/topology");
    fs::create_directories(baseDir + "/calibration");
    fs::create_directories(baseDir + "/thermal");
    fs::create_directories("/tmp/omniq_proc/qpu");

    const auto &qmmu = driver.getQMMU();

    // 1. Static attributes
    {
        std::ofstream f(baseDir + "/num_physical_qubits");
        f << qmmu.getNumPhysicalQubits() << "\n";
    }
    {
        std::ofstream f(baseDir + "/technology");
        f << qpu_tech_name(qmmu.getTechnology()) << "\n";
    }

    // 2. Topology
    {
        std::ofstream f(baseDir + "/topology/coupling_matrix");
        for (const auto &e : qmmu.getCouplerEdges()) {
            f << e.q1 << " <-> " << e.q2 << " (fidelity: " << std::fixed
              << std::setprecision(4) << e.fidelity * 100.0 << "%)\n";
        }
    }

    // 3. Calibration
    {
        std::ofstream ft1(baseDir + "/calibration/t1_relaxation_us");
        std::ofstream ft2(baseDir + "/calibration/t2_dephasing_us");
        std::ofstream ferr(baseDir + "/calibration/single_qubit_error");
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
    {
        std::ofstream f(baseDir + "/thermal/mixing_chamber_mk");
        f << "14.25 mK\n";
    }

    // 5. Procfs live status
    {
        std::ofstream f("/tmp/omniq_proc/qpu/status");
        f << "OmniQ QPU Accelerator Subsystem Status\n";
        f << "=====================================\n";
        f << "Driver: OmniQ-QOS v0.1.0-alpha (drivers/accel/qpu)\n";
        f << "Physical Qubits: " << qmmu.getNumPhysicalQubits() << "\n";
        f << "Allocated Qubits: " << qmmu.getNumAllocatedQubits() << "\n";
        f << "Guard Band Qubits: " << qmmu.getNumGuardBandQubits() << "\n";
        f << "Free Qubits: " << qmmu.getNumFreeQubits() << "\n";
        f << "Active Partitions: " << qmmu.getAllActivePartitions().size() << "\n";
    }
}

int main(int argc, char *argv[]) {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    int numQubits = 32;
    if (argc > 1) {
        numQubits = std::atoi(argv[1]);
        if (numQubits < 4) numQubits = 32;
    }

    std::cout << "========================================================\n";
    std::cout << " OmniQ-QOS: Quantum Operating System Daemon (omniqd)   \n";
    std::cout << " Linux Compute Accelerator Subsystem for QPUs          \n";
    std::cout << "========================================================\n";

    omniq::qos::VirtualQPUDriver driver(numQubits, QPU_TECH_SUPERCONDUCTING_TRANSMON);

    std::string sysfsPath = "/tmp/omniq_sys/class/accel/qpu0";
    exportSysfsTree(driver, sysfsPath);

    std::cout << "[INFO] Initialized QPU Controller with " << numQubits << " physical transmons.\n";
    std::cout << "[INFO] Exported virtual sysfs tree to: " << sysfsPath << "\n";
    std::cout << "[INFO] Exported virtual procfs node to: /tmp/omniq_proc/qpu/status\n";
    std::cout << "[INFO] QMMU ready for dynamic spatial partition allocation.\n";

    // Demo: allocate two concurrent tenant partitions with guard bands!
    std::cout << "\n--- Demonstrating Quantum Multi-Tenancy Slicing ---\n";

    uint32_t pidA = 0;
    struct qpu_alloc_req reqA = {4, QPU_ALLOC_FLAG_ISOLATED | QPU_ALLOC_FLAG_HIGH_FIDELITY, 0, 0, 0, 0};
    driver.ioctlAllocQubits(&reqA, 1001);
    pidA = reqA.partition_id;

    uint32_t pidB = 0;
    struct qpu_alloc_req reqB = {6, QPU_ALLOC_FLAG_ISOLATED, 0, 0, 0, 0};
    driver.ioctlAllocQubits(&reqB, 1002);
    pidB = reqB.partition_id;

    std::cout << "Tenant A (PID 1001): Allocated Partition #" << pidA << " (4 virtual qubits)\n";
    std::cout << "Tenant B (PID 1002): Allocated Partition #" << pidB << " (6 virtual qubits)\n";
    std::cout << "Total Active: " << driver.getQMMU().getNumAllocatedQubits() << " qubits, "
              << driver.getQMMU().getNumGuardBandQubits() << " guard band isolation qubits, "
              << driver.getQMMU().getNumFreeQubits() << " free.\n";

    // Submit Bell state on Partition A
    std::string bellQasm =
        "OPENQASM 2.0;\n"
        "include \"qelib1.inc\";\n"
        "qreg q[4];\n"
        "creg c[4];\n"
        "h q[0];\n"
        "cx q[0], q[1];\n";

    struct qpu_submit_job jobReq = {pidA, QPU_JOB_FLAG_DYNAMIC_DECOUPLING, 1000, 500, 0, 0, 0};
    driver.ioctlSubmitJob(&jobReq, bellQasm);

    std::cout << "[JOB] Submitted Bell pair on Partition #" << pidA << " -> Job ID #" << jobReq.job_id << "\n";

    struct qpu_readout_entry entries[16];
    struct qpu_result_req resReq = {jobReq.job_id, 1000, 0, 0, 0, reinterpret_cast<uint64_t>(entries), 16};
    driver.ioctlReadResults(&resReq, entries);

    std::cout << "[RESULT] Job #" << jobReq.job_id << " Completed (" << resReq.total_shots_collected << " shots):\n";
    for (uint32_t i = 0; i < resReq.num_unique_outcomes; ++i) {
        std::cout << "   |State 0x" << std::hex << entries[i].basis_state << std::dec << "⟩: "
                  << entries[i].count << " shots ("
                  << (entries[i].count * 100.0 / resReq.total_shots_collected) << "%)\n";
    }

    // Refresh sysfs status
    exportSysfsTree(driver, sysfsPath);

    std::cout << "\n[DAEMON] omniqd background monitor running. Press Ctrl+C or kill to stop.\n";
    int cycle = 0;
    while (g_running && cycle < 3) { // Run 3 cycles for headless demo or keeps running
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        cycle++;
    }

    // Clean up
    struct qpu_free_req freeReqA = {pidA, 0};
    struct qpu_free_req freeReqB = {pidB, 0};
    driver.ioctlFreeQubits(&freeReqA);
    driver.ioctlFreeQubits(&freeReqB);

    std::cout << "[DAEMON] Partitions freed and qubits returned to pool. Shutting down cleanly.\n";
    return 0;
}
