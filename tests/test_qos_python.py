"""
OmniQ-QOS: Python SDK Integration Tests
tests/test_qos_python.py

Copyright (c) 2026 Quantum Quorum
"""

import os
import sys
import pytest

# Add omniq-python to PYTHONPATH
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "omniq-python")))

from omniq import Circuit, QPUDevice, SimulatorDevice, QPUClient

def test_circuit_to_qasm_bell_state():
    qc = Circuit(2)
    qc.h(0)
    qc.cx(0, 1)
    qc.measure(0, 0)
    qc.measure(1, 1)
    
    qasm = qc.to_qasm()
    assert "OPENQASM 2.0;" in qasm
    assert 'include "qelib1.inc";' in qasm
    assert "qreg q[2];" in qasm
    assert "creg c[2];" in qasm
    assert "h q[0];" in qasm
    assert "cx q[0], q[1];" in qasm
    assert "measure q[0] -> c[0];" in qasm
    assert "measure q[1] -> c[1];" in qasm

def test_circuit_to_qasm_extended_gates():
    qc = Circuit(3)
    qc.rx(0, "3.14159265")
    qc.ry(1, 1.5707963)
    qc.rz(2, 0.785398)
    qc.s(0)
    qc.sdg(1)
    qc.t(2)
    qc.tdg(0)
    qc.swap(1, 2)
    
    qasm = qc.to_qasm()
    assert "rx(3.14159265) q[0];" in qasm
    assert "ry(1.5707963) q[1];" in qasm
    assert "rz(0.785398) q[2];" in qasm
    assert "s q[0];" in qasm
    assert "sdg q[1];" in qasm
    assert "t q[2];" in qasm
    assert "tdg q[0];" in qasm
    assert "swap q[1], q[2];" in qasm

def test_qpu_client_telemetry_queries():
    client = QPUClient(":force-direct:")
    try:
        topo = client.get_topology()
        assert topo["num_physical_qubits"] >= 32
        assert topo["technology"] == 1  # Superconducting Transmon
        assert len(topo["edges"]) > 0

        cals = client.get_calibration()
        assert len(cals) == topo["num_physical_qubits"]
        assert cals[0]["t1_us"] > 0
        assert cals[0]["t2_us"] > 0
        assert cals[0]["readout_fidelity"] > 0.95

        cryo = client.get_cryo()
        assert cryo["stage_50k_k"] > 40.0
        assert cryo["stage_4k_k"] < 5.0
        assert cryo["stage_mxc_mk"] < 50.0  # Under 50 mK base temp
        assert cryo["ready"] is True
    finally:
        client.close()

def test_qpu_device_bell_state_execution():
    qpu = QPUDevice(dev_path=":force-direct:")
    qc = Circuit(2)
    qc.h(0)
    qc.cx(0, 1)
    qc.measure(0, 0)
    qc.measure(1, 1)

    counts = qpu.execute(qc, shots=2048)
    assert isinstance(counts, dict)
    assert len(counts) > 0
    total_shots = sum(counts.values())
    assert total_shots == 2048
    # Bell state produces |00> and |11>
    assert "00" in counts
    assert "11" in counts
    assert counts["00"] > 500
    assert counts["11"] > 500
    # No |01> or |10> for ideal Bell state
    assert counts.get("01", 0) == 0
    assert counts.get("10", 0) == 0

def test_qpu_active_reset():
    qpu = QPUDevice(dev_path=":force-direct:")
    res = qpu.reset_qubit(0, active=True)
    assert res is True
