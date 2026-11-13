/*
 * OmniQ-QOS: Quantum Memory Management Unit (QMMU)
 * include/omniq/qmmu.h
 *
 * Copyright (c) 2026 Quantum Quorum
 */

#ifndef _OMNIQ_QMMU_H
#define _OMNIQ_QMMU_H

#include "../uapi/linux/qpu.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <memory>

namespace omniq {
namespace qos {

enum class QubitState {
    FREE,
    ALLOCATED,
    GUARD_BAND,
    DEFECTIVE
};

struct PhysicalQubitNode {
    int id;
    QubitState state;
    int partitionOwner;      // -1 if free
    double t1_us;
    double t2_us;
    double singleQubitError;
    double frequency_ghz;
    std::vector<int> neighbors;
};

struct PhysicalCouplerEdge {
    int q1;
    int q2;
    double fidelity;         // e.g. 0.995 (99.5%)
    double duration_ns;
};

struct PartitionDescriptor {
    uint32_t partitionId;
    uint32_t processId;
    std::vector<int> physicalQubits;
    std::vector<int> guardQubits;
    std::unordered_map<int, int> virtualToPhysical; // virtualIdx -> physicalId
    uint32_t flags;
    uint64_t allocatedAtNs;
};

class QMMU {
public:
    explicit QMMU(int numPhysicalQubits = 32, uint32_t technology = QPU_TECH_SUPERCONDUCTING_TRANSMON);
    ~QMMU() = default;

    // Topology initialization
    void initializeGridTopology(int rows, int cols);
    void initializeHeavyHexTopology(int numQubits);
    void addCoupler(int q1, int q2, double fidelity = 0.995, double duration_ns = 200.0);
    void setQubitCalibration(int qid, double t1_us, double t2_us, double errorRate, double freq_ghz);
    void markDefective(int qid, bool isDefective = true);

    // Allocation & Freeing (Virtual Memory Management)
    int allocatePartition(uint32_t numVirtualQubits, uint32_t flags, uint32_t processId,
                          uint32_t &outPartitionId, std::vector<int> &outPhysicalQubits);
    bool freePartition(uint32_t partitionId);

    // Virtual-to-Physical Address Translation
    int translateVirtualToPhysical(uint32_t partitionId, int virtualQubit) const;

    // Telemetry & Inspection
    int getNumPhysicalQubits() const;
    int getNumAllocatedQubits() const;
    int getNumGuardBandQubits() const;
    int getNumFreeQubits() const;
    uint32_t getTechnology() const { return technology_; }

    std::vector<PhysicalQubitNode> getPhysicalNodes() const;
    std::vector<PhysicalCouplerEdge> getCouplerEdges() const;
    bool getPartitionInfo(uint32_t partitionId, PartitionDescriptor &outDesc) const;
    std::vector<PartitionDescriptor> getAllActivePartitions() const;

private:
    std::vector<int> findBestConnectedCluster(int requiredQubits, uint32_t flags,
                                              std::vector<int> &outGuards);
    double scoreCandidateCluster(const std::vector<int> &cluster) const;

    mutable std::mutex mmMutex_;
    int numPhysicalQubits_;
    uint32_t technology_;
    uint32_t nextPartitionId_;
    std::vector<PhysicalQubitNode> nodes_;
    std::vector<PhysicalCouplerEdge> edges_;
    std::unordered_map<uint32_t, PartitionDescriptor> partitions_;
};

} // namespace qos
} // namespace omniq

#endif /* _OMNIQ_QMMU_H */

