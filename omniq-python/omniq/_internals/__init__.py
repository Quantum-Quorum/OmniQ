"""
Internal C++ bindings for OmniQ.

This module contains the low-level bindings to the C++ core.
It imports classes from the compiled _omniq_core extension.
"""

try:
    from _omniq_core import Circuit as _Circuit
    from _omniq_core import Statevector as _Statevector
    from _omniq_core import GateType
except ImportError:
    # Fallback for when the C++ extension is not explicitly installed or in path
    # Check if we are in development mode and can find the build artifact
    import sys
    import os
    
    # Try to find the built module in likely build directories
    possible_paths = [
        os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../libomniq-core/build")),
        os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../libomniq-core/build/lib")),
    ]
    
    found = False
    for path in possible_paths:
        if os.path.exists(path):
            sys.path.append(path)
            try:
                from _omniq_core import Circuit as _Circuit
                from _omniq_core import Statevector as _Statevector
                from _omniq_core import GateType
                found = True
                break
            except ImportError:
                sys.path.remove(path)
                continue
    
    if not found:
        raise ImportError("Could not import _omniq_core C++ extension. Please ensure libomniq-core is built.")

# Placeholder implementations for classes not yet bound
class _DensityMatrix:
    """Placeholder for C++ DensityMatrix class."""
    def __init__(self, num_qubits=0, matrix=None):
        self.num_qubits = num_qubits if matrix is None else len(matrix).bit_length() - 1
    # ... implementation omitted for brevity, would need real binding ...

class _GroversAlgorithm:
    """Placeholder for C++ GroversAlgorithm class."""
    pass

class _QPE:
    """Placeholder for C++ QPE class."""
    pass
