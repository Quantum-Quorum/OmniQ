/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * OmniQ-QOS: Linux Compute Accelerator Subsystem for QPUs
 * User-space API definitions (include/uapi/linux/qpu.h)
 *
 * Copyright (c) 2026 Quantum Quorum
 */

#ifndef _UAPI_LINUX_QPU_H
#define _UAPI_LINUX_QPU_H

#include <stdint.h>

#ifdef __linux__
#include <linux/types.h>
#include <linux/ioctl.h>
#else
/* Portability fallback for macOS / non-Linux host testing */
#ifndef _IOC_NRBITS
#define _IOC_NRBITS	8
#define _IOC_TYPEBITS	8
#define _IOC_SIZEBITS	14
#define _IOC_DIRBITS	2

#define _IOC_NRMASK	((1 << _IOC_NRBITS)-1)
#define _IOC_TYPEMASK	((1 << _IOC_TYPEBITS)-1)
#define _IOC_SIZEMASK	((1 << _IOC_SIZEBITS)-1)
#define _IOC_DIRMASK	((1 << _IOC_DIRBITS)-1)

#define _IOC_NRSHIFT	0
#define _IOC_TYPESHIFT	(_IOC_NRSHIFT+_IOC_NRBITS)
#define _IOC_SIZESHIFT	(_IOC_TYPESHIFT+_IOC_TYPEBITS)
#define _IOC_DIRSHIFT	(_IOC_SIZESHIFT+_IOC_SIZEBITS)

#define _IOC_NONE	0U
#define _IOC_WRITE	1U
#define _IOC_READ	2U

#define _IOC(dir,type,nr,size) \
	(((dir)  << _IOC_DIRSHIFT) | \
	 ((type) << _IOC_TYPESHIFT) | \
	 ((nr)   << _IOC_NRSHIFT) | \
	 ((size) << _IOC_SIZESHIFT))

#define _IO(type,nr)		_IOC(_IOC_NONE,(type),(nr),0)
#define _IOR(type,nr,size)	_IOC(_IOC_READ,(type),(nr),sizeof(size))
#define _IOW(type,nr,size)	_IOC(_IOC_WRITE,(type),(nr),sizeof(size))
#define _IOWR(type,nr,size)	_IOC(_IOC_READ|_IOC_WRITE,(type),(nr),sizeof(size))
#endif
typedef uint8_t  __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef int32_t  __s32;
typedef int64_t  __s64;
#endif

#define QPU_IOCTL_MAGIC 'Q'

/* QPU Technology Types */
#define QPU_TECH_SUPERCONDUCTING_TRANSMON  0x01
#define QPU_TECH_TRAPPED_ION               0x02
#define QPU_TECH_NEUTRAL_ATOM              0x03
#define QPU_TECH_PHOTONIC                  0x04
#define QPU_TECH_MAGNON_DUAL_RAIL          0x05

/* QPU Allocation Flags */
#define QPU_ALLOC_FLAG_NONE                0x00000000
#define QPU_ALLOC_FLAG_ISOLATED            0x00000001 /* Enforce physical guard-band spacing */
#define QPU_ALLOC_FLAG_HIGH_FIDELITY       0x00000002 /* Prioritize qubits with highest 2Q fidelities */
#define QPU_ALLOC_FLAG_LONG_COHERENCE      0x00000004 /* Prioritize longest T1/T2 times */
#define QPU_ALLOC_FLAG_FAULT_TOLERANT      0x00000008 /* Allocate ancillas for QEC stabilizers */

/* QPU Job Flags */
#define QPU_JOB_FLAG_NONE                  0x00000000
#define QPU_JOB_FLAG_DYNAMIC_DECOUPLING    0x00000001 /* Kernel auto-injects CPMG/XY-8 on idle cycles */
#define QPU_JOB_FLAG_ACTIVE_RESET          0x00000002 /* Active resonator reset rather than thermal wait */
#define QPU_JOB_FLAG_RETURN_STATEVECTOR    0x00000004 /* Debug mode: return full amplitudes if simulator */

/* Maximum limits for ABI stability */
#define QPU_MAX_PHYSICAL_QUBITS            1024
#define QPU_MAX_COUPLER_EDGES              4096
#define QPU_MAX_ACTIVE_PARTITIONS          64
#define QPU_MAX_NAME_LEN                   64

/* ========================================================================= */
/* IOCTL 0x01: Query Hardware Topology & Static Properties                  */
/* ========================================================================= */

struct qpu_coupler_edge {
    __u32 qubit_a;
    __u32 qubit_b;
    __u32 fidelity_ppm;            /* Gate fidelity in parts-per-million (e.g., 995000 = 99.5%) */
    __u32 typical_duration_ns;     /* 2Q gate duration in nanoseconds */
};

struct qpu_topology_query {
    char  device_name[QPU_MAX_NAME_LEN];
    __u32 technology;              /* QPU_TECH_* */
    __u32 num_physical_qubits;     /* Total physical qubits on chip */
    __u32 num_edges;               /* Total 2-qubit coupler edges */
    __u32 guard_band_distance;     /* Minimum physical distance between isolated tenants */
    __u64 edges_buffer_ptr;        /* Pointer to array of struct qpu_coupler_edge */
    __u32 max_edges_capacity;
};
#define QPU_IOCTL_QUERY_TOPOLOGY    _IOWR(QPU_IOCTL_MAGIC, 0x01, struct qpu_topology_query)

/* ========================================================================= */
/* IOCTL 0x02: Query Dynamic Calibration Matrix (T1, T2, Error Rates)       */
/* ========================================================================= */

struct qpu_qubit_calibration {
    __u32 physical_id;
    __u64 t1_ns;                   /* Relaxation time in nanoseconds */
    __u64 t2_ns;                   /* Dephasing time in nanoseconds */
    __u32 readout_fidelity_ppm;    /* Readout fidelity (ppm) */
    __u32 single_qubit_fidelity_ppm; /* 1Q gate fidelity (ppm) */
    __u32 frequency_mhz;           /* Qubit resonant transition frequency */
    __u32 is_alive;                /* 1 = healthy, 0 = defect / offline */
};

struct qpu_calibration_query {
    __u64 timestamp_ns;            /* Epoch timestamp of calibration sweep */
    __u32 num_qubits;
    __u64 calibration_buffer_ptr;  /* Pointer to array of struct qpu_qubit_calibration */
};
#define QPU_IOCTL_QUERY_CALIBRATION _IOWR(QPU_IOCTL_MAGIC, 0x02, struct qpu_calibration_query)

/* ========================================================================= */
/* IOCTL 0x03: Allocate Virtual Qubit Partition (QMMU)                      */
/* ========================================================================= */

struct qpu_alloc_req {
    __u32 num_virtual_qubits;      /* Requested logical/virtual qubit count */
    __u32 flags;                   /* QPU_ALLOC_FLAG_* */
    __u32 min_t1_ns;               /* Constraint or 0 */
    __u32 min_t2_ns;               /* Constraint or 0 */
    __u32 partition_id;            /* [OUTPUT] Allocated partition descriptor */
    __u32 physical_qubits_allocated; /* [OUTPUT] Actual physical qubits assigned (including guard/ancilla) */
};
#define QPU_IOCTL_ALLOC_QUBITS      _IOWR(QPU_IOCTL_MAGIC, 0x03, struct qpu_alloc_req)

/* ========================================================================= */
/* IOCTL 0x04: Free Partition                                                */
/* ========================================================================= */

struct qpu_free_req {
    __u32 partition_id;
    __u32 reset_mode;              /* QPU_JOB_FLAG_ACTIVE_RESET */
};
#define QPU_IOCTL_FREE_QUBITS       _IOW(QPU_IOCTL_MAGIC, 0x04, struct qpu_free_req)

/* ========================================================================= */
/* IOCTL 0x05: Submit Quantum Circuit / Microcode Job                       */
/* ========================================================================= */

struct qpu_submit_job {
    __u32 partition_id;
    __u32 flags;                   /* QPU_JOB_FLAG_* */
    __u32 shots;                   /* Number of measurement repetitions */
    __u32 timeout_ms;              /* Maximum execution timeout */
    __u64 qasm_or_bytecode_ptr;    /* Pointer to OpenQASM 3.0 / QIR bytecode string */
    __u32 bytecode_len;
    __u64 job_id;                  /* [OUTPUT] Asynchronous job tracking ticket */
};
#define QPU_IOCTL_SUBMIT_JOB        _IOWR(QPU_IOCTL_MAGIC, 0x05, struct qpu_submit_job)

/* ========================================================================= */
/* IOCTL 0x06: Poll / Wait for Execution Results                            */
/* ========================================================================= */

struct qpu_readout_entry {
    __u64 basis_state;             /* Measured bitstring (bit 0 = q0, bit 1 = q1, ...) */
    __u32 count;                   /* Frequency count */
};

struct qpu_result_req {
    __u64 job_id;
    __u32 timeout_ms;
    __u32 is_completed;            /* [OUTPUT] 1 = ready, 0 = still running */
    __u32 total_shots_collected;   /* [OUTPUT] Total valid shots */
    __u32 num_unique_outcomes;     /* [OUTPUT] Number of distinct observed bitstrings */
    __u64 entries_buffer_ptr;      /* Pointer to array of struct qpu_readout_entry */
    __u32 max_entries_capacity;
};
#define QPU_IOCTL_READ_RESULTS      _IOWR(QPU_IOCTL_MAGIC, 0x06, struct qpu_result_req)

#endif /* _UAPI_LINUX_QPU_H */
