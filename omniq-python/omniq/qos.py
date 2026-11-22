"""
OmniQ-QOS: Python ctypes interface for Quantum Operating System and QPU accelerator.
omniq-python/omniq/qos.py

Copyright (c) 2026 Quantum Quorum
"""

import os
import sys
import ctypes
from ctypes import (
    c_char, c_char_p, c_int, c_uint32, c_uint64, c_uint8, c_int32,
    c_void_p, c_bool, Structure, POINTER, byref, create_string_buffer
)
from typing import Optional, Dict, List, Any, Union

# Constants matching UAPI linux/qpu.h
QPU_TECH_SUPERCONDUCTING_TRANSMON = 0x01
QPU_TECH_TRAPPED_ION             = 0x02
QPU_TECH_NEUTRAL_ATOM            = 0x03
QPU_TECH_PHOTONIC                = 0x04
QPU_TECH_SILICON_SPIN            = 0x05

QPU_ALLOC_FLAG_ISOLATED          = 0x00000001
QPU_ALLOC_FLAG_HIGH_FIDELITY     = 0x00000002
QPU_ALLOC_FLAG_TOPOLOGICAL_NEAREST = 0x00000004
QPU_ALLOC_FLAG_DYNAMIC_EXPAND    = 0x00000008

QPU_JOB_FLAG_DYNAMIC_DECOUPLING  = 0x00000001
QPU_JOB_FLAG_ACTIVE_RESET        = 0x00000002
QPU_JOB_FLAG_FAST_FEEDFORWARD    = 0x00000004

QPU_RESET_MODE_THERMAL           = 0x00000000
QPU_RESET_MODE_ACTIVE            = 0x00000001

# ctypes Structures
class QpuCouplerEdge(Structure):
    _fields_ = [
        ("qubit1", c_uint32),
        ("qubit2", c_uint32),
        ("coupling_type", c_uint32),
        ("fidelity_ppm", c_uint32),
    ]

class QpuTopologyQuery(Structure):
    _fields_ = [
        ("device_name", c_char * 64),
        ("technology", c_uint32),
        ("num_physical_qubits", c_uint32),
        ("num_edges", c_uint32),
        ("guard_band_distance", c_uint32),
        ("edges_buffer_ptr", c_uint64),
        ("max_edges_capacity", c_uint32),
    ]

class QpuQubitCalibration(Structure):
    _fields_ = [
        ("physical_id", c_uint32),
        ("t1_ns", c_uint64),
        ("t2_ns", c_uint64),
        ("readout_fidelity_ppm", c_uint32),
        ("single_qubit_fidelity_ppm", c_uint32),
        ("frequency_mhz", c_uint32),
        ("is_alive", c_uint32),
    ]

class QpuCalibrationQuery(Structure):
    _fields_ = [
        ("timestamp_ns", c_uint64),
        ("num_qubits", c_uint32),
        ("calibration_buffer_ptr", c_uint64),
    ]

class QpuAllocReq(Structure):
    _fields_ = [
        ("num_virtual_qubits", c_uint32),
        ("flags", c_uint32),
        ("min_t1_ns", c_uint32),
        ("min_t2_ns", c_uint32),
        ("partition_id", c_uint32),
        ("physical_qubits_allocated", c_uint32),
    ]

class QpuFreeReq(Structure):
    _fields_ = [
        ("partition_id", c_uint32),
        ("reset_mode", c_uint32),
    ]

class QpuSubmitJob(Structure):
    _fields_ = [
        ("partition_id", c_uint32),
        ("flags", c_uint32),
        ("shots", c_uint32),
        ("timeout_ms", c_uint32),
        ("qasm_or_bytecode_ptr", c_uint64),
        ("bytecode_len", c_uint32),
        ("job_id", c_uint64),
    ]

class QpuReadoutEntry(Structure):
    _fields_ = [
        ("basis_state", c_uint64),
        ("count", c_uint32),
    ]

class QpuResultReq(Structure):
    _fields_ = [
        ("job_id", c_uint64),
        ("timeout_ms", c_uint32),
        ("is_completed", c_uint32),
        ("total_shots_collected", c_uint32),
        ("num_unique_outcomes", c_uint32),
        ("entries_buffer_ptr", c_uint64),
        ("max_entries_capacity", c_uint32),
    ]

class QpuPartitionEntry(Structure):
    _fields_ = [
        ("partition_id", c_uint32),
        ("process_id", c_uint32),
        ("num_virtual_qubits", c_uint32),
        ("num_physical_qubits", c_uint32),
        ("num_guard_qubits", c_uint32),
        ("flags", c_uint32),
        ("avg_fidelity_ppm", c_uint32),
        ("physical_qubits", c_uint32 * 64),
        ("guard_qubits", c_uint32 * 64),
    ]

class QpuPartitionQuery(Structure):
    _fields_ = [
        ("num_partitions", c_uint32),
        ("max_partitions_capacity", c_uint32),
        ("partitions_buffer_ptr", c_uint64),
    ]

class QpuCryoTelemetry(Structure):
    _fields_ = [
        ("timestamp_ns", c_uint64),
        ("stage_50k_mk", c_uint32),
        ("stage_4k_mk", c_uint32),
        ("stage_still_mk", c_uint32),
        ("stage_cold_plate_mk", c_uint32),
        ("stage_mxc_mk", c_uint32),
        ("stage_mxc_uk", c_uint32),
        ("vacuum_pressure_ubar", c_uint32),
        ("is_cryo_ready", c_uint32),
    ]

class QpuResetQubitReq(Structure):
    _fields_ = [
        ("physical_id", c_uint32),
        ("reset_mode", c_uint32),
        ("duration_ns", c_uint32),
        ("success", c_uint32),
    ]

def _find_library() -> str:
    """Find the libomniq-qpu shared library."""
    custom_path = os.environ.get("OMNIQ_LIB_PATH")
    if custom_path and os.path.exists(custom_path):
        return custom_path

    # Current repository build directories
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    search_paths = [
        os.path.join(repo_root, "build", "lib", "libomniq-qpu.dylib"),
        os.path.join(repo_root, "build", "lib", "libomniq-qpu.so"),
        os.path.join(repo_root, "build_release", "lib", "libomniq-qpu.dylib"),
        os.path.join(repo_root, "build_release", "lib", "libomniq-qpu.so"),
        "/usr/local/lib/libomniq-qpu.so",
        "/usr/local/lib/libomniq-qpu.dylib",
        "/usr/lib/libomniq-qpu.so",
    ]

    for p in search_paths:
        if os.path.exists(p):
            return p

    # System library search fallback
    if sys.platform == "darwin":
        return "libomniq-qpu.dylib"
    return "libomniq-qpu.so"

_lib = None

def get_lib():
    global _lib
    if _lib is None:
        lib_path = _find_library()
        try:
            _lib = ctypes.CDLL(lib_path)
        except OSError as e:
            raise RuntimeError(f"Could not load OmniQ-QOS C library: {e}")

        # Setup function signatures
        _lib.qpu_open.argtypes = [c_char_p]
        _lib.qpu_open.restype = c_void_p

        _lib.qpu_close.argtypes = [c_void_p]
        _lib.qpu_close.restype = None

        _lib.qpu_is_ipc.argtypes = [c_void_p]
        _lib.qpu_is_ipc.restype = c_bool

        _lib.qpu_get_topology.argtypes = [c_void_p, POINTER(QpuTopologyQuery), POINTER(QpuCouplerEdge), c_uint32]
        _lib.qpu_get_topology.restype = c_int

        _lib.qpu_get_calibration.argtypes = [c_void_p, POINTER(QpuQubitCalibration), c_uint32, POINTER(c_uint64)]
        _lib.qpu_get_calibration.restype = c_int

        _lib.qpu_alloc.argtypes = [c_void_p, c_uint32, c_uint32, POINTER(c_uint32)]
        _lib.qpu_alloc.restype = c_int

        _lib.qpu_free.argtypes = [c_void_p, c_uint32]
        _lib.qpu_free.restype = c_int

        _lib.qpu_submit.argtypes = [c_void_p, c_uint32, c_char_p, c_uint32, c_uint32, POINTER(c_uint64)]
        _lib.qpu_submit.restype = c_int

        _lib.qpu_wait.argtypes = [c_void_p, c_uint64, c_uint32, POINTER(QpuReadoutEntry), c_uint32, POINTER(c_uint32), POINTER(c_uint32)]
        _lib.qpu_wait.restype = c_int

        _lib.qpu_run_sync.argtypes = [c_void_p, c_uint32, c_char_p, c_uint32, POINTER(QpuReadoutEntry), c_uint32, POINTER(c_uint32), POINTER(c_uint32)]
        _lib.qpu_run_sync.restype = c_int

        _lib.qpu_query_partitions.argtypes = [c_void_p, POINTER(QpuPartitionEntry), c_uint32, POINTER(c_uint32)]
        _lib.qpu_query_partitions.restype = c_int

        _lib.qpu_query_cryo.argtypes = [c_void_p, POINTER(QpuCryoTelemetry)]
        _lib.qpu_query_cryo.restype = c_int

        _lib.qpu_reset_qubit.argtypes = [c_void_p, c_uint32, c_uint32]
        _lib.qpu_reset_qubit.restype = c_int

    return _lib

class QPUClient:
    """High-level Python client for OmniQ-QOS QPU Accelerator and Daemon."""

    def __init__(self, dev_path: Optional[str] = None):
        lib = get_lib()
        c_path = dev_path.encode('utf-8') if dev_path else None
        self._dev = lib.qpu_open(c_path)
        if not self._dev:
            raise RuntimeError(f"Failed to open QPU device (path: {dev_path})")
        self._lib = lib

    def close(self):
        if self._dev:
            self._lib.qpu_close(self._dev)
            self._dev = None

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    @property
    def is_ipc(self) -> bool:
        return bool(self._lib.qpu_is_ipc(self._dev)) if self._dev else False

    def get_topology(self) -> Dict[str, Any]:
        """Query physical chip architecture and coupler graph."""
        topo = QpuTopologyQuery()
        edges = (QpuCouplerEdge * 128)()
        ret = self._lib.qpu_get_topology(self._dev, byref(topo), edges, 128)
        if ret != 0:
            raise RuntimeError(f"Failed to query topology (error {ret})")

        edge_list = []
        for i in range(min(topo.num_edges, 128)):
            edge_list.append({
                "q1": edges[i].qubit1,
                "q2": edges[i].qubit2,
                "fidelity": edges[i].fidelity_ppm / 1000000.0
            })

        return {
            "device_name": topo.device_name.decode('utf-8', errors='ignore'),
            "technology": topo.technology,
            "num_physical_qubits": topo.num_physical_qubits,
            "num_edges": topo.num_edges,
            "guard_band_distance": topo.guard_band_distance,
            "edges": edge_list
        }

    def get_calibration(self) -> List[Dict[str, Any]]:
        """Query physical qubit calibration matrix (T1, T2, error rates, frequency)."""
        topo = self.get_topology()
        num_qubits = topo["num_physical_qubits"]
        cals = (QpuQubitCalibration * num_qubits)()
        ts = c_uint64(0)
        ret = self._lib.qpu_get_calibration(self._dev, cals, num_qubits, byref(ts))
        if ret != 0:
            raise RuntimeError(f"Failed to query calibration (error {ret})")

        res = []
        for i in range(num_qubits):
            res.append({
                "physical_id": cals[i].physical_id,
                "t1_us": cals[i].t1_ns / 1000.0,
                "t2_us": cals[i].t2_ns / 1000.0,
                "readout_fidelity": cals[i].readout_fidelity_ppm / 1000000.0,
                "single_qubit_fidelity": cals[i].single_qubit_fidelity_ppm / 1000000.0,
                "frequency_ghz": cals[i].frequency_mhz / 1000.0,
                "is_alive": bool(cals[i].is_alive)
            })
        return res

    def get_cryo(self) -> Dict[str, Any]:
        """Query cryogenic dilution refrigerator thermal stages."""
        cryo = QpuCryoTelemetry()
        ret = self._lib.qpu_query_cryo(self._dev, byref(cryo))
        if ret != 0:
            raise RuntimeError(f"Failed to query cryo telemetry (error {ret})")

        return {
            "timestamp_ns": cryo.timestamp_ns,
            "stage_50k_k": cryo.stage_50k_mk / 1000.0,
            "stage_4k_k": cryo.stage_4k_mk / 1000.0,
            "stage_still_mk": cryo.stage_still_mk,
            "stage_cold_plate_mk": cryo.stage_cold_plate_mk,
            "stage_mxc_mk": cryo.stage_mxc_uk / 1000.0,
            "vacuum_pressure_ubar": cryo.vacuum_pressure_ubar,
            "ready": bool(cryo.is_cryo_ready)
        }

    def get_partitions(self) -> List[Dict[str, Any]]:
        """Query active multi-tenant spatial partitions."""
        parts = (QpuPartitionEntry * 32)()
        num_parts = c_uint32(0)
        ret = self._lib.qpu_query_partitions(self._dev, parts, 32, byref(num_parts))
        if ret != 0:
            raise RuntimeError(f"Failed to query partitions (error {ret})")

        res = []
        for i in range(num_parts.value):
            p = parts[i]
            phys = [p.physical_qubits[k] for k in range(p.num_physical_qubits)]
            guards = [p.guard_qubits[k] for k in range(p.num_guard_qubits)]
            res.append({
                "partition_id": p.partition_id,
                "process_id": p.process_id,
                "num_virtual_qubits": p.num_virtual_qubits,
                "physical_qubits": phys,
                "guard_qubits": guards,
                "avg_fidelity": p.avg_fidelity_ppm / 1000000.0
            })
        return res

    def allocate(self, num_qubits: int, flags: int = QPU_ALLOC_FLAG_ISOLATED) -> int:
        """Allocate a dedicated spatial qubit partition with QMMU guard bands."""
        part_id = c_uint32(0)
        ret = self._lib.qpu_alloc(self._dev, num_qubits, flags, byref(part_id))
        if ret != 0:
            raise RuntimeError(f"Failed to allocate partition with {num_qubits} qubits (error {ret})")
        return part_id.value

    def free(self, partition_id: int) -> bool:
        """Free an allocated partition, returning qubits and guard bands to the chip pool."""
        ret = self._lib.qpu_free(self._dev, partition_id)
        return ret == 0

    def submit(self, partition_id: int, qasm_or_code: str, shots: int = 1024, flags: int = 0) -> int:
        """Submit an OpenQASM quantum circuit job to a partition."""
        c_code = qasm_or_code.encode('utf-8')
        job_id = c_uint64(0)
        ret = self._lib.qpu_submit(self._dev, partition_id, c_code, shots, flags, byref(job_id))
        if ret != 0:
            raise RuntimeError(f"Failed to submit circuit job (error {ret})")
        return job_id.value

    def wait(self, job_id: int, timeout_ms: int = 2000, num_qubits: int = 2) -> Dict[str, int]:
        """Wait for circuit execution and collect measurement readout bitstrings."""
        max_entries = 64
        entries = (QpuReadoutEntry * max_entries)()
        num_entries = c_uint32(0)
        total_shots = c_uint32(0)
        ret = self._lib.qpu_wait(self._dev, job_id, timeout_ms, entries, max_entries,
                                 byref(num_entries), byref(total_shots))
        if ret != 0:
            raise RuntimeError(f"Failed to read results for job {job_id} (error {ret})")

        counts = {}
        for i in range(num_entries.value):
            state_int = entries[i].basis_state
            # Format bitstring
            bitstr = format(state_int, f'0{num_qubits}b')
            counts[bitstr] = entries[i].count

        return counts

    def run_sync(self, qasm_code: str, num_qubits: int = 2, shots: int = 1024) -> Dict[str, int]:
        """Allocate partition, execute circuit, read results, and free partition atomically."""
        pid = self.allocate(num_qubits)
        try:
            jid = self.submit(pid, qasm_code, shots=shots)
            return self.wait(jid, timeout_ms=2000, num_qubits=num_qubits)
        finally:
            self.free(pid)

    def reset_qubit(self, physical_id: int, active: bool = True) -> bool:
        """Dispatch microwave active reset pulse to return physical qubit to |0>."""
        mode = QPU_RESET_MODE_ACTIVE if active else QPU_RESET_MODE_THERMAL
        ret = self._lib.qpu_reset_qubit(self._dev, physical_id, mode)
        return ret == 0
