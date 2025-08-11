"""
Circuit module for OmniQ.

Provides Python bindings to the core quantum circuit functionality.
"""

import numpy as np
from typing import List, Optional, Union, Dict, Any
from ._internals import _Circuit, _Statevector, _DensityMatrix

class Circuit:
    """
    Quantum circuit representation.
    
    This class provides a high-level interface for constructing and executing
    quantum circuits in OmniQ.
    """
    
    def __init__(self, num_qubits: int, num_classical_bits: int = 0):
        """
        Initialize a quantum circuit.
        
        Args:
            num_qubits: Number of qubits in the circuit
            num_classical_bits: Number of classical bits (default: 0)
        """
        self._circuit = _Circuit(num_qubits, num_classical_bits)
        self.num_qubits = num_qubits
        self.num_classical_bits = num_classical_bits
    
    def add_gate(self, gate_name: str, qubits: List[int], **kwargs) -> 'Circuit':
        """
        Add a gate to the circuit.
        
        Args:
            gate_name: Name of the gate (e.g., 'H', 'X', 'CNOT', 'RX')
            qubits: List of qubit indices
            **kwargs: Additional parameters for parameterized gates
            
        Returns:
            self for method chaining
        """
        if gate_name.upper() == 'H':
            self._circuit.add_hadamard(qubits[0])
        elif gate_name.upper() == 'X':
            self._circuit.add_pauli_x(qubits[0])
        elif gate_name.upper() == 'Y':
            self._circuit.add_pauli_y(qubits[0])
        elif gate_name.upper() == 'Z':
            self._circuit.add_pauli_z(qubits[0])
        elif gate_name.upper() == 'CNOT':
            self._circuit.add_cnot(qubits[0], qubits[1])
        elif gate_name.upper() == 'SWAP':
            self._circuit.add_swap(qubits[0], qubits[1])
        elif gate_name.upper() == 'RX':
            self._circuit.add_rotation_x(qubits[0], kwargs.get('angle', 0.0))
        elif gate_name.upper() == 'RY':
            self._circuit.add_rotation_y(qubits[0], kwargs.get('angle', 0.0))
        elif gate_name.upper() == 'RZ':
            self._circuit.add_rotation_z(qubits[0], kwargs.get('angle', 0.0))
        elif gate_name.upper() == 'P':
            self._circuit.add_phase_shift(qubits[0], kwargs.get('angle', 0.0))
        else:
            raise ValueError(f"Unknown gate: {gate_name}")
        
        return self
    
    def add_measurement(self, qubit: int, basis: str = "computational") -> 'Circuit':
        """
        Add a measurement to the circuit.
        
        Args:
            qubit: Qubit index to measure
            basis: Measurement basis (default: "computational")
            
        Returns:
            self for method chaining
        """
        self._circuit.add_measurement(qubit, basis)
        return self
    
    def execute(self, initial_state: Optional['Statevector'] = None) -> 'Statevector':
        """
        Execute the circuit and return the final state.
        
        Args:
            initial_state: Initial quantum state (optional)
            
        Returns:
            Final quantum state
            
        Raises:
            ValueError: If circuit is empty or invalid
            RuntimeError: If execution fails
        """
        if not self._circuit.get_gate_count():
            raise ValueError("Cannot execute empty circuit")
        
        try:
            if initial_state is None:
                return Statevector(self._circuit.execute_statevector())
            else:
                if initial_state.num_qubits != self.num_qubits:
                    raise ValueError(f"Initial state has {initial_state.num_qubits} qubits, "
                                   f"but circuit expects {self.num_qubits} qubits")
                return Statevector(self._circuit.execute_statevector(initial_state._statevector))
        except Exception as e:
            raise RuntimeError(f"Circuit execution failed: {str(e)}") from e
    
    def execute_density_matrix(self, initial_state: Optional['DensityMatrix'] = None) -> 'DensityMatrix':
        """
        Execute the circuit and return the final density matrix.
        
        Args:
            initial_state: Initial density matrix (optional)
            
        Returns:
            Final density matrix
        """
        if initial_state is None:
            return DensityMatrix(self._circuit.execute_density_matrix())
        else:
            return DensityMatrix(self._circuit.execute_density_matrix(initial_state._density_matrix))
    
    def optimize(self) -> 'Circuit':
        """
        Optimize the circuit by removing redundant gates.
        
        Returns:
            self for method chaining
        """
        self._circuit.optimize()
        return self
    
    def to_qasm(self) -> str:
        """
        Convert the circuit to OpenQASM format.
        
        Returns:
            OpenQASM string representation
        """
        return self._circuit.to_qasm()
    
    def __str__(self) -> str:
        """String representation of the circuit."""
        return self._circuit.to_string()
    
    def __repr__(self) -> str:
        """Detailed string representation of the circuit."""
        return f"Circuit({self.num_qubits} qubits, {self.num_classical_bits} classical bits)"

class Statevector:
    """
    Quantum statevector representation.
    
    This class represents a pure quantum state as a complex vector.
    """
    
    def __init__(self, num_qubits: int = 0, amplitudes: Optional[np.ndarray] = None):
        """
        Initialize a quantum statevector.
        
        Args:
            num_qubits: Number of qubits (if creating from scratch)
            amplitudes: Complex amplitudes (if creating from existing state)
        """
        if amplitudes is not None:
            self._statevector = _Statevector(amplitudes.tolist())
        else:
            self._statevector = _Statevector(num_qubits)
        
        self.num_qubits = self._statevector.get_num_qubits()
    
    def apply_gate(self, gate_name: str, qubit: int, **kwargs) -> 'Statevector':
        """
        Apply a quantum gate to the state.
        
        Args:
            gate_name: Name of the gate
            qubit: Qubit index
            **kwargs: Additional parameters
            
        Returns:
            self for method chaining
        """
        if gate_name.upper() == 'H':
            self._statevector.apply_hadamard(qubit)
        elif gate_name.upper() == 'X':
            self._statevector.apply_pauli_x(qubit)
        elif gate_name.upper() == 'Y':
            self._statevector.apply_pauli_y(qubit)
        elif gate_name.upper() == 'Z':
            self._statevector.apply_pauli_z(qubit)
        elif gate_name.upper() == 'RX':
            self._statevector.apply_rotation_x(qubit, kwargs.get('angle', 0.0))
        elif gate_name.upper() == 'RY':
            self._statevector.apply_rotation_y(qubit, kwargs.get('angle', 0.0))
        elif gate_name.upper() == 'RZ':
            self._statevector.apply_rotation_z(qubit, kwargs.get('angle', 0.0))
        else:
            raise ValueError(f"Unknown gate: {gate_name}")
        
        return self
    
    def measure(self, qubit: int) -> int:
        """
        Measure a qubit in the computational basis.
        
        Args:
            qubit: Qubit index to measure
            
        Returns:
            Measurement result (0 or 1)
        """
        return self._statevector.measure(qubit)
    
    def measure_expectation(self, qubit: int, observable: str = "Z") -> float:
        """
        Calculate the expectation value of an observable.
        
        Args:
            qubit: Qubit index
            observable: Observable to measure (default: "Z")
            
        Returns:
            Expectation value
        """
        return self._statevector.measure_expectation(qubit, observable)
    
    def get_amplitudes(self) -> np.ndarray:
        """
        Get the complex amplitudes as a numpy array.
        
        Returns:
            Complex amplitudes array
        """
        return np.array(self._statevector.get_amplitudes())
    
    def set_amplitudes(self, amplitudes: np.ndarray) -> None:
        """
        Set the complex amplitudes.
        
        Args:
            amplitudes: Complex amplitudes array
        """
        self._statevector.set_amplitudes(amplitudes.tolist())
    
    def normalize(self) -> 'Statevector':
        """
        Normalize the statevector.
        
        Returns:
            self for method chaining
        """
        self._statevector.normalize()
        return self
    
    def get_norm(self) -> float:
        """
        Get the norm of the statevector.
        
        Returns:
            Norm value
        """
        return self._statevector.get_norm()
    
    def __str__(self) -> str:
        """String representation of the statevector."""
        return self._statevector.to_string()
    
    def __repr__(self) -> str:
        """Detailed string representation of the statevector."""
        return f"Statevector({self.num_qubits} qubits)"

class DensityMatrix:
    """
    Quantum density matrix representation.
    
    This class represents a mixed quantum state as a density matrix.
    """
    
    def __init__(self, num_qubits: int = 0, matrix: Optional[np.ndarray] = None):
        """
        Initialize a quantum density matrix.
        
        Args:
            num_qubits: Number of qubits (if creating from scratch)
            matrix: Density matrix (if creating from existing matrix)
        """
        if matrix is not None:
            self._density_matrix = _DensityMatrix(matrix.tolist())
        else:
            self._density_matrix = _DensityMatrix(num_qubits)
        
        self.num_qubits = self._density_matrix.get_num_qubits()
    
    def apply_gate(self, gate_name: str, qubit: int) -> 'DensityMatrix':
        """
        Apply a quantum gate to the density matrix.
        
        Args:
            gate_name: Name of the gate
            qubit: Qubit index
            
        Returns:
            self for method chaining
        """
        if gate_name.upper() == 'H':
            self._density_matrix.apply_hadamard(qubit)
        elif gate_name.upper() == 'X':
            self._density_matrix.apply_pauli_x(qubit)
        elif gate_name.upper() == 'Y':
            self._density_matrix.apply_pauli_y(qubit)
        elif gate_name.upper() == 'Z':
            self._density_matrix.apply_pauli_z(qubit)
        elif gate_name.upper() == 'CNOT':
            # For CNOT, we need control and target qubits
            raise NotImplementedError("CNOT for density matrix not implemented yet")
        else:
            raise ValueError(f"Unknown gate: {gate_name}")
        
        return self
    
    def measure_expectation(self, qubit: int, observable: str = "Z") -> float:
        """
        Calculate the expectation value of an observable.
        
        Args:
            qubit: Qubit index
            observable: Observable to measure (default: "Z")
            
        Returns:
            Expectation value
        """
        return self._density_matrix.measure_expectation(qubit, observable)
    
    def get_matrix(self) -> np.ndarray:
        """
        Get the density matrix as a numpy array.
        
        Returns:
            Density matrix array
        """
        return np.array(self._density_matrix.get_matrix())
    
    def set_matrix(self, matrix: np.ndarray) -> None:
        """
        Set the density matrix.
        
        Args:
            matrix: Density matrix array
        """
        self._density_matrix.set_matrix(matrix.tolist())
    
    def normalize(self) -> 'DensityMatrix':
        """
        Normalize the density matrix.
        
        Returns:
            self for method chaining
        """
        self._density_matrix.normalize()
        return self
    
    def get_purity(self) -> float:
        """
        Calculate the purity of the density matrix.
        
        Returns:
            Purity value
        """
        return self._density_matrix.get_purity()
    
    def get_entropy(self) -> float:
        """
        Calculate the von Neumann entropy.
        
        Returns:
            Entropy value
        """
        return self._density_matrix.get_entropy()
    
    def __str__(self) -> str:
        """String representation of the density matrix."""
        return self._density_matrix.to_string()
    
    def __repr__(self) -> str:
        """Detailed string representation of the density matrix."""
        return f"DensityMatrix({self.num_qubits} qubits)"
