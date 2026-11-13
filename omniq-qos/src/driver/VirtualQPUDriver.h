/*
 * OmniQ-QOS: Virtual QPU Driver Engine
 * src/driver/VirtualQPUDriver.h
 *
 * Copyright (c) 2026 Quantum Quorum
 */

#ifndef _OMNIQ_VIRTUAL_QPU_DRIVER_H
#define _OMNIQ_VIRTUAL_QPU_DRIVER_H

#include "../../include/uapi/linux/qpu.h"
#include "../../include/omniq/qmmu.h"
#include "omniq/Circuit.h"
#include "omniq/Statevector.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>

namespace omniq {
namespace qos {

struct JobRecord {
    uint64_t jobId;
    uint32_t partitionId;
    uint32_t shots;
    uint32_t flags;
    bool isCompleted;
    std::unordered_map<uint64_t, uint32_t> counts; // basisState -> count
    uint32_t totalShotsCollected;
    uint64_t submittedAtNs;
    uint64_t completedAtNs;
};

class VirtualQPUDriver {
public:
    explicit VirtualQPUDriver(int numQubits = 32, uint32_t technology = QPU_TECH_SUPERCONDUCTING_TRANSMON);
    ~VirtualQPUDriver() = default;

    // Emulated Kernel IOCTL Handlers
    int ioctlQueryTopology(struct qpu_topology_query *query, struct qpu_coupler_edge *userEdges);
    int ioctlQueryCalibration(struct qpu_calibration_query *query, struct qpu_qubit_calibration *userCals);
    int ioctlAllocQubits(struct qpu_alloc_req *req, uint32_t processId = 1000);
    int ioctlFreeQubits(struct qpu_free_req *req);
    int ioctlSubmitJob(struct qpu_submit_job *jobReq, const std::string &circuitCode);
    int ioctlReadResults(struct qpu_result_req *resReq, struct qpu_readout_entry *userEntries);

    QMMU &getQMMU() { return qmmu_; }
    const QMMU &getQMMU() const { return qmmu_; }

private:
    void executeCircuitJob(uint64_t jobId, uint32_t partitionId, const std::string &code, uint32_t shots, uint32_t flags);
    bool parseAndExecuteQASM(omniq::Circuit &circuit, const std::string &qasmCode, int numQubits);

    mutable std::mutex driverMutex_;
    QMMU qmmu_;
    uint64_t nextJobId_;
    std::string deviceName_;
    std::unordered_map<uint64_t, JobRecord> jobs_;
};

} // namespace qos
} // namespace omniq

#endif /* _OMNIQ_VIRTUAL_QPU_DRIVER_H */

