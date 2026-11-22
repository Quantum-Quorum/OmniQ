/*
 * OmniQ-QOS: Unit Tests for Quantum Operating System & QMMU Subsystem
 * tests/test_qos.cpp
 *
 * Copyright (c) 2026 Quantum Quorum
 */

#include "omniq/qmmu.h"
#include "omniq/qpu.h"
#include "../omniq-qos/src/driver/VirtualQPUDriver.h"
#include "omniq/DaemonServer.h"
#include "omniq/omniqd_ipc.h"
#include "omniq/Circuit.h"
#include <gtest/gtest.h>
#include <unordered_set>
#include <cmath>
#include <fstream>
#include <thread>
#include <chrono>
#include <filesystem>

using namespace omniq::qos;

// Test 1: QMMU Basic Partition Allocation & Deallocation
TEST(QoSTest, QMMUAllocationAndFree) {
    QMMU qmmu(32, QPU_TECH_SUPERCONDUCTING_TRANSMON);
    EXPECT_EQ(qmmu.getNumPhysicalQubits(), 32);
    EXPECT_EQ(qmmu.getNumFreeQubits(), 32);
    EXPECT_EQ(qmmu.getNumAllocatedQubits(), 0);
    EXPECT_EQ(qmmu.getNumGuardBandQubits(), 0);

    uint32_t pid1 = 0;
    std::vector<int> phys1;
    int ret1 = qmmu.allocatePartition(4, QPU_ALLOC_FLAG_ISOLATED, 1001, pid1, phys1);
    EXPECT_EQ(ret1, 0);
    EXPECT_GT(pid1, 0u);
    EXPECT_EQ(phys1.size(), 4u);
    EXPECT_EQ(qmmu.getNumAllocatedQubits(), 4);
    EXPECT_GT(qmmu.getNumGuardBandQubits(), 0);

    // Free partition
    bool freed = qmmu.freePartition(pid1);
    EXPECT_TRUE(freed);
    EXPECT_EQ(qmmu.getNumAllocatedQubits(), 0);
    EXPECT_EQ(qmmu.getNumGuardBandQubits(), 0);
    EXPECT_EQ(qmmu.getNumFreeQubits(), 32);
}

// Test 2: QMMU Strict Multi-Tenant Boundary Isolation
TEST(QoSTest, StrictMultiTenantIsolation) {
    QMMU qmmu(32, QPU_TECH_SUPERCONDUCTING_TRANSMON);

    // Tenant 1 allocates 4 qubits with ISOLATED flag
    uint32_t pidA = 0;
    std::vector<int> physA;
    ASSERT_EQ(qmmu.allocatePartition(4, QPU_ALLOC_FLAG_ISOLATED, 1001, pidA, physA), 0);

    PartitionDescriptor descA;
    ASSERT_TRUE(qmmu.getPartitionInfo(pidA, descA));
    std::unordered_set<int> setA(descA.physicalQubits.begin(), descA.physicalQubits.end());
    std::unordered_set<int> guardA(descA.guardQubits.begin(), descA.guardQubits.end());

    // Tenant 2 allocates 4 qubits with ISOLATED flag
    uint32_t pidB = 0;
    std::vector<int> physB;
    ASSERT_EQ(qmmu.allocatePartition(4, QPU_ALLOC_FLAG_ISOLATED, 1002, pidB, physB), 0);

    PartitionDescriptor descB;
    ASSERT_TRUE(qmmu.getPartitionInfo(pidB, descB));
    std::unordered_set<int> setB(descB.physicalQubits.begin(), descB.physicalQubits.end());

    // Strict boundary checks:
    // 1. Data qubits of Tenant A and Tenant B must be completely disjoint
    for (int qB : setB) {
        EXPECT_EQ(setA.count(qB), 0u) << "Physical qubit " << qB << " allocated to both tenants!";
        // 2. Data qubits of Tenant B cannot intersect with Guard band of Tenant A
        EXPECT_EQ(guardA.count(qB), 0u) << "Tenant B data qubit " << qB << " violates Tenant A guard band!";
    }

    // 3. Data qubits of Tenant A cannot intersect with Guard band of Tenant B
    std::unordered_set<int> guardB(descB.guardQubits.begin(), descB.guardQubits.end());
    for (int qA : setA) {
        EXPECT_EQ(guardB.count(qA), 0u) << "Tenant A data qubit " << qA << " violates Tenant B guard band!";
    }

    // Free Tenant A and verify Tenant B remains intact
    EXPECT_TRUE(qmmu.freePartition(pidA));
    EXPECT_TRUE(qmmu.getPartitionInfo(pidB, descB));
    EXPECT_EQ(descB.physicalQubits.size(), 4u);
    EXPECT_TRUE(qmmu.freePartition(pidB));
}

// Test 3: Coupler Fidelity Scoring
TEST(QoSTest, CouplerFidelityScoring) {
    QMMU qmmu(16, QPU_TECH_SUPERCONDUCTING_TRANSMON);

    // Set specific high-fidelity coupler between qubits 0 and 1
    qmmu.addCoupler(0, 1, 0.9999, 150.0);

    uint32_t pid = 0;
    std::vector<int> phys;
    int ret = qmmu.allocatePartition(2, QPU_ALLOC_FLAG_HIGH_FIDELITY, 2001, pid, phys);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(phys.size(), 2u);

    // Virtual-to-physical address translation
    int p0 = qmmu.translateVirtualToPhysical(pid, 0);
    int p1 = qmmu.translateVirtualToPhysical(pid, 1);
    EXPECT_GE(p0, 0);
    EXPECT_GE(p1, 0);
    EXPECT_NE(p0, p1);

    // Unmapped virtual qubit returns -1
    EXPECT_EQ(qmmu.translateVirtualToPhysical(pid, 99), -1);
    EXPECT_EQ(qmmu.translateVirtualToPhysical(9999, 0), -1);

    qmmu.freePartition(pid);
}

// Test 4: Cryogenic Telemetry Query (IOCTL 0x08)
TEST(QoSTest, CryoTelemetryIOCTL) {
    VirtualQPUDriver driver(32);
    struct qpu_cryo_telemetry cryo;
    std::memset(&cryo, 0, sizeof(cryo));

    EXPECT_EQ(driver.ioctlQueryCryo(&cryo), 0);
    EXPECT_GT(cryo.timestamp_ns, 0u);
    EXPECT_EQ(cryo.stage_50k_mk, 50250u);
    EXPECT_EQ(cryo.stage_4k_mk, 3820u);
    EXPECT_EQ(cryo.stage_still_mk, 845u);
    EXPECT_EQ(cryo.stage_cold_plate_mk, 98u);
    EXPECT_EQ(cryo.stage_mxc_mk, 14u);
    EXPECT_EQ(cryo.stage_mxc_uk, 14250u);
    EXPECT_EQ(cryo.is_cryo_ready, 1u);

    // Null pointer must return -EINVAL (-22)
    EXPECT_EQ(driver.ioctlQueryCryo(nullptr), -22);
}

// Test 5: Active Qubit Reset (IOCTL 0x09)
TEST(QoSTest, ActiveQubitResetIOCTL) {
    VirtualQPUDriver driver(32);

    // Valid active reset on qubit 0
    struct qpu_reset_qubit_req reqActive = {0, QPU_RESET_MODE_ACTIVE, 0, 0};
    EXPECT_EQ(driver.ioctlResetQubit(&reqActive), 0);
    EXPECT_EQ(reqActive.duration_ns, 240u);
    EXPECT_EQ(reqActive.success, 1u);

    // Valid thermal relaxation reset on qubit 1
    struct qpu_reset_qubit_req reqThermal = {1, QPU_RESET_MODE_THERMAL, 0, 0};
    EXPECT_EQ(driver.ioctlResetQubit(&reqThermal), 0);
    EXPECT_EQ(reqThermal.duration_ns, 600000u);
    EXPECT_EQ(reqThermal.success, 1u);

    // Invalid reset mode (e.g. 99) must fail with -22 and success = 0
    struct qpu_reset_qubit_req reqInvalidMode = {0, 99, 0, 0};
    EXPECT_EQ(driver.ioctlResetQubit(&reqInvalidMode), -22);
    EXPECT_EQ(reqInvalidMode.success, 0u);

    // Out-of-bounds physical qubit ID must fail with -22
    struct qpu_reset_qubit_req reqOOB = {1000, QPU_RESET_MODE_ACTIVE, 0, 0};
    EXPECT_EQ(driver.ioctlResetQubit(&reqOOB), -22);

    // Null pointer must fail with -22
    EXPECT_EQ(driver.ioctlResetQubit(nullptr), -22);
}

// Test 6: Partition Query (IOCTL 0x07)
TEST(QoSTest, PartitionQueryIOCTL) {
    VirtualQPUDriver driver(32);

    struct qpu_alloc_req allocReq = {2, QPU_ALLOC_FLAG_ISOLATED, 0, 0, 0, 0};
    ASSERT_EQ(driver.ioctlAllocQubits(&allocReq, 4001), 0);

    // Query partitions count with null buffer and capacity 0
    struct qpu_partition_query queryCount;
    std::memset(&queryCount, 0, sizeof(queryCount));
    queryCount.max_partitions_capacity = 0;
    queryCount.partitions_buffer_ptr = 0;
    EXPECT_EQ(driver.ioctlQueryPartitions(&queryCount, nullptr), 0);
    EXPECT_EQ(queryCount.num_partitions, 1u);

    // Query with actual buffer
    struct qpu_partition_entry entries[4];
    struct qpu_partition_query queryBuf;
    std::memset(&queryBuf, 0, sizeof(queryBuf));
    queryBuf.max_partitions_capacity = 4;
    queryBuf.partitions_buffer_ptr = reinterpret_cast<uint64_t>(entries);
    EXPECT_EQ(driver.ioctlQueryPartitions(&queryBuf, nullptr), 0);
    EXPECT_EQ(queryBuf.num_partitions, 1u);
    EXPECT_EQ(entries[0].partition_id, allocReq.partition_id);
    EXPECT_EQ(entries[0].process_id, 4001u);
    EXPECT_EQ(entries[0].num_virtual_qubits, 2u);

    struct qpu_free_req freeReq = {allocReq.partition_id, 0};
    driver.ioctlFreeQubits(&freeReq);
}

// Test 7: QASM Parser with Rotations, Phase, Swap, and Measure
TEST(QoSTest, ExpandedQASMParserAndExecution) {
    VirtualQPUDriver driver(32);

    struct qpu_alloc_req allocReq = {2, QPU_ALLOC_FLAG_ISOLATED, 0, 0, 0, 0};
    ASSERT_EQ(driver.ioctlAllocQubits(&allocReq, 5001), 0);
    uint32_t pid = allocReq.partition_id;

    // Test RX(pi) gate: |0> -> |1> with probability 1.0
    const char *qasmRx =
        "OPENQASM 2.0;\n"
        "include \"qelib1.inc\";\n"
        "qreg q[2];\n"
        "creg c[2];\n"
        "rx(pi) q[0];\n"
        "measure q[0] -> c[0];\n";

    struct qpu_submit_job jobRx = {pid, 0, 1024, 500, 0, 0, 0};
    EXPECT_EQ(driver.ioctlSubmitJob(&jobRx, qasmRx), 0);

    struct qpu_readout_entry entriesRx[8];
    struct qpu_result_req resRx = {jobRx.job_id, 1000, 0, 0, 0, reinterpret_cast<uint64_t>(entriesRx), 8};
    EXPECT_EQ(driver.ioctlReadResults(&resRx, entriesRx), 0);
    EXPECT_EQ(resRx.is_completed, 1u);
    EXPECT_EQ(resRx.total_shots_collected, 1024u);

    // Qubit 0 was rotated by pi from |0> to |1>, so bit 0 must be 1 (state 1)
    EXPECT_EQ(resRx.num_unique_outcomes, 1u);
    EXPECT_EQ(entriesRx[0].basis_state, 1u);
    EXPECT_EQ(entriesRx[0].count, 1024u);

    // Test SWAP gate: prepare q0=1, q1=0, swap -> q0=0, q1=1 (state 2)
    const char *qasmSwap =
        "OPENQASM 2.0;\n"
        "include \"qelib1.inc\";\n"
        "qreg q[2];\n"
        "creg c[2];\n"
        "x q[0];\n"
        "swap q[0], q[1];\n"
        "measure q[0] -> c[0];\n"
        "measure q[1] -> c[1];\n";

    struct qpu_submit_job jobSwap = {pid, 0, 512, 500, 0, 0, 0};
    EXPECT_EQ(driver.ioctlSubmitJob(&jobSwap, qasmSwap), 0);

    struct qpu_readout_entry entriesSwap[8];
    struct qpu_result_req resSwap = {jobSwap.job_id, 1000, 0, 0, 0, reinterpret_cast<uint64_t>(entriesSwap), 8};
    EXPECT_EQ(driver.ioctlReadResults(&resSwap, entriesSwap), 0);
    EXPECT_EQ(resSwap.is_completed, 1u);
    EXPECT_EQ(resSwap.num_unique_outcomes, 1u);
    EXPECT_EQ(entriesSwap[0].basis_state, 2u); // |01> = binary 10 in integer = 2
    EXPECT_EQ(entriesSwap[0].count, 512u);

    // Test Memory Protection: accessing unmapped qubit q[10] must be rejected
    const char *qasmOOB =
        "OPENQASM 2.0;\n"
        "qreg q[2];\n"
        "x q[10];\n";

    struct qpu_submit_job jobOOB = {pid, 0, 100, 500, 0, 0, 0};
    EXPECT_EQ(driver.ioctlSubmitJob(&jobOOB, qasmOOB), 0);
    struct qpu_result_req resOOB = {jobOOB.job_id, 1000, 0, 0, 0, 0, 0};
    EXPECT_EQ(driver.ioctlReadResults(&resOOB, nullptr), 0);
    // Unmapped access resulted in 0 shots collected
    EXPECT_EQ(resOOB.total_shots_collected, 0u);

    struct qpu_free_req freeReq = {pid, 0};
    driver.ioctlFreeQubits(&freeReq);
}

// Test 8: Daemon UNIX Domain Socket IPC Client-Server Communication & Sysfs/Procfs Export
TEST(QoSTest, DaemonIPCClientServer) {
    const std::string testSock = "/tmp/omniq_test_ipc1.sock";
    const std::string testSysfs = "/tmp/omniq_test_sys1/class/accel/qpu0";
    const std::string testProcfs = "/tmp/omniq_test_proc1/qpu";

    DaemonServer server(32, QPU_TECH_SUPERCONDUCTING_TRANSMON, testSock, testSysfs, testProcfs);
    ASSERT_EQ(server.start(), 0);

    std::thread serverThread([&server]() {
        server.run();
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // 1. Open device over UNIX domain socket IPC
    qpu_device_t *client = qpu_open(testSock.c_str());
    ASSERT_NE(client, nullptr);
    if (!qpu_is_ipc(client)) {
        // Fallback for sandboxed test environments where connect() gets EPERM
        int clientFd = server.createClientSocket();
        char fdPath[32];
        snprintf(fdPath, sizeof(fdPath), "fd:%d", clientFd);
        qpu_close(client);
        client = qpu_open(fdPath);
        ASSERT_NE(client, nullptr);
        ASSERT_TRUE(qpu_is_ipc(client));
    }

    // 2. Query Topology over IPC
    struct qpu_topology_query topo;
    std::memset(&topo, 0, sizeof(topo));
    struct qpu_coupler_edge edges[32];
    EXPECT_EQ(qpu_get_topology(client, &topo, edges, 32), 0);
    EXPECT_EQ(topo.technology, static_cast<uint32_t>(QPU_TECH_SUPERCONDUCTING_TRANSMON));
    EXPECT_EQ(topo.num_physical_qubits, 32u);
    EXPECT_GT(topo.num_edges, 0u);
    EXPECT_GT(edges[0].fidelity_ppm, 900000u);

    // 3. Query Calibration over IPC
    struct qpu_qubit_calibration cals[8];
    uint64_t calTimestamp = 0;
    EXPECT_EQ(qpu_get_calibration(client, cals, 8, &calTimestamp), 0);
    EXPECT_GT(calTimestamp, 0u);
    EXPECT_GT(cals[0].t1_ns, 0u);
    EXPECT_GT(cals[0].t2_ns, 0u);
    EXPECT_EQ(cals[0].is_alive, 1u);

    // 4. Query Cryogenic Telemetry over IPC
    struct qpu_cryo_telemetry cryo;
    std::memset(&cryo, 0, sizeof(cryo));
    EXPECT_EQ(qpu_query_cryo(client, &cryo), 0);
    EXPECT_EQ(cryo.stage_50k_mk, 50250u);
    EXPECT_EQ(cryo.stage_4k_mk, 3820u);
    EXPECT_EQ(cryo.stage_still_mk, 845u);
    EXPECT_EQ(cryo.stage_cold_plate_mk, 98u);
    EXPECT_EQ(cryo.stage_mxc_mk, 14u);
    EXPECT_EQ(cryo.stage_mxc_uk, 14250u);
    EXPECT_EQ(cryo.is_cryo_ready, 1u);

    // 5. Allocate 2-qubit isolated partition over IPC
    uint32_t partitionId = 0;
    EXPECT_EQ(qpu_alloc(client, 2, QPU_ALLOC_FLAG_ISOLATED, &partitionId), 0);
    EXPECT_GT(partitionId, 0u);

    // 6. Query Active Partitions over IPC
    struct qpu_partition_entry parts[4];
    uint32_t numParts = 0;
    EXPECT_EQ(qpu_query_partitions(client, parts, 4, &numParts), 0);
    EXPECT_EQ(numParts, 1u);
    EXPECT_EQ(parts[0].partition_id, partitionId);
    EXPECT_EQ(parts[0].num_virtual_qubits, 2u);
    EXPECT_EQ(parts[0].process_id, static_cast<uint32_t>(getpid()));

    // 7. Submit Bell State circuit over IPC
    const char *bellQasm =
        "OPENQASM 2.0;\n"
        "include \"qelib1.inc\";\n"
        "qreg q[2];\n"
        "creg c[2];\n"
        "h q[0];\n"
        "cx q[0], q[1];\n"
        "measure q[0] -> c[0];\n"
        "measure q[1] -> c[1];\n";

    uint64_t jobId = 0;
    EXPECT_EQ(qpu_submit(client, partitionId, bellQasm, 1000, 0, &jobId), 0);
    EXPECT_GT(jobId, 0u);

    // 8. Wait for measurement readout over IPC
    struct qpu_readout_entry results[8];
    uint32_t numEntries = 0;
    uint32_t totalShots = 0;
    EXPECT_EQ(qpu_wait(client, jobId, 1000, results, 8, &numEntries, &totalShots), 0);
    EXPECT_EQ(totalShots, 1000u);
    EXPECT_GE(numEntries, 1u);
    uint32_t shotsSum = 0;
    for (uint32_t i = 0; i < numEntries; ++i) {
        EXPECT_TRUE(results[i].basis_state == 0u || results[i].basis_state == 3u);
        shotsSum += results[i].count;
    }
    EXPECT_EQ(shotsSum, 1000u);

    // 9. Dispatch Active Qubit Reset over IPC
    EXPECT_EQ(qpu_reset_qubit(client, 0, QPU_RESET_MODE_ACTIVE), 0);

    // 10. Free Partition over IPC
    EXPECT_EQ(qpu_free(client, partitionId), 0);

    // Verify partition freed
    EXPECT_EQ(qpu_query_partitions(client, parts, 4, &numParts), 0);
    EXPECT_EQ(numParts, 0u);

    // 11. Verify Live Sysfs & Procfs Exports
    std::ifstream sysfsPhys(testSysfs + "/num_physical_qubits");
    ASSERT_TRUE(sysfsPhys.is_open());
    int sysPhysVal = 0;
    sysfsPhys >> sysPhysVal;
    EXPECT_EQ(sysPhysVal, 32);

    std::ifstream procStatus(testProcfs + "/status");
    ASSERT_TRUE(procStatus.is_open());
    std::string procContent((std::istreambuf_iterator<char>(procStatus)),
                            std::istreambuf_iterator<char>());
    EXPECT_NE(procContent.find("OmniQ QPU Accelerator Subsystem Status"), std::string::npos);
    EXPECT_NE(procContent.find("Total Jobs Completed: 1"), std::string::npos);

    // Cleanup client and server
    qpu_close(client);
    server.stop();
    if (serverThread.joinable()) {
        serverThread.join();
    }

    // Verify socket unlinked
    EXPECT_FALSE(std::filesystem::exists(testSock));
}

// Test 9: Daemon IPC Multi-Tenant Concurrent Clients
TEST(QoSTest, DaemonIPCMultiTenantConcurrent) {
    const std::string testSock = "/tmp/omniq_test_ipc2.sock";
    const std::string testSysfs = "/tmp/omniq_test_sys2/class/accel/qpu0";
    const std::string testProcfs = "/tmp/omniq_test_proc2/qpu";

    DaemonServer server(32, QPU_TECH_SUPERCONDUCTING_TRANSMON, testSock, testSysfs, testProcfs);
    ASSERT_EQ(server.start(), 0);

    std::thread serverThread([&server]() {
        server.run();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Client A and Client B connect to the daemon
    qpu_device_t *clientA = qpu_open(testSock.c_str());
    qpu_device_t *clientB = qpu_open(testSock.c_str());
    ASSERT_NE(clientA, nullptr);
    ASSERT_NE(clientB, nullptr);
    if (!qpu_is_ipc(clientA)) {
        int fdA = server.createClientSocket();
        char fdPathA[32];
        snprintf(fdPathA, sizeof(fdPathA), "fd:%d", fdA);
        qpu_close(clientA);
        clientA = qpu_open(fdPathA);
        ASSERT_NE(clientA, nullptr);
        ASSERT_TRUE(qpu_is_ipc(clientA));
    }
    if (!qpu_is_ipc(clientB)) {
        int fdB = server.createClientSocket();
        char fdPathB[32];
        snprintf(fdPathB, sizeof(fdPathB), "fd:%d", fdB);
        qpu_close(clientB);
        clientB = qpu_open(fdPathB);
        ASSERT_NE(clientB, nullptr);
        ASSERT_TRUE(qpu_is_ipc(clientB));
    }

    // Client A allocates 4 qubits
    uint32_t pidA = 0;
    EXPECT_EQ(qpu_alloc(clientA, 4, QPU_ALLOC_FLAG_ISOLATED, &pidA), 0);
    EXPECT_GT(pidA, 0u);

    // Client B allocates 4 qubits
    uint32_t pidB = 0;
    EXPECT_EQ(qpu_alloc(clientB, 4, QPU_ALLOC_FLAG_ISOLATED, &pidB), 0);
    EXPECT_GT(pidB, 0u);
    EXPECT_NE(pidA, pidB);

    // Query partitions from client A: both partitions visible
    struct qpu_partition_entry parts[8];
    uint32_t numParts = 0;
    EXPECT_EQ(qpu_query_partitions(clientA, parts, 8, &numParts), 0);
    EXPECT_EQ(numParts, 2u);

    // Disjointness check between tenant A and tenant B
    std::unordered_set<uint32_t> physA, guardA, physB, guardB;
    for (uint32_t i = 0; i < numParts; ++i) {
        if (parts[i].partition_id == pidA) {
            for (uint32_t j = 0; j < parts[i].num_physical_qubits; ++j) physA.insert(parts[i].physical_qubits[j]);
            for (uint32_t j = 0; j < parts[i].num_guard_qubits; ++j) guardA.insert(parts[i].guard_qubits[j]);
        } else if (parts[i].partition_id == pidB) {
            for (uint32_t j = 0; j < parts[i].num_physical_qubits; ++j) physB.insert(parts[i].physical_qubits[j]);
            for (uint32_t j = 0; j < parts[i].num_guard_qubits; ++j) guardB.insert(parts[i].guard_qubits[j]);
        }
    }
    // Verify physical qubits are disjoint
    for (uint32_t q : physB) {
        EXPECT_EQ(physA.count(q), 0u);
        EXPECT_EQ(guardA.count(q), 0u);
    }
    for (uint32_t q : physA) {
        EXPECT_EQ(guardB.count(q), 0u);
    }

    // Both submit circuits
    const char *qasmA = "OPENQASM 2.0;\ninclude \"qelib1.inc\";\nqreg q[4];\nx q[0];\n";
    const char *qasmB = "OPENQASM 2.0;\ninclude \"qelib1.inc\";\nqreg q[4];\nx q[1];\n";
    uint64_t jobA = 0, jobB = 0;
    EXPECT_EQ(qpu_submit(clientA, pidA, qasmA, 500, 0, &jobA), 0);
    EXPECT_EQ(qpu_submit(clientB, pidB, qasmB, 500, 0, &jobB), 0);

    struct qpu_readout_entry resEntries[4];
    uint32_t nEnt = 0, totShots = 0;
    EXPECT_EQ(qpu_wait(clientA, jobA, 1000, resEntries, 4, &nEnt, &totShots), 0);
    EXPECT_EQ(totShots, 500u);

    EXPECT_EQ(qpu_wait(clientB, jobB, 1000, resEntries, 4, &nEnt, &totShots), 0);
    EXPECT_EQ(totShots, 500u);

    // Client A frees partition
    EXPECT_EQ(qpu_free(clientA, pidA), 0);
    EXPECT_EQ(qpu_query_partitions(clientB, parts, 8, &numParts), 0);
    EXPECT_EQ(numParts, 1u);
    EXPECT_EQ(parts[0].partition_id, pidB);

    // Client B frees partition
    EXPECT_EQ(qpu_free(clientB, pidB), 0);
    EXPECT_EQ(qpu_query_partitions(clientA, parts, 8, &numParts), 0);
    EXPECT_EQ(numParts, 0u);

    qpu_close(clientA);
    qpu_close(clientB);
    server.stop();
    if (serverThread.joinable()) {
        serverThread.join();
    }
}

// Test 10: Client Standalone Direct Fallback & IPC Error Paths
TEST(QoSTest, ClientFallbackAndIPCErrorPaths) {
    // 1. Target non-existent socket: should fall back cleanly to direct driver mode
    qpu_device_t *fallbackClient = qpu_open("/tmp/nonexistent_socket_9999.sock");
    ASSERT_NE(fallbackClient, nullptr);

    struct qpu_topology_query topo;
    std::memset(&topo, 0, sizeof(topo));
    EXPECT_EQ(qpu_get_topology(fallbackClient, &topo, nullptr, 0), 0);
    EXPECT_EQ(topo.num_physical_qubits, 32u);

    qpu_close(fallbackClient);

    // 2. Explicit force direct mode
    qpu_device_t *directClient = qpu_open(":force-direct:");
    ASSERT_NE(directClient, nullptr);
    struct qpu_cryo_telemetry cryo;
    EXPECT_EQ(qpu_query_cryo(directClient, &cryo), 0);
    EXPECT_EQ(cryo.is_cryo_ready, 1u);
    qpu_close(directClient);

    // 3. Error propagation over IPC
    const std::string testSock = "/tmp/omniq_test_ipc3.sock";
    DaemonServer server(16, QPU_TECH_SUPERCONDUCTING_TRANSMON, testSock);
    ASSERT_EQ(server.start(), 0);
    std::thread serverThread([&server]() { server.run(); });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    qpu_device_t *ipcClient = qpu_open(testSock.c_str());
    ASSERT_NE(ipcClient, nullptr);
    if (!qpu_is_ipc(ipcClient)) {
        int fdIpc = server.createClientSocket();
        char fdPath[32];
        snprintf(fdPath, sizeof(fdPath), "fd:%d", fdIpc);
        qpu_close(ipcClient);
        ipcClient = qpu_open(fdPath);
        ASSERT_NE(ipcClient, nullptr);
        ASSERT_TRUE(qpu_is_ipc(ipcClient));
    }

    // Submitting job to non-existent partition must fail with error
    uint64_t dummyJob = 0;
    EXPECT_NE(qpu_submit(ipcClient, 9999, "OPENQASM 2.0;\n", 100, 0, &dummyJob), 0);

    // Freeing non-existent partition must fail
    EXPECT_NE(qpu_free(ipcClient, 9999), 0);

    // Resetting invalid physical qubit (>= 16) must fail
    EXPECT_NE(qpu_reset_qubit(ipcClient, 100, QPU_RESET_MODE_ACTIVE), 0);

    // Resetting with invalid mode must fail
    EXPECT_NE(qpu_reset_qubit(ipcClient, 0, 99), 0);

    // Requesting more qubits than available in chip must fail
    uint32_t oobPart = 0;
    EXPECT_NE(qpu_alloc(ipcClient, 64, 0, &oobPart), 0);

    qpu_close(ipcClient);
    server.stop();
    if (serverThread.joinable()) {
        serverThread.join();
    }
}
