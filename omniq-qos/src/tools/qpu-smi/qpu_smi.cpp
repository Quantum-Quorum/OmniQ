/*
 * OmniQ-QOS: Quantum Processing Unit System Management Interface (qpu-smi)
 * src/tools/qpu-smi/qpu_smi.cpp
 *
 * Copyright (c) 2026 Quantum Quorum
 */

#include "omniq/qpu.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <unistd.h>

// ANSI Colors
#define ANSI_RESET   "\033[0m"
#define ANSI_BOLD    "\033[1m"
#define ANSI_DIM     "\033[2m"
#define ANSI_CYAN    "\033[36m"
#define ANSI_BCYAN   "\033[1;36m"
#define ANSI_GREEN   "\033[32m"
#define ANSI_BGREEN  "\033[1;32m"
#define ANSI_YELLOW  "\033[33m"
#define ANSI_BYELLOW "\033[1;33m"
#define ANSI_RED     "\033[31m"
#define ANSI_BRED    "\033[1;31m"
#define ANSI_WHITE   "\033[37m"
#define ANSI_BWHITE  "\033[1;37m"
#define ANSI_GRAY    "\033[90m"
#define ANSI_MAGENTA "\033[35m"

static volatile bool g_running = true;

static void sigHandler(int) {
    g_running = false;
}

static void printHelp(const char *prog) {
    std::cout << "OmniQ System Management Interface (qpu-smi) v0.1.0-alpha\n";
    std::cout << "Usage: " << prog << " [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  (no args)             Display real-time QPU status snapshot\n";
    std::cout << "  -w, --watch [sec]     Continuously monitor QPU with live dashboard (default: 1s)\n";
    std::cout << "  -q, --query-qubits    Display full physical qubit calibration metrics table\n";
    std::cout << "  -c, --query-cryo      Display cryogenic dilution refrigerator stage temperatures\n";
    std::cout << "  -p, --partitions      Display active multi-tenant spatial partitions table\n";
    std::cout << "  -r, --reset <qid>     Dispatch active microwave resonator reset pulse to qubit <qid>\n";
    std::cout << "  --format=json         Output telemetry in machine-readable JSON format\n";
    std::cout << "  --device <path>       Specify custom QPU device node or socket path\n";
    std::cout << "  -h, --help            Show this help documentation\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << prog << "\n";
    std::cout << "  " << prog << " --watch 2\n";
    std::cout << "  " << prog << " --query-qubits\n";
    std::cout << "  " << prog << " --reset 0\n";
    std::cout << "  " << prog << " --format=json\n";
}

static void renderDashboard(qpu_device_t *qpu) {
    struct qpu_topology_query topo;
    std::memset(&topo, 0, sizeof(topo));
    struct qpu_coupler_edge edges[128];
    int retTopo = qpu_get_topology(qpu, &topo, edges, 128);

    struct qpu_cryo_telemetry cryo;
    std::memset(&cryo, 0, sizeof(cryo));
    int retCryo = qpu_query_cryo(qpu, &cryo);

    struct qpu_partition_entry parts[32];
    uint32_t numParts = 0;
    int retParts = qpu_query_partitions(qpu, parts, 32, &numParts);

    uint32_t numPhys = (retTopo == 0) ? topo.num_physical_qubits : 32;

    // Build allocation map:
    // '.' = free, 'A'..'Z' = tenant partition, 'G' = guard band, 'X' = defect
    std::string allocMap(numPhys, '.');
    uint32_t totalAllocated = 0;
    uint32_t totalGuard = 0;

    if (retParts == 0) {
        for (uint32_t i = 0; i < numParts; ++i) {
            char label = 'A' + (i % 26);
            for (uint32_t j = 0; j < parts[i].num_physical_qubits; ++j) {
                uint32_t q = parts[i].physical_qubits[j];
                if (q < numPhys) {
                    allocMap[q] = label;
                    totalAllocated++;
                }
            }
            for (uint32_t j = 0; j < parts[i].num_guard_qubits; ++j) {
                uint32_t g = parts[i].guard_qubits[j];
                if (g < numPhys) {
                    allocMap[g] = 'G';
                    totalGuard++;
                }
            }
        }
    }
    uint32_t totalFree = (numPhys > (totalAllocated + totalGuard)) ? (numPhys - totalAllocated - totalGuard) : 0;

    // Header Box
    std::cout << ANSI_CYAN << "+-------------------------------------------------------------------------------------------------+\n" << ANSI_RESET;
    std::cout << ANSI_CYAN << "| " << ANSI_BOLD << ANSI_BWHITE << "OmniQ-SMI: Quantum Processing Unit System Management Interface" << ANSI_RESET
              << ANSI_CYAN << "        Driver: " << ANSI_GREEN << "0.1.0-alpha" << ANSI_CYAN << " |\n";
    std::cout << ANSI_CYAN << "+-----------------------------------------------+-------------------------------------------------+\n" << ANSI_RESET;

    // Device & Cryo Status
    std::string devName = (retTopo == 0 && topo.device_name[0] != '\0') ? topo.device_name : "OmniQ-QPU-Accel-0";
    std::string techName = (retTopo == 0) ? qpu_tech_name(topo.technology) : "Superconducting Transmon";
    std::cout << ANSI_CYAN << "| " << ANSI_BOLD << ANSI_WHITE << "QPU 0: " << ANSI_BCYAN << std::left << std::setw(39) << devName << ANSI_RESET
              << ANSI_CYAN << "| " << ANSI_WHITE << "Tech: " << ANSI_BYELLOW << std::left << std::setw(41) << techName << ANSI_RESET << ANSI_CYAN << "|\n";

    std::string cryoStatus = (retCryo == 0 && cryo.is_cryo_ready == 1) ? "SUPERCONDUCTING NOMINAL" : "WARM / CALIBRATION";
    std::string cryoColor = (retCryo == 0 && cryo.is_cryo_ready == 1) ? ANSI_BGREEN : ANSI_BRED;
    std::ostringstream mxcStr;
    if (retCryo == 0) {
        mxcStr << std::fixed << std::setprecision(3) << (cryo.stage_mxc_uk / 1000.0) << " mK";
    } else {
        mxcStr << "N/A";
    }

    std::cout << ANSI_CYAN << "| " << ANSI_WHITE << "Cryo Status: " << cryoColor << std::left << std::setw(33) << cryoStatus << ANSI_RESET
              << ANSI_CYAN << "| " << ANSI_WHITE << "MXC Base Temp: " << ANSI_BCYAN << std::left << std::setw(12) << mxcStr.str() << ANSI_RESET
              << ANSI_WHITE << "Vacuum: " << ANSI_GRAY << "1.2e-6 bar" << ANSI_CYAN << "    |\n";

    std::ostringstream stagesStr;
    if (retCryo == 0) {
        stagesStr << "50K: " << std::fixed << std::setprecision(1) << (cryo.stage_50k_mk / 1000.0) << "K | "
                  << "4K: " << std::fixed << std::setprecision(2) << (cryo.stage_4k_mk / 1000.0) << "K";
    } else {
        stagesStr << "50K: N/A | 4K: N/A";
    }
    std::ostringstream stagesStr2;
    if (retCryo == 0) {
        stagesStr2 << "Still: " << cryo.stage_still_mk << " mK | Cold Plate: " << cryo.stage_cold_plate_mk << " mK";
    } else {
        stagesStr2 << "Still: N/A | Cold Plate: N/A";
    }
    std::cout << ANSI_CYAN << "| " << ANSI_GRAY << std::left << std::setw(45) << stagesStr.str() << ANSI_RESET
              << ANSI_CYAN << "| " << ANSI_GRAY << std::left << std::setw(47) << stagesStr2.str() << ANSI_RESET << ANSI_CYAN << " |\n";

    std::cout << ANSI_CYAN << "+-----------------------------------------------+-------------------------------------------------+\n" << ANSI_RESET;

    // Physical Qubit Allocation Matrix
    std::cout << ANSI_CYAN << "| " << ANSI_BOLD << ANSI_BWHITE << "Physical Qubit Allocation Matrix (" << numPhys << " Qubits)" << ANSI_RESET
              << ANSI_CYAN << std::string(62, ' ') << "|\n";

    std::cout << ANSI_CYAN << "|   [" << ANSI_RESET;
    for (size_t i = 0; i < allocMap.size(); ++i) {
        char c = allocMap[i];
        if (c == '.') {
            std::cout << ANSI_GRAY << '.' << ANSI_RESET;
        } else if (c == 'G') {
            std::cout << ANSI_YELLOW << 'G' << ANSI_RESET;
        } else if (c == 'X') {
            std::cout << ANSI_RED << 'X' << ANSI_RESET;
        } else {
            std::cout << ANSI_BCYAN << c << ANSI_RESET;
        }
    }
    std::cout << ANSI_CYAN << "]" << std::string(93 - allocMap.size(), ' ') << "|\n";

    std::ostringstream statStr;
    statStr << "Total: " << numPhys << "  |  Allocated: " << totalAllocated
            << "  |  Guard Band: " << totalGuard << "  |  Free: " << totalFree << "  |  Defective: 0";
    std::cout << ANSI_CYAN << "|   " << ANSI_WHITE << std::left << std::setw(92) << statStr.str() << ANSI_CYAN << "|\n";
    std::cout << ANSI_CYAN << "+-------------------------------------------------------------------------------------------------+\n" << ANSI_RESET;

    // Active Multi-Tenant Spatial Partitions Table
    std::cout << ANSI_CYAN << "| " << ANSI_BOLD << ANSI_BWHITE << "Active Spatial Partitions (Multi-Tenant QMMU Slices)" << ANSI_RESET
              << ANSI_CYAN << std::string(45, ' ') << "|\n";
    std::cout << ANSI_CYAN << "| " << ANSI_DIM << "Part ID  | PID    | Virtual | Physical Allocation           | Guards    | Avg 2Q Fid | Status   "
              << ANSI_CYAN << "|\n";
    std::cout << ANSI_CYAN << "|----------+--------+---------+-------------------------------+-----------+------------+----------|\n" << ANSI_RESET;

    if (numParts == 0) {
        std::cout << ANSI_CYAN << "| " << ANSI_GRAY << "No active multi-tenant partitions allocated. All qubits in idle ground state.               "
                  << ANSI_CYAN << "|\n" << ANSI_RESET;
    } else {
        for (uint32_t i = 0; i < numParts; ++i) {
            std::ostringstream partIdStr, pidStr, virtStr, physStr, guardStr, fidStr;
            partIdStr << "#" << parts[i].partition_id;
            pidStr << parts[i].process_id;
            virtStr << parts[i].num_virtual_qubits;

            if (parts[i].num_physical_qubits <= 4) {
                for (size_t k = 0; k < parts[i].num_physical_qubits; ++k) {
                    physStr << "Q" << parts[i].physical_qubits[k] << (k + 1 < parts[i].num_physical_qubits ? "," : "");
                }
            } else {
                physStr << "Q" << parts[i].physical_qubits[0] << ".." << "Q" << parts[i].physical_qubits[parts[i].num_physical_qubits - 1]
                        << " (" << parts[i].num_physical_qubits << "Q)";
            }

            guardStr << parts[i].num_guard_qubits << " guards";
            fidStr << std::fixed << std::setprecision(2) << (parts[i].avg_fidelity_ppm / 10000.0) << "%";

            std::cout << ANSI_CYAN << "| "
                      << ANSI_BCYAN << std::left << std::setw(8) << partIdStr.str() << ANSI_CYAN << " | "
                      << ANSI_WHITE << std::left << std::setw(6) << pidStr.str() << ANSI_CYAN << " | "
                      << ANSI_WHITE << std::left << std::setw(7) << virtStr.str() << ANSI_CYAN << " | "
                      << ANSI_GREEN << std::left << std::setw(29) << physStr.str() << ANSI_CYAN << " | "
                      << ANSI_YELLOW << std::left << std::setw(9) << guardStr.str() << ANSI_CYAN << " | "
                      << ANSI_BWHITE << std::left << std::setw(10) << fidStr.str() << ANSI_CYAN << " | "
                      << ANSI_BGREEN << std::left << std::setw(8) << "ACTIVE" << ANSI_CYAN << " |\n" << ANSI_RESET;
        }
    }
    std::cout << ANSI_CYAN << "+-------------------------------------------------------------------------------------------------+\n" << ANSI_RESET;
}

static void printQubitsTable(qpu_device_t *qpu) {
    struct qpu_topology_query topo;
    std::memset(&topo, 0, sizeof(topo));
    struct qpu_coupler_edge edges[128];
    if (qpu_get_topology(qpu, &topo, edges, 128) != 0) {
        std::cerr << "[-] Failed to query topology.\n";
        return;
    }

    uint32_t numQubits = topo.num_physical_qubits;
    std::vector<struct qpu_qubit_calibration> cals(numQubits);
    uint64_t timestamp = 0;
    if (qpu_get_calibration(qpu, cals.data(), numQubits, &timestamp) != 0) {
        std::cerr << "[-] Failed to query calibration.\n";
        return;
    }

    std::cout << "========================================================================================\n";
    std::cout << " OmniQ QPU Physical Qubit Calibration Matrix (Sweep Timestamp: " << timestamp << " ns)\n";
    std::cout << "========================================================================================\n";
    std::cout << std::left
              << std::setw(6)  << "QID"
              << std::setw(10) << "Status"
              << std::setw(14) << "T1 (us)"
              << std::setw(14) << "T2 (us)"
              << std::setw(16) << "Readout Fid"
              << std::setw(16) << "1Q Gate Fid"
              << std::setw(14) << "Freq (GHz)"
              << "\n";
    std::cout << "----------------------------------------------------------------------------------------\n";

    for (uint32_t i = 0; i < numQubits; ++i) {
        double t1_us = cals[i].t1_ns / 1000.0;
        double t2_us = cals[i].t2_ns / 1000.0;
        double roFid = cals[i].readout_fidelity_ppm / 10000.0;
        double gateFid = cals[i].single_qubit_fidelity_ppm / 10000.0;
        double freqGhz = cals[i].frequency_mhz / 1000.0;
        std::string status = (cals[i].is_alive == 1) ? "ONLINE" : "DEFECT";

        std::cout << std::left
                  << std::setw(6)  << cals[i].physical_id
                  << std::setw(10) << status
                  << std::fixed << std::setprecision(1) << std::setw(14) << t1_us
                  << std::fixed << std::setprecision(1) << std::setw(14) << t2_us
                  << std::fixed << std::setprecision(3) << std::setw(16) << (std::to_string(roFid).substr(0, 6) + "%")
                  << std::fixed << std::setprecision(3) << std::setw(16) << (std::to_string(gateFid).substr(0, 6) + "%")
                  << std::fixed << std::setprecision(3) << std::setw(14) << freqGhz
                  << "\n";
    }
    std::cout << "========================================================================================\n";
}

static void printCryoDetails(qpu_device_t *qpu) {
    struct qpu_cryo_telemetry cryo;
    std::memset(&cryo, 0, sizeof(cryo));
    if (qpu_query_cryo(qpu, &cryo) != 0) {
        std::cerr << "[-] Failed to query cryo telemetry.\n";
        return;
    }

    std::cout << "===================================================================\n";
    std::cout << " OmniQ Dilution Refrigerator Cryogenic Telemetry Interface\n";
    std::cout << " Timestamp: " << cryo.timestamp_ns << " ns\n";
    std::cout << "===================================================================\n";
    std::cout << "  50K Thermal Stage:      " << std::fixed << std::setprecision(3) << (cryo.stage_50k_mk / 1000.0) << " K (" << cryo.stage_50k_mk << " mK)\n";
    std::cout << "  4K Thermal Stage:       " << std::fixed << std::setprecision(3) << (cryo.stage_4k_mk / 1000.0) << " K (" << cryo.stage_4k_mk << " mK)\n";
    std::cout << "  Still Stage:            " << cryo.stage_still_mk << " mK\n";
    std::cout << "  Cold Plate Stage:       " << cryo.stage_cold_plate_mk << " mK\n";
    std::cout << "  Mixing Chamber (MXC):   " << std::fixed << std::setprecision(3) << (cryo.stage_mxc_uk / 1000.0) << " mK (" << cryo.stage_mxc_uk << " uK)\n";
    std::cout << "  Vacuum Chamber:         " << cryo.vacuum_pressure_ubar << " ubar\n";
    std::cout << "  Cryostat Status:        " << ((cryo.is_cryo_ready == 1) ? "SUPERCONDUCTING NOMINAL (<50 mK)" : "WARM / CALIBRATION") << "\n";
    std::cout << "===================================================================\n";
}

static void printPartitionsDetails(qpu_device_t *qpu) {
    struct qpu_partition_entry parts[64];
    uint32_t numParts = 0;
    if (qpu_query_partitions(qpu, parts, 64, &numParts) != 0) {
        std::cerr << "[-] Failed to query active partitions.\n";
        return;
    }

    std::cout << "====================================================================================\n";
    std::cout << " OmniQ QMMU Active Spatial Partitions (Total Active: " << numParts << ")\n";
    std::cout << "====================================================================================\n";
    if (numParts == 0) {
        std::cout << "No active multi-tenant partitions allocated.\n";
    } else {
        for (uint32_t i = 0; i < numParts; ++i) {
            std::cout << "Partition #" << parts[i].partition_id << " (Owner PID: " << parts[i].process_id << ")\n";
            std::cout << "  Virtual Qubit Capacity:   " << parts[i].num_virtual_qubits << "\n";
            std::cout << "  Assigned Physical Qubits: [";
            for (uint32_t j = 0; j < parts[i].num_physical_qubits; ++j) {
                std::cout << parts[i].physical_qubits[j] << (j + 1 < parts[i].num_physical_qubits ? ", " : "");
            }
            std::cout << "]\n";
            std::cout << "  Isolated Guard Bands:     [";
            for (uint32_t j = 0; j < parts[i].num_guard_qubits; ++j) {
                std::cout << parts[i].guard_qubits[j] << (j + 1 < parts[i].num_guard_qubits ? ", " : "");
            }
            std::cout << "]\n";
            std::cout << "  Average Coupler Fidelity: " << std::fixed << std::setprecision(4)
                      << (parts[i].avg_fidelity_ppm / 10000.0) << "%\n";
            std::cout << "------------------------------------------------------------------------------------\n";
        }
    }
}

static void printJson(qpu_device_t *qpu) {
    struct qpu_topology_query topo;
    std::memset(&topo, 0, sizeof(topo));
    struct qpu_coupler_edge edges[128];
    int retTopo = qpu_get_topology(qpu, &topo, edges, 128);

    struct qpu_cryo_telemetry cryo;
    std::memset(&cryo, 0, sizeof(cryo));
    int retCryo = qpu_query_cryo(qpu, &cryo);

    uint32_t numQubits = (retTopo == 0) ? topo.num_physical_qubits : 32;
    std::vector<struct qpu_qubit_calibration> cals(numQubits);
    uint64_t calTime = 0;
    qpu_get_calibration(qpu, cals.data(), numQubits, &calTime);

    struct qpu_partition_entry parts[64];
    uint32_t numParts = 0;
    qpu_query_partitions(qpu, parts, 64, &numParts);

    std::cout << "{\n";
    std::cout << "  \"device\": {\n";
    std::cout << "    \"name\": \"" << ((retTopo == 0) ? topo.device_name : "OmniQ-QPU-Accel-0") << "\",\n";
    std::cout << "    \"technology\": \"" << ((retTopo == 0) ? qpu_tech_name(topo.technology) : "Superconducting Transmon") << "\",\n";
    std::cout << "    \"num_physical_qubits\": " << numQubits << "\n";
    std::cout << "  },\n";

    std::cout << "  \"cryo\": {\n";
    std::cout << "    \"timestamp_ns\": " << cryo.timestamp_ns << ",\n";
    std::cout << "    \"mxc_temperature_mk\": " << (cryo.stage_mxc_uk / 1000.0) << ",\n";
    std::cout << "    \"stage_50k_k\": " << (cryo.stage_50k_mk / 1000.0) << ",\n";
    std::cout << "    \"stage_4k_k\": " << (cryo.stage_4k_mk / 1000.0) << ",\n";
    std::cout << "    \"still_mk\": " << cryo.stage_still_mk << ",\n";
    std::cout << "    \"cold_plate_mk\": " << cryo.stage_cold_plate_mk << ",\n";
    std::cout << "    \"vacuum_pressure_ubar\": " << cryo.vacuum_pressure_ubar << ",\n";
    std::cout << "    \"ready\": " << ((cryo.is_cryo_ready == 1) ? "true" : "false") << "\n";
    std::cout << "  },\n";

    std::cout << "  \"partitions\": [\n";
    for (uint32_t i = 0; i < numParts; ++i) {
        std::cout << "    {\n";
        std::cout << "      \"id\": " << parts[i].partition_id << ",\n";
        std::cout << "      \"pid\": " << parts[i].process_id << ",\n";
        std::cout << "      \"num_virtual_qubits\": " << parts[i].num_virtual_qubits << ",\n";
        std::cout << "      \"physical_qubits\": [";
        for (uint32_t j = 0; j < parts[i].num_physical_qubits; ++j) {
            std::cout << parts[i].physical_qubits[j] << (j + 1 < parts[i].num_physical_qubits ? ", " : "");
        }
        std::cout << "],\n";
        std::cout << "      \"guard_qubits\": [";
        for (uint32_t j = 0; j < parts[i].num_guard_qubits; ++j) {
            std::cout << parts[i].guard_qubits[j] << (j + 1 < parts[i].num_guard_qubits ? ", " : "");
        }
        std::cout << "],\n";
        std::cout << "      \"avg_fidelity\": " << (parts[i].avg_fidelity_ppm / 1000000.0) << "\n";
        std::cout << "    }" << (i + 1 < numParts ? "," : "") << "\n";
    }
    std::cout << "  ]\n";
    std::cout << "}\n";
}

int main(int argc, char *argv[]) {
    std::signal(SIGINT, sigHandler);
    std::signal(SIGTERM, sigHandler);

    bool optWatch = false;
    int watchSec = 1;
    bool optQubits = false;
    bool optCryo = false;
    bool optPartitions = false;
    bool optReset = false;
    uint32_t resetQid = 0;
    bool optJson = false;
    const char *devPath = nullptr;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            return 0;
        } else if (arg == "-w" || arg == "--watch") {
            optWatch = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                watchSec = std::atoi(argv[++i]);
                if (watchSec < 1) watchSec = 1;
            }
        } else if (arg == "-q" || arg == "--query-qubits") {
            optQubits = true;
        } else if (arg == "-c" || arg == "--query-cryo") {
            optCryo = true;
        } else if (arg == "-p" || arg == "--partitions") {
            optPartitions = true;
        } else if (arg == "-r" || arg == "--reset") {
            optReset = true;
            if (i + 1 < argc) {
                resetQid = static_cast<uint32_t>(std::atoi(argv[++i]));
            }
        } else if (arg == "--format=json") {
            optJson = true;
        } else if (arg == "--device") {
            if (i + 1 < argc) {
                devPath = argv[++i];
            }
        }
    }

    qpu_device_t *qpu = qpu_open(devPath);
    if (!qpu) {
        std::cerr << "[-] Error: Failed to connect to QPU device/daemon.\n";
        return 1;
    }

    if (optReset) {
        std::cout << "[*] Dispatching active microwave resonator reset on Physical Qubit " << resetQid << "...\n";
        int ret = qpu_reset_qubit(qpu, resetQid, QPU_RESET_MODE_ACTIVE);
        if (ret == 0) {
            std::cout << "[+] Physical Qubit " << resetQid << " successfully reset to |0> ground state.\n";
        } else {
            std::cerr << "[-] Qubit reset failed with error code: " << ret << "\n";
        }
        qpu_close(qpu);
        return (ret == 0) ? 0 : 1;
    }

    if (optJson) {
        printJson(qpu);
        qpu_close(qpu);
        return 0;
    }

    if (optQubits) {
        printQubitsTable(qpu);
        qpu_close(qpu);
        return 0;
    }

    if (optCryo) {
        printCryoDetails(qpu);
        qpu_close(qpu);
        return 0;
    }

    if (optPartitions) {
        printPartitionsDetails(qpu);
        qpu_close(qpu);
        return 0;
    }

    if (optWatch) {
        while (g_running) {
            std::cout << "\033[H\033[J"; // Clear terminal screen
            renderDashboard(qpu);
            std::cout << ANSI_GRAY << "Refreshing every " << watchSec << "s. Press Ctrl+C to exit.\n" << ANSI_RESET;
            std::this_thread::sleep_for(std::chrono::seconds(watchSec));
        }
    } else {
        renderDashboard(qpu);
    }

    qpu_close(qpu);
    return 0;
}
