#!/usr/bin/env python3
"""
OmniQ Magnonic Quantum Computing Demonstration:
1. Logical Layer: Translating Dual-Rail magnonic gates (Beam splitters, Cross-Kerr) to standard qubits (H, X, CZ).
2. Physical Layer: Simulating d-dimensional Qudits with sparse operations (Aharonov-Casher RZ, CSUM) and Dual-Rail Magnon Loss.
"""

import sys
import os
import math

sys.path.insert(0, os.path.abspath('omniq-python'))
sys.path.insert(0, os.path.abspath('libomniq-core/build'))

from omniq import (
    MagnonicGate,
    DualRailCompiler,
    create_dual_rail_grover,
    NoiseModel
)

def run_logical_demo():
    print("==================================================")
    print(" 1. Logical Layer: Dual-Rail Magnonic Simulation  ")
    print("==================================================")
    
    # Target value 2 in 2-qubit Grover search
    target = 2
    circuit = create_dual_rail_grover(num_qubits=2, target_value=target)
    
    print(f"Constructed Logical Circuit for Grover's Search (Target = {target}):")
    print(f"  Total Gates: {len(circuit.gates)}")
    print(f"  Qubits: {circuit.num_qubits}")
    print("  Circuit Dict Representation:")
    print(circuit.to_dict())

def run_physical_demo():
    print("\n==================================================")
    print(" 2. Physical Layer: Sparse Qudits & Magnon Loss   ")
    print("==================================================")
    
    try:
        import _omniq_core
    except ImportError:
        print("[!] _omniq_core C++ extension not loaded. Skipping physical execution.")
        return
        
    print("Creating 2 Qudits with dimension d=3 (Qutrit magnonic modes)...")
    q_sv = _omniq_core.QuditStatevector(num_qudits=2, dimension=3)
    print(f"Initial State: {q_sv.to_string()}")
    
    print("\nApplying Aharonov-Casher RZ(pi/3) rotation on Qudit 0...")
    q_sv.apply_aharonov_casher_rz(0, math.pi / 3)
    
    print("Applying CSUM (Controlled-SUM) Kerr interaction between Qudit 0 and 1...")
    q_sv.apply_csum(0, 1)
    
    print(f"Evolved State: {q_sv.to_string()}")
    
    print("\nInitializing Dual-Rail Magnon Decay Noise Channel (gamma = 0.05)...")
    d_channel = _omniq_core.DualRailDampingChannel(0.05)
    print(f"Channel Name: {d_channel.get_name()} ({d_channel.get_description()})")
    print(f"Generated {len(d_channel.get_kraus_operators())} trace-preserving Kraus operators.")

if __name__ == '__main__':
    run_logical_demo()
    run_physical_demo()
