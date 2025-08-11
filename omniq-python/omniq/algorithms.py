"""
Quantum algorithms module for OmniQ.

Provides Python bindings to quantum algorithms like Grover's and QPE.
"""

import numpy as np
from typing import List, Callable, Optional, Union, Dict, Any
from ._internals import _GroversAlgorithm, _QPE

class GroversAlgorithm:
    """
    Grover's algorithm implementation.
    
    This class provides a high-level interface for running Grover's algorithm
    for unstructured search problems.
    """
    
    def __init__(self, num_qubits: int, oracle: Callable[[List[int]], bool], num_solutions: int = 1):
        """
        Initialize Grover's algorithm.
        
        Args:
            num_qubits: Number of qubits in the search space
            oracle: Oracle function that marks solutions
            num_solutions: Number of solutions (for optimal iteration count)
        """
        self._grover = _GroversAlgorithm(num_qubits, oracle, num_solutions)
        self.num_qubits = num_qubits
        self.num_solutions = num_solutions
    
    def set_iterations(self, iterations: int) -> None:
        """
        Set the number of Grover iterations.
        
        Args:
            iterations: Number of iterations
        """
        self._grover.set_iterations(iterations)
    
    def get_optimal_iterations(self) -> int:
        """
        Calculate the optimal number of iterations.
        
        Returns:
            Optimal number of iterations
        """
        return self._grover.get_optimal_iterations()
    
    def execute(self, num_shots: int = 1000) -> List[int]:
        """
        Execute Grover's algorithm with measurements.
        
        Args:
            num_shots: Number of measurement shots
            
        Returns:
            List of measurement results
        """
        return self._grover.execute_with_measurements(num_shots)
    
    def get_success_probability(self) -> float:
        """
        Get the theoretical success probability.
        
        Returns:
            Success probability
        """
        return self._grover.get_success_probability()
    
    def build_circuit(self):
        """
        Build the complete Grover circuit.
        
        Returns:
            Quantum circuit implementing Grover's algorithm
        """
        from .circuit import Circuit
        circuit_data = self._grover.build_circuit()
        # Convert C++ circuit to Python circuit
        # This would need proper implementation
        return Circuit(self.num_qubits)
    
    def __str__(self) -> str:
        """String representation of Grover's algorithm."""
        return f"GroversAlgorithm({self.num_qubits} qubits, {self.num_solutions} solutions)"
    
    def __repr__(self) -> str:
        """Detailed string representation of Grover's algorithm."""
        return f"GroversAlgorithm(num_qubits={self.num_qubits}, num_solutions={self.num_solutions})"

class QPE:
    """
    Quantum Phase Estimation implementation.
    
    This class provides a high-level interface for running QPE to estimate
    eigenvalues of unitary operators.
    """
    
    def __init__(self, num_precision_qubits: int, num_eigenstate_qubits: int, 
                 unitary: Callable):
        """
        Initialize QPE.
        
        Args:
            num_precision_qubits: Number of precision qubits
            num_eigenstate_qubits: Number of eigenstate qubits
            unitary: Unitary operator function
        """
        self._qpe = _QPE(num_precision_qubits, num_eigenstate_qubits, unitary)
        self.num_precision_qubits = num_precision_qubits
        self.num_eigenstate_qubits = num_eigenstate_qubits
    
    def set_eigenvalues_and_states(self, eigenvalues: List[float], 
                                 eigenstates: List[complex]) -> None:
        """
        Set eigenvalues and eigenstates.
        
        Args:
            eigenvalues: List of eigenvalues
            eigenstates: List of eigenstate amplitudes
        """
        self._qpe.set_eigenvalues_and_states(eigenvalues, eigenstates)
    
    def execute(self, num_shots: int = 1000) -> List[float]:
        """
        Execute QPE with measurements.
        
        Args:
            num_shots: Number of measurement shots
            
        Returns:
            List of phase estimates (normalized to [0, 1))
        """
        return self._qpe.execute_with_measurements(num_shots)
    
    def build_circuit(self):
        """
        Build the complete QPE circuit.
        
        Returns:
            Quantum circuit implementing QPE
        """
        from .circuit import Circuit
        circuit_data = self._qpe.build_circuit()
        # Convert C++ circuit to Python circuit
        # This would need proper implementation
        return Circuit(self.num_precision_qubits + self.num_eigenstate_qubits)
    
    def __str__(self) -> str:
        """String representation of QPE."""
        return f"QPE({self.num_precision_qubits} precision, {self.num_eigenstate_qubits} eigenstate qubits)"
    
    def __repr__(self) -> str:
        """Detailed string representation of QPE."""
        return f"QPE(num_precision_qubits={self.num_precision_qubits}, num_eigenstate_qubits={self.num_eigenstate_qubits})"

# Utility functions for creating oracles
def create_database_oracle(target_value: int) -> Callable[[List[int]], bool]:
    """
    Create a simple oracle for database search.
    
    Args:
        target_value: The value to search for
        
    Returns:
        Oracle function
    """
    def oracle(input_bits: List[int]) -> bool:
        # Convert binary input to integer
        value = 0
        for i, bit in enumerate(input_bits):
            value |= (bit << i)
        return value == target_value
    
    return oracle

def create_sat_oracle(clauses: List[List[int]], num_variables: int) -> Callable[[List[int]], bool]:
    """
    Create an oracle for SAT problems.
    
    Args:
        clauses: List of clauses (each clause is a list of literals)
        num_variables: Number of variables
        
    Returns:
        Oracle function
    """
    def oracle(input_bits: List[int]) -> bool:
        # Check if the assignment satisfies all clauses
        for clause in clauses:
            clause_satisfied = False
            for literal in clause:
                var = abs(literal) - 1  # Variables are 1-indexed in DIMACS format
                if var >= num_variables:
                    continue
                
                var_value = input_bits[var] if var < len(input_bits) else False
                literal_value = var_value if literal > 0 else not var_value
                
                if literal_value:
                    clause_satisfied = True
                    break
            
            if not clause_satisfied:
                return False  # This clause is not satisfied
        
        return True  # All clauses are satisfied
    
    return oracle

def create_graph_coloring_oracle(edges: List[tuple], num_vertices: int, 
                               num_colors: int) -> Callable[[List[int]], bool]:
    """
    Create an oracle for graph coloring problems.
    
    Args:
        edges: List of edges as (vertex1, vertex2) tuples
        num_vertices: Number of vertices
        num_colors: Number of colors
        
    Returns:
        Oracle function
    """
    def oracle(input_bits: List[int]) -> bool:
        # Check if the coloring is valid (no adjacent vertices have the same color)
        for v1, v2 in edges:
            if v1 >= num_vertices or v2 >= num_vertices:
                continue
            
            # Calculate color indices in the input
            color1_idx = v1 * num_colors
            color2_idx = v2 * num_colors
            
            # Find the assigned colors
            color1 = -1
            color2 = -1
            for c in range(num_colors):
                if color1_idx + c < len(input_bits) and input_bits[color1_idx + c]:
                    color1 = c
                if color2_idx + c < len(input_bits) and input_bits[color2_idx + c]:
                    color2 = c
            
            # Check if adjacent vertices have the same color
            if color1 == color2 and color1 != -1:
                return False  # Invalid coloring
        
        return True  # Valid coloring
    
    return oracle

# Utility functions for QPE
def create_phase_rotation_unitary(phase: float) -> Callable:
    """
    Create a simple phase rotation unitary.
    
    Args:
        phase: Phase to rotate by
        
    Returns:
        Unitary operator function
    """
    def unitary(state, start_qubit):
        # This would apply the phase rotation to the state
        # Implementation depends on the state representation
        pass
    
    return unitary

def phase_to_eigenvalue(phase_measurement: float, num_precision_qubits: int) -> float:
    """
    Convert phase measurement to eigenvalue.
    
    Args:
        phase_measurement: Phase measurement (normalized to [0, 1))
        num_precision_qubits: Number of precision qubits
        
    Returns:
        Eigenvalue
    """
    return 2.0 * np.pi * phase_measurement

def eigenvalue_to_phase(eigenvalue: float, num_precision_qubits: int) -> float:
    """
    Convert eigenvalue to phase.
    
    Args:
        eigenvalue: Eigenvalue
        num_precision_qubits: Number of precision qubits
        
    Returns:
        Phase (normalized to [0, 1))
    """
    phase = eigenvalue / (2.0 * np.pi)
    return phase - np.floor(phase)  # Ensure phase is in [0, 1)

def estimate_precision(num_precision_qubits: int) -> float:
    """
    Estimate precision of QPE.
    
    Args:
        num_precision_qubits: Number of precision qubits
        
    Returns:
        Precision in bits
    """
    return float(num_precision_qubits)

def calculate_success_probability(num_precision_qubits: int, true_phase: float) -> float:
    """
    Calculate success probability for given precision.
    
    Args:
        num_precision_qubits: Number of precision qubits
        true_phase: True phase value
        
    Returns:
        Success probability
    """
    phase_error = abs(true_phase - eigenvalue_to_phase(true_phase, num_precision_qubits))
    success_prob = np.cos(np.pi * phase_error * (1 << num_precision_qubits))
    return success_prob ** 2 