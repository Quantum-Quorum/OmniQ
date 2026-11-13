/*
 * OmniQ-QOS: Quantum Memory Management Unit (QMMU) Implementation
 * src/qmmu/QMMU.cpp
 *
 * Copyright (c) 2026 Quantum Quorum
 */

#include "../../include/omniq/qmmu.h"
#include <algorithm>
#include <queue>
#include <chrono>
#include <random>
#include <cmath>

namespace omniq {
namespace qos {

QMMU::QMMU(int numPhysicalQubits, uint32_t technology)
    : numPhysicalQubits_(numPhysicalQubits), technology_(technology),
      nextPartitionId_(1) {
    nodes_.resize(numPhysicalQubits_);
    for (int i = 0; i < numPhysicalQubits_; ++i) {
        nodes_[i].id = i;
        nodes_[i].state = QubitState::FREE;
        nodes_[i].partitionOwner = -1;
        nodes_[i].t1_us = 120.0 + (i % 7) * 5.0;       // ~120 - 150 us
        nodes_[i].t2_us = 90.0 + (i % 5) * 6.0;        // ~90 - 114 us
        nodes_[i].singleQubitError = 0.0003 + (i % 3) * 0.0001; // ~0.03%
        nodes_[i].frequency_ghz = 5.0 + (i * 0.035);
    }

    // Default to a 2D mesh grid
    int cols = static_cast<int>(std::sqrt(numPhysicalQubits_));
    if (cols < 2) cols = 2;
    int rows = (numPhysicalQubits_ + cols - 1) / cols;
    initializeGridTopology(rows, cols);
}

void QMMU::initializeGridTopology(int rows, int cols) {
    std::lock_guard<std::mutex> lock(mmMutex_);
    edges_.clear();
    for (auto &n : nodes_) {
        n.neighbors.clear();
    }

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int q1 = r * cols + c;
            if (q1 >= numPhysicalQubits_) continue;

            // Horizontal neighbor (right)
            if (c + 1 < cols) {
                int q2 = r * cols + (c + 1);
                if (q2 < numPhysicalQubits_) {
                    nodes_[q1].neighbors.push_back(q2);
                    nodes_[q2].neighbors.push_back(q1);
                    edges_.push_back({q1, q2, 0.994 + (q1 % 5) * 0.001, 180.0});
                }
            }

            // Vertical neighbor (down)
            if (r + 1 < rows) {
                int q2 = (r + 1) * cols + c;
                if (q2 < numPhysicalQubits_) {
                    nodes_[q1].neighbors.push_back(q2);
                    nodes_[q2].neighbors.push_back(q1);
                    edges_.push_back({q1, q2, 0.993 + (q2 % 4) * 0.001, 210.0});
                }
            }
        }
    }
}

void QMMU::initializeHeavyHexTopology(int numQubits) {
    std::lock_guard<std::mutex> lock(mmMutex_);
    edges_.clear();
    for (auto &n : nodes_) {
        n.neighbors.clear();
    }

    // Connect in a heavy-hex / ring chain
    for (int i = 0; i < numQubits - 1; ++i) {
        nodes_[i].neighbors.push_back(i + 1);
        nodes_[i + 1].neighbors.push_back(i);
        edges_.push_back({i, i + 1, 0.996, 160.0});

        // Cross-bridges every 4 qubits
        if (i % 4 == 0 && (i + 4) < numQubits) {
            nodes_[i].neighbors.push_back(i + 4);
            nodes_[i + 4].neighbors.push_back(i);
            edges_.push_back({i, i + 4, 0.992, 220.0});
        }
    }
}

void QMMU::addCoupler(int q1, int q2, double fidelity, double duration_ns) {
    std::lock_guard<std::mutex> lock(mmMutex_);
    if (q1 >= 0 && q1 < numPhysicalQubits_ && q2 >= 0 && q2 < numPhysicalQubits_) {
        nodes_[q1].neighbors.push_back(q2);
        nodes_[q2].neighbors.push_back(q1);
        edges_.push_back({q1, q2, fidelity, duration_ns});
    }
}

void QMMU::setQubitCalibration(int qid, double t1_us, double t2_us, double errorRate, double freq_ghz) {
    std::lock_guard<std::mutex> lock(mmMutex_);
    if (qid >= 0 && qid < numPhysicalQubits_) {
        nodes_[qid].t1_us = t1_us;
        nodes_[qid].t2_us = t2_us;
        nodes_[qid].singleQubitError = errorRate;
        nodes_[qid].frequency_ghz = freq_ghz;
    }
}

void QMMU::markDefective(int qid, bool isDefective) {
    std::lock_guard<std::mutex> lock(mmMutex_);
    if (qid >= 0 && qid < numPhysicalQubits_) {
        nodes_[qid].state = isDefective ? QubitState::DEFECTIVE : QubitState::FREE;
    }
}

double QMMU::scoreCandidateCluster(const std::vector<int> &cluster) const {
    if (cluster.empty()) return 0.0;
    double score = 0.0;
    for (int q : cluster) {
        score += nodes_[q].t1_us * 0.3 + nodes_[q].t2_us * 0.4 - (nodes_[q].singleQubitError * 10000.0);
    }
    return score;
}

std::vector<int> QMMU::findBestConnectedCluster(int requiredQubits, uint32_t flags,
                                                std::vector<int> &outGuards) {
    outGuards.clear();
    std::vector<int> bestCluster;
    double bestScore = -1e9;

    bool isolated = (flags & QPU_ALLOC_FLAG_ISOLATED);

    // Try starting BFS from every available free qubit
    for (int startQ = 0; startQ < numPhysicalQubits_; ++startQ) {
        if (nodes_[startQ].state != QubitState::FREE)
            continue;

        std::vector<int> currentCluster;
        std::unordered_set<int> visited;
        std::queue<int> q;

        q.push(startQ);
        visited.insert(startQ);

        while (!q.empty() && static_cast<int>(currentCluster.size()) < requiredQubits) {
            int curr = q.front();
            q.pop();

            if (nodes_[curr].state == QubitState::FREE) {
                currentCluster.push_back(curr);
            }

            for (int nbr : nodes_[curr].neighbors) {
                if (visited.find(nbr) == visited.end()) {
                    visited.insert(nbr);
                    if (nodes_[nbr].state == QubitState::FREE) {
                        q.push(nbr);
                    }
                }
            }
        }

        if (static_cast<int>(currentCluster.size()) == requiredQubits) {
            double s = scoreCandidateCluster(currentCluster);
            if (s > bestScore) {
                bestScore = s;
                bestCluster = currentCluster;
            }
        }
    }

    if (bestCluster.empty()) {
        return {};
    }

    // Determine guard band perimeter if isolated flag is set
    if (isolated) {
        std::unordered_set<int> inCluster(bestCluster.begin(), bestCluster.end());
        std::unordered_set<int> guardSet;

        for (int q : bestCluster) {
            for (int nbr : nodes_[q].neighbors) {
                if (inCluster.find(nbr) == inCluster.end() && nodes_[nbr].state == QubitState::FREE) {
                    guardSet.insert(nbr);
                }
            }
        }
        outGuards.assign(guardSet.begin(), guardSet.end());
    }

    return bestCluster;
}

int QMMU::allocatePartition(uint32_t numVirtualQubits, uint32_t flags, uint32_t processId,
                            uint32_t &outPartitionId, std::vector<int> &outPhysicalQubits) {
    std::lock_guard<std::mutex> lock(mmMutex_);
    if (numVirtualQubits == 0 || numVirtualQubits > static_cast<uint32_t>(numPhysicalQubits_)) {
        return -22; // -EINVAL
    }

    std::vector<int> guards;
    std::vector<int> cluster = findBestConnectedCluster(numVirtualQubits, flags, guards);
    if (cluster.size() < numVirtualQubits) {
        return -12; // -ENOMEM: insufficient contiguous quantum memory
    }

    uint32_t pid = nextPartitionId_++;
    PartitionDescriptor desc;
    desc.partitionId = pid;
    desc.processId = processId;
    desc.physicalQubits = cluster;
    desc.guardQubits = guards;
    desc.flags = flags;
    desc.allocatedAtNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
                             std::chrono::system_clock::now().time_since_epoch()).count();

    // Setup virtual-to-physical translation table
    for (size_t v = 0; v < cluster.size(); ++v) {
        int phys = cluster[v];
        desc.virtualToPhysical[static_cast<int>(v)] = phys;
        nodes_[phys].state = QubitState::ALLOCATED;
        nodes_[phys].partitionOwner = static_cast<int>(pid);
    }

    // Lock guard band qubits
    for (int g : guards) {
        nodes_[g].state = QubitState::GUARD_BAND;
        nodes_[g].partitionOwner = static_cast<int>(pid);
    }

    partitions_[pid] = desc;
    outPartitionId = pid;
    outPhysicalQubits = cluster;
    return 0;
}

bool QMMU::freePartition(uint32_t partitionId) {
    std::lock_guard<std::mutex> lock(mmMutex_);
    auto it = partitions_.find(partitionId);
    if (it == partitions_.end()) {
        return false;
    }

    // Free physical data qubits
    for (int q : it->second.physicalQubits) {
        nodes_[q].state = QubitState::FREE;
        nodes_[q].partitionOwner = -1;
    }

    // Free guard band qubits
    for (int g : it->second.guardQubits) {
        nodes_[g].state = QubitState::FREE;
        nodes_[g].partitionOwner = -1;
    }

    partitions_.erase(it);
    return true;
}

int QMMU::translateVirtualToPhysical(uint32_t partitionId, int virtualQubit) const {
    std::lock_guard<std::mutex> lock(mmMutex_);
    auto it = partitions_.find(partitionId);
    if (it == partitions_.end()) {
        return -1;
    }
    auto mapIt = it->second.virtualToPhysical.find(virtualQubit);
    if (mapIt == it->second.virtualToPhysical.end()) {
        return -1;
    }
    return mapIt->second;
}

int QMMU::getNumPhysicalQubits() const {
    std::lock_guard<std::mutex> lock(mmMutex_);
    return numPhysicalQubits_;
}

int QMMU::getNumAllocatedQubits() const {
    std::lock_guard<std::mutex> lock(mmMutex_);
    int count = 0;
    for (const auto &n : nodes_) {
        if (n.state == QubitState::ALLOCATED) count++;
    }
    return count;
}

int QMMU::getNumGuardBandQubits() const {
    std::lock_guard<std::mutex> lock(mmMutex_);
    int count = 0;
    for (const auto &n : nodes_) {
        if (n.state == QubitState::GUARD_BAND) count++;
    }
    return count;
}

int QMMU::getNumFreeQubits() const {
    std::lock_guard<std::mutex> lock(mmMutex_);
    int count = 0;
    for (const auto &n : nodes_) {
        if (n.state == QubitState::FREE) count++;
    }
    return count;
}

std::vector<PhysicalQubitNode> QMMU::getPhysicalNodes() const {
    std::lock_guard<std::mutex> lock(mmMutex_);
    return nodes_;
}

std::vector<PhysicalCouplerEdge> QMMU::getCouplerEdges() const {
    std::lock_guard<std::mutex> lock(mmMutex_);
    return edges_;
}

bool QMMU::getPartitionInfo(uint32_t partitionId, PartitionDescriptor &outDesc) const {
    std::lock_guard<std::mutex> lock(mmMutex_);
    auto it = partitions_.find(partitionId);
    if (it == partitions_.end()) {
        return false;
    }
    outDesc = it->second;
    return true;
}

std::vector<PartitionDescriptor> QMMU::getAllActivePartitions() const {
    std::lock_guard<std::mutex> lock(mmMutex_);
    std::vector<PartitionDescriptor> result;
    for (const auto &p : partitions_) {
        result.push_back(p.second);
    }
    return result;
}

} // namespace qos
} // namespace omniq
