#!/usr/bin/env python3
"""
Simple test script for OmniQ Python API.

This script demonstrates basic usage of the OmniQ library.
"""

import numpy as np
import omniq

def test_basic_circuit():
    """Test basic circuit creation and execution."""
    print("=== Testing Basic Circuit ===")
    
    # Create a 2-qubit circuit
    circuit = omniq.Circuit(2)
    
    # Add some gates
    circuit.add_gate('H', [0])  # Hadamard on qubit 0
    circuit.add_gate('CNOT', [0, 1])  # CNOT with control=0, target=1
    
    print(f"Circuit: {circuit}")
    print(f"QASM: {circuit.to_qasm()}")
    
    # Execute the circuit
    result = circuit.execute()
    print(f"Final state: {result}")
    
    # Measure the qubits
    measurement1 = result.measure(0)
    measurement2 = result.measure(1)
    print(f"Measurements: qubit 0 = {measurement1}, qubit 1 = {measurement2}")

def test_grovers_algorithm():
    """Test Grover's algorithm for database search."""
    print("\n=== Testing Grover's Algorithm ===")
    
    # Create a simple database search oracle
    target_value = 5
    oracle = omniq.create_database_oracle(target_value)
    
    # Create Grover's algorithm
    grover = omniq.GroversAlgorithm(4, oracle, 1)
    
    print(f"Number of qubits: {grover.num_qubits}")
    print(f"Number of solutions: {grover.num_solutions}")
    print(f"Optimal iterations: {grover.get_optimal_iterations()}")
    print(f"Success probability: {grover.get_success_probability():.4f}")
    
    # Execute the algorithm
    results = grover.execute(num_shots=100)
    
    # Count results
    from collections import Counter
    result_counts = Counter(results)
    
    print("Top 5 results:")
    for result, count in result_counts.most_common(5):
        print(f"  Value {result}: {count} times")
    
    # Check if target was found
    if target_value in result_counts:
        print(f"✓ Target value {target_value} found {result_counts[target_value]} times!")
    else:
        print(f"✗ Target value {target_value} not found")

def test_statevector_operations():
    """Test statevector operations."""
    print("\n=== Testing Statevector Operations ===")
    
    # Create a 1-qubit statevector
    state = omniq.Statevector(1)
    print(f"Initial state: {state}")
    
    # Apply some gates
    state.apply_gate('H', 0)
    print(f"After Hadamard: {state}")
    
    state.apply_gate('X', 0)
    print(f"After X gate: {state}")
    
    # Get amplitudes
    amplitudes = state.get_amplitudes()
    print(f"Amplitudes: {amplitudes}")
    
    # Calculate expectation value
    expectation = state.measure_expectation(0, "Z")
    print(f"⟨Z⟩ = {expectation:.4f}")

def test_density_matrix():
    """Test density matrix operations."""
    print("\n=== Testing Density Matrix ===")
    
    # Create a 1-qubit density matrix
    rho = omniq.DensityMatrix(1)
    print(f"Initial density matrix: {rho}")
    
    # Apply some gates
    rho.apply_gate('H', 0)
    print(f"After Hadamard: {rho}")
    
    # Get matrix
    matrix = rho.get_matrix()
    print(f"Density matrix:\n{matrix}")
    
    # Calculate properties
    purity = rho.get_purity()
    entropy = rho.get_entropy()
    print(f"Purity: {purity:.4f}")
    print(f"Entropy: {entropy:.4f}")

def test_utility_functions():
    """Test utility functions."""
    print("\n=== Testing Utility Functions ===")
    
    # Test version info
    version_info = omniq.version_info()
    print(f"Version: {version_info['version']}")
    print(f"Author: {version_info['author']}")
    
    # Test constants
    print(f"Default device: {omniq.DEFAULT_DEVICE}")
    print(f"Supported devices: {omniq.SUPPORTED_DEVICES}")

def main():
    """Main test function."""
    print("OmniQ Python API Test")
    print("====================")
    
    try:
        test_basic_circuit()
        test_grovers_algorithm()
        test_statevector_operations()
        test_density_matrix()
        test_utility_functions()
        
        print("\n=== All Python Tests Completed! ===")
        
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()
        return 1
    
    return 0

if __name__ == "__main__":
    exit(main()) 