/*
 * OmniQ-QOS: POSIX QPU Client Example
 * examples/posix_qpu_client.c
 *
 * Copyright (c) 2026 Quantum Quorum
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/omniq/qpu.h"

int main() {
    printf("========================================================\n");
    printf(" OmniQ-QOS: Native POSIX Quantum Application Demo       \n");
    printf(" Communicating via Linux drivers/accel/qpu/ Interface   \n");
    printf("========================================================\n\n");

    // 1. Open the QPU device (connects to daemon IPC socket or falls back to virtual accelerator)
    printf("[1] Connecting to QPU accelerator (/tmp/omniq_qpu0.sock or direct)...\n");
    qpu_device_t *qpu = qpu_open(NULL);
    if (!qpu) {
        fprintf(stderr, "[-] Failed to open QPU device.\n");
        return 1;
    }
    printf("    [+] Successfully connected to QPU accelerator.\n\n");

    // 2. Query hardware topology & architecture
    printf("[2] Querying QPU Architecture & Coupler Topology...\n");
    struct qpu_topology_query topo;
    struct qpu_coupler_edge edges[32];
    memset(&topo, 0, sizeof(topo));

    if (qpu_get_topology(qpu, &topo, edges, 32) == 0) {
        printf("    [+] Device Name:       %s\n", topo.device_name);
        printf("    [+] Technology:        %s\n", qpu_tech_name(topo.technology));
        printf("    [+] Physical Qubits:   %u qubits\n", topo.num_physical_qubits);
        printf("    [+] Coupler Edges:     %u edges\n", topo.num_edges);
        printf("    [+] Guard Band Margin: %u qubit(s)\n", topo.guard_band_distance);
        printf("    [+] First 3 couplers:  (%u <-> %u, fid: %.2f%%), (%u <-> %u, fid: %.2f%%), (%u <-> %u, fid: %.2f%%)\n\n",
               edges[0].qubit_a, edges[0].qubit_b, edges[0].fidelity_ppm / 10000.0,
               edges[1].qubit_a, edges[1].qubit_b, edges[1].fidelity_ppm / 10000.0,
               edges[2].qubit_a, edges[2].qubit_b, edges[2].fidelity_ppm / 10000.0);
    }

    // 3. Query Real-Time Calibration (T1 / T2)
    printf("[3] Querying Live Calibration Matrix (/sys/class/accel/qpu0/calibration)...\n");
    struct qpu_qubit_calibration cals[8];
    uint64_t cal_time = 0;
    if (qpu_get_calibration(qpu, cals, 8, &cal_time) == 0) {
        printf("    [+] Physical Qubit 0: T1 = %.1f us, T2 = %.1f us, 1Q Fid = %.3f%%, Freq = %.2f GHz\n",
               cals[0].t1_ns / 1000.0, cals[0].t2_ns / 1000.0, cals[0].single_qubit_fidelity_ppm / 10000.0, cals[0].frequency_mhz / 1000.0);
        printf("    [+] Physical Qubit 1: T1 = %.1f us, T2 = %.1f us, 1Q Fid = %.3f%%, Freq = %.2f GHz\n\n",
               cals[1].t1_ns / 1000.0, cals[1].t2_ns / 1000.0, cals[1].single_qubit_fidelity_ppm / 10000.0, cals[1].frequency_mhz / 1000.0);
    }

    // 4. Query Cryogenic Dilution Refrigerator Telemetry
    printf("[4] Querying Dilution Refrigerator Telemetry (QPU_IOCTL_QUERY_CRYO)...\n");
    struct qpu_cryo_telemetry cryo;
    if (qpu_query_cryo(qpu, &cryo) == 0) {
        printf("    [+] 50K Stage:         %.3f K\n", cryo.stage_50k_mk / 1000.0);
        printf("    [+] 4K Stage:          %.3f K\n", cryo.stage_4k_mk / 1000.0);
        printf("    [+] Still Stage:       %.3f K (%u mK)\n", cryo.stage_still_mk / 1000.0, cryo.stage_still_mk);
        printf("    [+] Cold Plate Stage:  %.3f K (%u mK)\n", cryo.stage_cold_plate_mk / 1000.0, cryo.stage_cold_plate_mk);
        printf("    [+] Mixing Chamber:    %.3f mK (%u uK) [Status: %s]\n\n",
               cryo.stage_mxc_uk / 1000.0, cryo.stage_mxc_uk,
               cryo.is_cryo_ready ? "SUPERCONDUCTING NOMINAL (<50 mK)" : "WARM / DEGRADED");
    }

    // 5. Request QMMU to allocate an isolated 2-qubit partition
    printf("[5] Requesting QMMU Partition Allocation (num_qubits = 2, flags = ISOLATED)...\n");
    uint32_t partition_id = 0;
    int err = qpu_alloc(qpu, 2, QPU_ALLOC_FLAG_ISOLATED | QPU_ALLOC_FLAG_HIGH_FIDELITY, &partition_id);
    if (err != 0) {
        fprintf(stderr, "[-] QMMU allocation failed with error: %d\n", err);
        qpu_close(qpu);
        return 1;
    }
    printf("    [+] QMMU successfully provisioned Virtual Partition #%u\n", partition_id);

    // 6. Query Active Spatial Partitions
    printf("    [+] Inspecting Active Partitions (QPU_IOCTL_QUERY_PARTITIONS)...\n");
    struct qpu_partition_entry partitions[4];
    uint32_t num_partitions = 0;
    if (qpu_query_partitions(qpu, partitions, 4, &num_partitions) == 0) {
        printf("    [+] Total Active Partitions: %u\n", num_partitions);
        for (uint32_t p = 0; p < num_partitions; ++p) {
            printf("        -> Partition #%u (PID %u): %u virtual qubits, %u physical qubits, %u guard qubits, avg fid: %.2f%%\n",
                   partitions[p].partition_id, partitions[p].process_id,
                   partitions[p].num_virtual_qubits, partitions[p].num_physical_qubits,
                   partitions[p].num_guard_qubits, partitions[p].avg_fidelity_ppm / 10000.0);
        }
    }
    printf("\n");

    // 7. Submit OpenQASM Bell State Circuit
    printf("[7] Submitting Quantum Circuit to Partition #%u...\n", partition_id);
    const char *bell_circuit =
        "OPENQASM 2.0;\n"
        "include \"qelib1.inc\";\n"
        "qreg q[2];\n"
        "creg c[2];\n"
        "h q[0];\n"
        "cx q[0], q[1];\n";

    printf("--- Circuit OpenQASM ---\n%s------------------------\n", bell_circuit);

    uint64_t job_id = 0;
    err = qpu_submit(qpu, partition_id, bell_circuit, 4096, QPU_JOB_FLAG_DYNAMIC_DECOUPLING, &job_id);
    if (err != 0) {
        fprintf(stderr, "[-] Circuit submission failed with error: %d\n", err);
        qpu_free(qpu, partition_id);
        qpu_close(qpu);
        return 1;
    }
    printf("    [+] Job submitted successfully -> Ticket Job ID #%llu\n\n", (unsigned long long)job_id);

    // 8. Wait for measurement readout
    printf("[8] Reading back quantum measurement bitstrings...\n");
    struct qpu_readout_entry results[16];
    uint32_t num_entries = 0;
    uint32_t total_shots = 0;

    err = qpu_wait(qpu, job_id, 2000, results, 16, &num_entries, &total_shots);
    if (err != 0) {
        fprintf(stderr, "[-] Wait timed out or failed: %d\n", err);
    } else {
        printf("    [+] Collected %u shots across %u unique basis states:\n", total_shots, num_entries);
        for (uint32_t i = 0; i < num_entries; ++i) {
            double prob = (double)results[i].count * 100.0 / total_shots;
            printf("        |State %llu⟩ (|%s⟩): %u shots (%.2f%%)\n",
                   (unsigned long long)results[i].basis_state,
                   results[i].basis_state == 0 ? "00" : (results[i].basis_state == 3 ? "11" : "other"),
                   results[i].count, prob);
        }
    }

    // 9. Free partition
    printf("\n[9] Releasing Virtual Partition #%u (Triggering active qubit reset)...\n", partition_id);
    qpu_free(qpu, partition_id);
    printf("    [+] Partition freed back to QMMU pool.\n");

    // 10. Dispatch Individual Physical Qubit Reset
    printf("\n[10] Dispatching Active Qubit Reset on Physical Qubit 0 (QPU_IOCTL_RESET_QUBIT)...\n");
    if (qpu_reset_qubit(qpu, 0, QPU_RESET_MODE_ACTIVE) == 0) {
        printf("    [+] Physical Qubit 0 active microwave resonator reset succeeded (state -> |0>).\n");
    }

    qpu_close(qpu);
    printf("    [+] QPU device closed. POSIX Quantum Execution Completed Successfully!\n");
    return 0;
}

