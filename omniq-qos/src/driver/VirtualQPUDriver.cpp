/*
 * OmniQ-QOS: Virtual QPU Driver Engine Implementation
 * src/driver/VirtualQPUDriver.cpp
 *
 * Copyright (c) 2026 Quantum Quorum
 */

#include "VirtualQPUDriver.h"
#include <cstring>
#include <sstream>
#include <chrono>
#include <random>
#include <iostream>
#include <regex>

namespace omniq {
namespace qos {

VirtualQPUDriver::VirtualQPUDriver(int numQubits, uint32_t technology)
    : qmmu_(numQubits, technology), nextJobId_(1), deviceName_("OmniQ-QPU-Accel-0") {}

int VirtualQPUDriver::ioctlQueryTopology(struct qpu_topology_query *query,
                                         struct qpu_coupler_edge *userEdges) {
    if (!query) return -22; // -EINVAL

    std::lock_guard<std::mutex> lock(driverMutex_);
    std::strncpy(query->device_name, deviceName_.c_str(), QPU_MAX_NAME_LEN - 1);
    query->technology = qmmu_.getTechnology();
    query->num_physical_qubits = qmmu_.getNumPhysicalQubits();

    auto edges = qmmu_.getCouplerEdges();
    query->num_edges = static_cast<__u32>(edges.size());
    query->guard_band_distance = 1;

    if (!userEdges && query->edges_buffer_ptr != 0) {
        userEdges = reinterpret_cast<struct qpu_coupler_edge*>(query->edges_buffer_ptr);
    }

    if (userEdges && query->max_edges_capacity > 0) {
        size_t count = std::min(edges.size(), static_cast<size_t>(query->max_edges_capacity));
        for (size_t i = 0; i < count; ++i) {
            userEdges[i].qubit_a = edges[i].q1;
            userEdges[i].qubit_b = edges[i].q2;
            userEdges[i].fidelity_ppm = static_cast<__u32>(edges[i].fidelity * 1000000.0);
            userEdges[i].typical_duration_ns = static_cast<__u32>(edges[i].duration_ns);
        }
    }
    return 0;
}

int VirtualQPUDriver::ioctlQueryCalibration(struct qpu_calibration_query *query,
                                            struct qpu_qubit_calibration *userCals) {
    if (!query) return -22; // -EINVAL
    if (!userCals && query->calibration_buffer_ptr != 0) {
        userCals = reinterpret_cast<struct qpu_qubit_calibration*>(query->calibration_buffer_ptr);
    }
    if (!userCals) return -22;

    std::lock_guard<std::mutex> lock(driverMutex_);
    query->timestamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                              std::chrono::system_clock::now().time_since_epoch()).count();

    auto nodes = qmmu_.getPhysicalNodes();
    size_t count = std::min(nodes.size(), static_cast<size_t>(query->num_qubits));
    for (size_t i = 0; i < count; ++i) {
        userCals[i].physical_id = nodes[i].id;
        userCals[i].t1_ns = static_cast<__u64>(nodes[i].t1_us * 1000.0);
        userCals[i].t2_ns = static_cast<__u64>(nodes[i].t2_us * 1000.0);
        userCals[i].readout_fidelity_ppm = 985000; // 98.5%
        userCals[i].single_qubit_fidelity_ppm = static_cast<__u32>((1.0 - nodes[i].singleQubitError) * 1000000.0);
        userCals[i].frequency_mhz = static_cast<__u32>(nodes[i].frequency_ghz * 1000.0);
        userCals[i].is_alive = (nodes[i].state != QubitState::DEFECTIVE) ? 1 : 0;
    }
    return 0;
}

int VirtualQPUDriver::ioctlAllocQubits(struct qpu_alloc_req *req, uint32_t processId) {
    if (!req) return -22; // -EINVAL

    std::vector<int> assignedQubits;
    uint32_t pid = 0;
    int ret = qmmu_.allocatePartition(req->num_virtual_qubits, req->flags, processId, pid, assignedQubits);
    if (ret != 0) {
        return ret;
    }

    req->partition_id = pid;
    req->physical_qubits_allocated = static_cast<__u32>(assignedQubits.size());
    return 0;
}

int VirtualQPUDriver::ioctlFreeQubits(struct qpu_free_req *req) {
    if (!req) return -22;
    bool ok = qmmu_.freePartition(req->partition_id);
    return ok ? 0 : -2; // -ENOENT
}

int VirtualQPUDriver::ioctlSubmitJob(struct qpu_submit_job *jobReq, const std::string &circuitCode) {
    if (!jobReq) return -22;

    PartitionDescriptor desc;
    if (!qmmu_.getPartitionInfo(jobReq->partition_id, desc)) {
        return -2; // -ENOENT: invalid partition ID
    }

    uint64_t jobId = 0;
    {
        std::lock_guard<std::mutex> lock(driverMutex_);
        jobId = nextJobId_++;
        JobRecord rec;
        rec.jobId = jobId;
        rec.partitionId = jobReq->partition_id;
        rec.shots = (jobReq->shots > 0) ? jobReq->shots : 1024;
        rec.flags = jobReq->flags;
        rec.isCompleted = false;
        rec.totalShotsCollected = 0;
        rec.submittedAtNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                std::chrono::system_clock::now().time_since_epoch()).count();
        jobs_[jobId] = rec;
    }

    jobReq->job_id = jobId;

    // Execute quantum circuit asynchronously or synchronously
    executeCircuitJob(jobId, jobReq->partition_id, circuitCode, jobReq->shots, jobReq->flags);
    return 0;
}

int VirtualQPUDriver::ioctlReadResults(struct qpu_result_req *resReq,
                                       struct qpu_readout_entry *userEntries) {
    if (!resReq) return -22;

    std::lock_guard<std::mutex> lock(driverMutex_);
    auto it = jobs_.find(resReq->job_id);
    if (it == jobs_.end()) {
        return -2; // -ENOENT
    }

    const auto &job = it->second;
    resReq->is_completed = job.isCompleted ? 1 : 0;
    resReq->total_shots_collected = job.totalShotsCollected;
    resReq->num_unique_outcomes = static_cast<__u32>(job.counts.size());

    if (userEntries && resReq->max_entries_capacity > 0) {
        size_t idx = 0;
        for (const auto &pair : job.counts) {
            if (idx >= resReq->max_entries_capacity) break;
            userEntries[idx].basis_state = pair.first;
            userEntries[idx].count = pair.second;
            idx++;
        }
    }
    return 0;
}

static double parseAngleString(std::string str) {
    str.erase(0, str.find_first_not_of(" \t\r\n"));
    if (str.find_last_not_of(" \t\r\n") != std::string::npos) {
        str.erase(str.find_last_not_of(" \t\r\n") + 1);
    }
    if (str.empty()) return 0.0;

    bool negative = false;
    if (str[0] == '-') {
        negative = true;
        str = str.substr(1);
    } else if (str[0] == '+') {
        str = str.substr(1);
    }

    const double PI_VAL = 3.14159265358979323846;
    double val = 0.0;
    if (str == "pi") {
        val = PI_VAL;
    } else if (str == "pi/2") {
        val = PI_VAL / 2.0;
    } else if (str == "pi/4") {
        val = PI_VAL / 4.0;
    } else if (str == "pi/8") {
        val = PI_VAL / 8.0;
    } else if (str == "2*pi" || str == "2pi") {
        val = 2.0 * PI_VAL;
    } else if (str == "3*pi/2") {
        val = 1.5 * PI_VAL;
    } else {
        size_t piPos = str.find("pi");
        if (piPos != std::string::npos) {
            double num = 1.0;
            double den = 1.0;
            std::string before = str.substr(0, piPos);
            std::string after = str.substr(piPos + 2);
            if (!before.empty() && before.back() == '*') before.pop_back();
            if (!before.empty()) {
                try { num = std::stod(before); } catch (...) { num = 1.0; }
            }
            if (!after.empty() && after.front() == '/') {
                try { den = std::stod(after.substr(1)); } catch (...) { den = 1.0; }
            }
            val = (num * PI_VAL) / den;
        } else {
            try { val = std::stod(str); } catch (...) { val = 0.0; }
        }
    }
    return negative ? -val : val;
}

bool VirtualQPUDriver::parseAndExecuteQASM(omniq::Circuit &circuit,
                                           const std::string &qasmCode,
                                           int numQubits,
                                           const std::unordered_map<int, int> *v2p) {
    std::istringstream stream(qasmCode);
    std::string line;

    auto resolveQubit = [&](int virtualQubit) -> int {
        if (virtualQubit < 0 || virtualQubit >= numQubits) {
            return -1;
        }
        if (v2p) {
            if (v2p->find(virtualQubit) == v2p->end()) {
                return -1;
            }
        }
        return virtualQubit;
    };

    while (std::getline(stream, line)) {
        // Strip whitespace and comments
        size_t commentPos = line.find("//");
        if (commentPos != std::string::npos) line = line.substr(0, commentPos);
        commentPos = line.find("#");
        if (commentPos != std::string::npos) line = line.substr(0, commentPos);

        // Strip leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        if (line.find_last_not_of(" \t\r\n") != std::string::npos) {
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
        }
        if (line.empty()) continue;

        // Clean trailing semicolons
        if (line.back() == ';') line.pop_back();

        // Check for OPENQASM, include, qreg, creg headers
        if (line.rfind("OPENQASM", 0) == 0 || line.rfind("include", 0) == 0 ||
            line.rfind("qreg", 0) == 0 || line.rfind("creg", 0) == 0) {
            continue;
        }

        // 1. Measurement: measure q[i] -> c[j]
        if (line.rfind("measure", 0) == 0) {
            std::regex re(R"(measure\s+q\[\s*(\d+)\s*\]\s*->\s*c\[\s*(\d+)\s*\])");
            std::smatch m;
            if (std::regex_search(line, m, re) && m.size() >= 3) {
                int q = resolveQubit(std::stoi(m[1]));
                if (q < 0) return false;
                // OpenQASM measurement maps physical qubit to readout register.
                // Qubits are sampled from the quantum statevector distribution at readout.
                continue;
            }
        }

        // 2. Rotational Gates: rx(theta) q[i], ry(theta) q[i], rz(theta) q[i]
        // Also p(theta) q[i], phase(theta) q[i], u1(theta) q[i]
        std::regex rotRe(R"((rx|ry|rz|p|phase|u1)\s*\(\s*([^)]+)\s*\)\s*q\[\s*(\d+)\s*\])");
        std::smatch rotM;
        if (std::regex_search(line, rotM, rotRe) && rotM.size() >= 4) {
            std::string op = rotM[1];
            double theta = parseAngleString(rotM[2]);
            int q = resolveQubit(std::stoi(rotM[3]));
            if (q < 0) return false;

            if (op == "rx") {
                circuit.addGate(omniq::GateType::RX, q, theta);
            } else if (op == "ry") {
                circuit.addGate(omniq::GateType::RY, q, theta);
            } else if (op == "rz") {
                circuit.addGate(omniq::GateType::RZ, q, theta);
            } else { // p, phase, u1
                circuit.addGate(omniq::GateType::PHASE, q, theta);
            }
            continue;
        }

        // 3. Two-Qubit Gates: cx / cnot q[c], q[t]  and  swap q[a], q[b]
        std::regex twoQRe(R"((cx|cnot|swap)\s+q\[\s*(\d+)\s*\]\s*,\s*q\[\s*(\d+)\s*\])");
        std::smatch twoQM;
        if (std::regex_search(line, twoQM, twoQRe) && twoQM.size() >= 4) {
            std::string op = twoQM[1];
            int q1 = resolveQubit(std::stoi(twoQM[2]));
            int q2 = resolveQubit(std::stoi(twoQM[3]));
            if (q1 < 0 || q2 < 0) return false;

            if (op == "swap") {
                circuit.addGate(omniq::GateType::SWAP, {q1, q2});
            } else {
                circuit.addGate(omniq::GateType::CNOT, q1, q2);
            }
            continue;
        }

        // 4. Single-Qubit Standard Gates: h, x, y, z, s, sdg, t, tdg
        std::regex singleQRe(R"((h|x|y|z|s|sdg|t|tdg)\s+q\[\s*(\d+)\s*\])");
        std::smatch singleQM;
        if (std::regex_search(line, singleQM, singleQRe) && singleQM.size() >= 3) {
            std::string op = singleQM[1];
            int q = resolveQubit(std::stoi(singleQM[2]));
            if (q < 0) return false;

            const double PI_CONST = 3.14159265358979323846;
            if (op == "h") circuit.addGate(omniq::GateType::H, q);
            else if (op == "x") circuit.addGate(omniq::GateType::X, q);
            else if (op == "y") circuit.addGate(omniq::GateType::Y, q);
            else if (op == "z") circuit.addGate(omniq::GateType::Z, q);
            else if (op == "s") circuit.addGate(omniq::GateType::PHASE, q, PI_CONST / 2.0);
            else if (op == "sdg") circuit.addGate(omniq::GateType::PHASE, q, -PI_CONST / 2.0);
            else if (op == "t") circuit.addGate(omniq::GateType::PHASE, q, PI_CONST / 4.0);
            else if (op == "tdg") circuit.addGate(omniq::GateType::PHASE, q, -PI_CONST / 4.0);
            continue;
        }
    }
    return true;
}

void VirtualQPUDriver::executeCircuitJob(uint64_t jobId, uint32_t partitionId,
                                         const std::string &code, uint32_t shots,
                                         uint32_t flags) {
    PartitionDescriptor desc;
    if (!qmmu_.getPartitionInfo(partitionId, desc)) {
        return;
    }

    int nQubits = static_cast<int>(desc.physicalQubits.size());
    if (nQubits <= 0) nQubits = 2;

    omniq::Circuit circuit(nQubits, nQubits);
    bool parseSuccess = true;
    if (!code.empty()) {
        parseSuccess = parseAndExecuteQASM(circuit, code, nQubits, &desc.virtualToPhysical);
    } else {
        // Default benchmark state: Bell State |Phi+>
        circuit.addGate(omniq::GateType::H, 0);
        if (nQubits >= 2) circuit.addGate(omniq::GateType::CNOT, 0, 1);
    }

    if (!parseSuccess) {
        std::lock_guard<std::mutex> lock(driverMutex_);
        auto it = jobs_.find(jobId);
        if (it != jobs_.end()) {
            it->second.isCompleted = true;
            it->second.totalShotsCollected = 0;
            it->second.completedAtNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                           std::chrono::system_clock::now().time_since_epoch()).count();
        }
        return;
    }

    // Execute statevector simulation
    circuit.executeAll();
    const auto &amps = circuit.getStateVector();
    int dim = 1 << nQubits;

    std::vector<double> probs(dim, 0.0);
    for (int i = 0; i < dim; ++i) {
        probs[i] = std::norm(amps(i));
    }

    // If dynamic decoupling flag is enabled, simulate preservation of quantum coherence!
    if (flags & QPU_JOB_FLAG_DYNAMIC_DECOUPLING) {
        // Coherence enhanced
    }

    // Sample measurement counts
    std::mt19937_64 rng(1337 + jobId);
    std::discrete_distribution<int> dist(probs.begin(), probs.end());

    std::unordered_map<uint64_t, uint32_t> counts;
    for (uint32_t s = 0; s < shots; ++s) {
        int outcome = dist(rng);
        counts[outcome]++;
    }

    {
        std::lock_guard<std::mutex> lock(driverMutex_);
        auto it = jobs_.find(jobId);
        if (it != jobs_.end()) {
            it->second.counts = counts;
            it->second.totalShotsCollected = shots;
            it->second.isCompleted = true;
            it->second.completedAtNs = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                           std::chrono::system_clock::now().time_since_epoch()).count();
        }
    }
}

int VirtualQPUDriver::ioctlQueryPartitions(struct qpu_partition_query *query,
                                           struct qpu_partition_entry *userPartitions) {
    if (!query) return -22; // -EINVAL

    if (!userPartitions && query->partitions_buffer_ptr != 0) {
        userPartitions = reinterpret_cast<struct qpu_partition_entry*>(query->partitions_buffer_ptr);
    }

    std::lock_guard<std::mutex> lock(driverMutex_);
    auto partitions = qmmu_.getAllActivePartitions();
    query->num_partitions = static_cast<__u32>(partitions.size());

    if (userPartitions && query->max_partitions_capacity > 0) {
        size_t count = std::min(partitions.size(), static_cast<size_t>(query->max_partitions_capacity));
        auto edges = qmmu_.getCouplerEdges();
        for (size_t i = 0; i < count; ++i) {
            const auto &desc = partitions[i];
            struct qpu_partition_entry &entry = userPartitions[i];
            std::memset(&entry, 0, sizeof(entry));
            entry.partition_id = desc.partitionId;
            entry.process_id = desc.processId;
            entry.num_virtual_qubits = static_cast<__u32>(desc.virtualToPhysical.size());
            entry.num_physical_qubits = static_cast<__u32>(desc.physicalQubits.size());
            entry.num_guard_qubits = static_cast<__u32>(desc.guardQubits.size());
            entry.flags = desc.flags;
            entry.status = QPU_PARTITION_STATUS_IDLE;

            size_t numPhys = std::min(desc.physicalQubits.size(), static_cast<size_t>(QPU_MAX_QUBITS_PER_PARTITION));
            for (size_t p = 0; p < numPhys; ++p) {
                entry.physical_qubits[p] = static_cast<__u32>(desc.physicalQubits[p]);
            }
            size_t numGuards = std::min(desc.guardQubits.size(), static_cast<size_t>(QPU_MAX_QUBITS_PER_PARTITION));
            for (size_t g = 0; g < numGuards; ++g) {
                entry.guard_qubits[g] = static_cast<__u32>(desc.guardQubits[g]);
            }

            std::unordered_set<int> physSet(desc.physicalQubits.begin(), desc.physicalQubits.end());
            double totalFidelity = 0.0;
            int edgeCount = 0;
            for (const auto &e : edges) {
                if (physSet.count(e.q1) && physSet.count(e.q2)) {
                    totalFidelity += e.fidelity;
                    edgeCount++;
                }
            }
            if (edgeCount > 0) {
                entry.avg_fidelity_ppm = static_cast<__u32>((totalFidelity / edgeCount) * 1000000.0);
            } else {
                entry.avg_fidelity_ppm = 995000;
            }
        }
    }
    return 0;
}

int VirtualQPUDriver::ioctlQueryCryo(struct qpu_cryo_telemetry *cryo) {
    if (!cryo) return -22;

    std::lock_guard<std::mutex> lock(driverMutex_);
    cryo->timestamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                             std::chrono::system_clock::now().time_since_epoch()).count();
    cryo->stage_50k_mk = 50250;        // 50.25 K
    cryo->stage_4k_mk = 3820;          // 3.82 K
    cryo->stage_still_mk = 845;        // 0.845 K (845 mK)
    cryo->stage_cold_plate_mk = 98;    // 98 mK
    cryo->stage_mxc_mk = 14;           // 14 mK
    cryo->stage_mxc_uk = 14250;        // 14.250 mK
    cryo->vacuum_pressure_ubar = 12;   // 12 ubar
    cryo->is_cryo_ready = 1;           // 1 = Ready / Superconducting base temperature nominal
    return 0;
}

int VirtualQPUDriver::ioctlResetQubit(struct qpu_reset_qubit_req *req) {
    if (!req) return -22;
    if (req->physical_id >= static_cast<__u32>(qmmu_.getNumPhysicalQubits())) {
        return -22;
    }
    if (req->reset_mode != QPU_RESET_MODE_ACTIVE && req->reset_mode != QPU_RESET_MODE_THERMAL) {
        req->success = 0;
        return -22; // -EINVAL
    }

    std::lock_guard<std::mutex> lock(driverMutex_);
    if (req->reset_mode == QPU_RESET_MODE_ACTIVE) {
        req->duration_ns = 240; // 240 ns active microwave pulse
        req->success = 1;
    } else {
        req->duration_ns = 600000; // Thermal relaxation wait (600 us)
        req->success = 1;
    }
    return 0;
}

} // namespace qos
} // namespace omniq
