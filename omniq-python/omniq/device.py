"""
Device module for OmniQ.

Provides device abstractions for quantum simulators and hardware.
"""

from typing import Optional, Dict, Any, Union
from .qos import QPUClient, QPU_ALLOC_FLAG_ISOLATED

class Device:
    """Base class for quantum devices."""
    
    def __init__(self, name: str, **kwargs):
        self.name = name
        self.config = kwargs
    
    def execute(self, circuit, **kwargs):
        """Execute a circuit on this device."""
        raise NotImplementedError
    
    def __str__(self):
        return f"Device({self.name})"

class SimulatorDevice(Device):
    """Quantum simulator device."""
    
    def __init__(self, name: str = "default.qubit", **kwargs):
        super().__init__(name, **kwargs)
    
    def execute(self, circuit, shots: int = 1024, **kwargs) -> Dict[str, int]:
        """Execute circuit on simulator via QPUClient direct mode."""
        client = QPUClient(":force-direct:")
        try:
            qasm_str = circuit.to_qasm() if hasattr(circuit, 'to_qasm') else str(circuit)
            num_qubits = getattr(circuit, 'num_qubits', 2)
            return client.run_sync(qasm_str, num_qubits=num_qubits, shots=shots)
        finally:
            client.close()

class QPUDevice(Device):
    """Native Quantum Processing Unit accelerator device on OmniQ-OS."""
    
    def __init__(self, name: str = "omniq.qpu0", dev_path: Optional[str] = None, **kwargs):
        super().__init__(name, **kwargs)
        self.dev_path = dev_path
        self._client: Optional[QPUClient] = None

    def connect(self) -> QPUClient:
        """Connect to QPU device or daemon."""
        if self._client is None:
            self._client = QPUClient(self.dev_path)
        return self._client

    def close(self):
        """Close QPU connection."""
        if self._client:
            self._client.close()
            self._client = None

    def __enter__(self):
        self.connect()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def execute(self, circuit: Any, shots: int = 1024, flags: int = 0, **kwargs) -> Dict[str, int]:
        """
        Execute an OpenQASM circuit on the QPU accelerator.
        Allocates a dedicated spatial partition with QMMU guard bands,
        submits the circuit job, reads measurement outcomes, and frees the partition.
        """
        client = self.connect()
        qasm_str = circuit.to_qasm() if hasattr(circuit, 'to_qasm') else str(circuit)
        num_qubits = getattr(circuit, 'num_qubits', 2)

        pid = client.allocate(num_qubits, flags=QPU_ALLOC_FLAG_ISOLATED)
        try:
            job_id = client.submit(pid, qasm_str, shots=shots, flags=flags)
            return client.wait(job_id, timeout_ms=2000, num_qubits=num_qubits)
        finally:
            client.free(pid)

    def get_topology(self) -> Dict[str, Any]:
        """Query QPU chip topology and coupler graph."""
        return self.connect().get_topology()

    def get_calibration(self) -> list:
        """Query physical qubit calibration metrics."""
        return self.connect().get_calibration()

    def get_cryo(self) -> Dict[str, Any]:
        """Query dilution refrigerator thermal stage telemetry."""
        return self.connect().get_cryo()

    def reset_qubit(self, physical_id: int, active: bool = True) -> bool:
        """Dispatch active microwave pulse reset to physical qubit."""
        return self.connect().reset_qubit(physical_id, active=active)
