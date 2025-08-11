"""
OmniQ: Seamless Quantum Programming Across Python and C.

A modern, open-source software library designed for quantum computing research and development.
"""

__version__ = "0.1.0"
__author__ = "Goutham Arcot"
__email__ = "goutham.arcot@quantum-quorum.com"

# Import core modules
from .circuit import Circuit, Statevector, DensityMatrix
from .device import Device, SimulatorDevice, QPUDevice
from .algorithms import GroversAlgorithm, QPE
from .crypto import ShorsAlgorithm, GroversCrypto
from .qml import VariationalCircuit, QMLModel
from .optimization import QAOA, VQE

# Import utility functions
from .utils import (
    create_bell_state,
    create_ghz_state,
    measure_expectation,
    calculate_fidelity,
    random_circuit
)

# Version info
def version_info():
    """Return version information."""
    return {
        'version': __version__,
        'author': __author__,
        'email': __email__
    }

# Package level constants
DEFAULT_DEVICE = 'default.qubit'
SUPPORTED_DEVICES = ['default.qubit', 'lightning.qubit', 'ibmq.manila', 'ionq.simulator']

# Import all public API
__all__ = [
    # Core classes
    'Circuit',
    'Statevector', 
    'DensityMatrix',
    'Device',
    'SimulatorDevice',
    'QPUDevice',
    
    # Algorithms
    'GroversAlgorithm',
    'QPE',
    'ShorsAlgorithm',
    'GroversCrypto',
    
    # QML
    'VariationalCircuit',
    'QMLModel',
    
    # Optimization
    'QAOA',
    'VQE',
    
    # Utilities
    'create_bell_state',
    'create_ghz_state', 
    'measure_expectation',
    'calculate_fidelity',
    'random_circuit',
    
    # Constants
    'DEFAULT_DEVICE',
    'SUPPORTED_DEVICES',
    
    # Version
    'version_info',
    '__version__'
]
