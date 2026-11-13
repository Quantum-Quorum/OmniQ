/*
 * OmniQ-QOS: User-space C POSIX Interface
 * include/omniq/qpu.h
 *
 * Copyright (c) 2026 Quantum Quorum
 */

#ifndef _OMNIQ_QPU_H
#define _OMNIQ_QPU_H

#include "../uapi/linux/qpu.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct qpu_device qpu_device_t;

/**
 * Open a connection to a QPU device or virtual QPU daemon.
 * @param dev_path Path to the device node (e.g. "/dev/accel/qpu0" or "/tmp/omniq_qpu0.sock")
 * @return Handle to QPU device, or NULL on error.
 */
qpu_device_t *qpu_open(const char *dev_path);

/**
 * Close and release resources associated with the QPU device handle.
 */
void qpu_close(qpu_device_t *dev);

/**
 * Query physical chip topology and 2-qubit coupler connectivity graph.
 */
int qpu_get_topology(qpu_device_t *dev, struct qpu_topology_query *topo,
                     struct qpu_coupler_edge *edges, uint32_t max_edges);

/**
 * Query dynamic calibration parameters (T1, T2, gate error rates per physical qubit).
 */
int qpu_get_calibration(qpu_device_t *dev, struct qpu_qubit_calibration *cals,
                        uint32_t max_qubits, uint64_t *timestamp_out);

/**
 * Request the Quantum Memory Management Unit (QMMU) to allocate a virtual partition.
 * @param dev QPU handle
 * @param num_qubits Number of logical/virtual qubits needed
 * @param flags QPU_ALLOC_FLAG_* bitmask
 * @param out_partition_id Pointer to receive the allocated partition ID
 * @return 0 on success, negative errno on failure.
 */
int qpu_alloc(qpu_device_t *dev, uint32_t num_qubits, uint32_t flags,
              uint32_t *out_partition_id);

/**
 * Free an allocated virtual qubit partition and trigger active reset.
 */
int qpu_free(qpu_device_t *dev, uint32_t partition_id);

/**
 * Submit an OpenQASM circuit or QIR bytecode to an active partition.
 * @param dev QPU handle
 * @param partition_id Active partition ID from qpu_alloc()
 * @param qasm_or_code Circuit description string (OpenQASM 2.0 / 3.0)
 * @param shots Number of experimental measurement repetitions
 * @param flags QPU_JOB_FLAG_* bitmask
 * @param out_job_id Pointer to receive asynchronous job tracking ID
 * @return 0 on success, negative errno on failure.
 */
int qpu_submit(qpu_device_t *dev, uint32_t partition_id, const char *qasm_or_code,
               uint32_t shots, uint32_t flags, uint64_t *out_job_id);

/**
 * Wait for job completion and collect histogram bitstring counts.
 * @param dev QPU handle
 * @param job_id Job tracking ID
 * @param timeout_ms Maximum time to wait in milliseconds
 * @param entries Buffer to store readout bitstrings and counts
 * @param max_entries Capacity of entries array
 * @param out_num_entries Pointer to receive number of unique bitstrings observed
 * @param out_total_shots Pointer to receive total valid shots collected
 * @return 0 on success, negative errno on failure (e.g. -ETIMEDOUT).
 */
int qpu_wait(qpu_device_t *dev, uint64_t job_id, uint32_t timeout_ms,
             struct qpu_readout_entry *entries, uint32_t max_entries,
             uint32_t *out_num_entries, uint32_t *out_total_shots);

/**
 * Synchronous execution helper: allocates, compiles, executes, and frees partition in one call.
 */
int qpu_run_sync(qpu_device_t *dev, uint32_t num_qubits, const char *qasm_str,
                 uint32_t shots, struct qpu_readout_entry *entries,
                 uint32_t max_entries, uint32_t *out_num_entries);

/**
 * Human-readable string for QPU technology type.
 */
const char *qpu_tech_name(uint32_t tech);

#ifdef __cplusplus
}
#endif

#endif /* _OMNIQ_QPU_H */
