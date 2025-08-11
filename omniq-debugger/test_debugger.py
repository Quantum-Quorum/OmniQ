#!/usr/bin/env python3
"""
Test script for OmniQ Debugger
This script simulates quantum operations to test the debugger functionality.
"""

import time
import random
import math

def simulate_quantum_state():
    """Simulate a simple quantum state evolution."""
    print("=== OmniQ Debugger Test ===")
    print("Simulating quantum circuit execution...")
    
    # Simulate a 2-qubit Bell state circuit
    steps = [
        "Initialize |00⟩ state",
        "Apply Hadamard to qubit 0",
        "Apply CNOT(0,1)",
        "Measure qubits"
    ]
    
    for i, step in enumerate(steps):
        print(f"Step {i+1}: {step}")
        
        # Simulate some quantum state data
        if i == 1:  # After Hadamard
            print("  Qubit 0: |ψ⟩ = 0.707|0⟩ + 0.707|1⟩")
            print("  Qubit 1: |ψ⟩ = 1.000|0⟩ + 0.000|1⟩")
        elif i == 2:  # After CNOT
            print("  Qubit 0: |ψ⟩ = 0.707|0⟩ + 0.000|1⟩")
            print("  Qubit 1: |ψ⟩ = 0.707|0⟩ + 0.000|1⟩")
            print("  Bell state: |ψ⟩ = 0.707|00⟩ + 0.707|11⟩")
        
        time.sleep(0.5)  # Simulate processing time
    
    print("\n=== Measurement Results ===")
    measurements = []
    for _ in range(100):
        # Simulate measurement with 50/50 probability for Bell state
        result = random.choice([0, 1])
        measurements.append(result)
    
    print(f"|00⟩ count: {measurements.count(0)}")
    print(f"|11⟩ count: {measurements.count(1)}")
    print(f"Entanglement fidelity: {abs(measurements.count(0) - measurements.count(1)) / 100:.3f}")

def test_quantum_properties():
    """Test various quantum properties calculations."""
    print("\n=== Quantum Properties Test ===")
    
    # Simulate density matrix for Bell state
    rho = [
        [0.5, 0.0, 0.0, 0.5],
        [0.0, 0.0, 0.0, 0.0],
        [0.0, 0.0, 0.0, 0.0],
        [0.5, 0.0, 0.0, 0.5]
    ]
    
    print("Density Matrix (Bell state):")
    for row in rho:
        print(f"  {row}")
    
    # Calculate purity
    purity = sum(rho[i][i] for i in range(4))
    print(f"Purity: {purity:.3f}")
    
    # Calculate von Neumann entropy
    eigenvalues = [0.5, 0.5, 0.0, 0.0]
    entropy = -sum(e * math.log2(e) if e > 0 else 0 for e in eigenvalues)
    print(f"Von Neumann Entropy: {entropy:.3f}")
    
    # Calculate concurrence
    concurrence = 1.0  # For Bell state
    print(f"Concurrence: {concurrence:.3f}")

def test_bloch_sphere():
    """Test Bloch sphere visualization."""
    print("\n=== Bloch Sphere Test ===")
    
    # Test different qubit states
    states = [
        ("|0⟩", 0.0, 0.0),
        ("|1⟩", math.pi, 0.0),
        ("|+⟩", math.pi/2, 0.0),
        ("|-⟩", math.pi/2, math.pi),
        ("|i⟩", math.pi/2, math.pi/2),
        ("|-i⟩", math.pi/2, -math.pi/2)
    ]
    
    for name, theta, phi in states:
        x = math.sin(theta) * math.cos(phi)
        y = math.sin(theta) * math.sin(phi)
        z = math.cos(theta)
        print(f"{name}: θ={theta:.2f}, φ={phi:.2f} → (x={x:.3f}, y={y:.3f}, z={z:.3f})")

if __name__ == "__main__":
    simulate_quantum_state()
    test_quantum_properties()
    test_bloch_sphere()