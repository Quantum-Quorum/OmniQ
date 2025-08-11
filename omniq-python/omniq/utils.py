"""
Utility functions for OmniQ.

Provides helper functions for common quantum computing tasks.
"""

import numpy as np
from typing import List, Optional
from .circuit import Circuit, Statevector, DensityMatrix

def create_bell_state() -> Statevector:
    """
    Create a Bell state (|00⟩ + |11⟩)/√2.
    
    Returns:
        Bell state as Statevector
    """
    circuit = Circuit(2)
    circuit.add_gate('H', [0])
    circuit.add_gate('CNOT', [0, 1])
    return circuit.execute()

def create_ghz_state(num_qubits: int) -> Statevector:
    """
    Create a GHZ state (|0...0⟩ + |1...1⟩)/√2.
    
    Args:
        num_qubits: Number of qubits
        
    Returns:
        GHZ state as Statevector
    """
    circuit = Circuit(num_qubits)
    circuit.add_gate('H', [0])
    for i in range(1, num_qubits):
        circuit.add_gate('CNOT', [0, i])
    return circuit.execute()

def measure_expectation(state: Statevector, qubit: int, observable: str = "Z") -> float:
    """
    Measure expectation value of an observable.
    
    Args:
        state: Quantum state
        qubit: Qubit index
        observable: Observable to measure
        
    Returns:
        Expectation value
    """
    return state.measure_expectation(qubit, observable)

def calculate_fidelity(state1: Statevector, state2: Statevector) -> float:
    """
    Calculate fidelity between two quantum states.
    
    Args:
        state1: First quantum state
        state2: Second quantum state
        
    Returns:
        Fidelity value
    """
    # Calculate |⟨ψ₁|ψ₂⟩|²
    amps1 = state1.get_amplitudes()
    amps2 = state2.get_amplitudes()
    
    overlap = np.sum(np.conj(amps1) * amps2)
    fidelity = np.abs(overlap) ** 2
    
    return float(fidelity)

def random_circuit(num_qubits: int, depth: int, seed: Optional[int] = None) -> Circuit:
    """
    Generate a random quantum circuit.
    
    Args:
        num_qubits: Number of qubits
        depth: Circuit depth
        seed: Random seed
        
    Returns:
        Random quantum circuit
    """
    if seed is not None:
        np.random.seed(seed)
    
    circuit = Circuit(num_qubits)
    gates = ['H', 'X', 'Y', 'Z', 'RX', 'RY', 'RZ']
    
    for layer in range(depth):
        # Apply random single-qubit gates
        for qubit in range(num_qubits):
            gate = np.random.choice(gates)
            if gate in ['RX', 'RY', 'RZ']:
                angle = np.random.uniform(0, 2 * np.pi)
                circuit.add_gate(gate, [qubit], angle=angle)
            else:
                circuit.add_gate(gate, [qubit])
        
        # Apply random two-qubit gates
        if num_qubits > 1:
            for _ in range(num_qubits // 2):
                qubit1 = np.random.randint(0, num_qubits)
                qubit2 = np.random.randint(0, num_qubits)
                if qubit1 != qubit2:
                    circuit.add_gate('CNOT', [qubit1, qubit2])
    
    return circuit

def create_quantum_state_from_amplitudes(amplitudes: np.ndarray) -> Statevector:
    """
    Create a quantum state from given amplitudes.
    
    Args:
        amplitudes: Complex amplitudes array
        
    Returns:
        Quantum state
    """
    return Statevector(amplitudes=amplitudes)

def create_mixed_state_from_matrix(matrix: np.ndarray) -> DensityMatrix:
    """
    Create a mixed state from given density matrix.
    
    Args:
        matrix: Density matrix
        
    Returns:
        Mixed quantum state
    """
    return DensityMatrix(matrix=matrix) 