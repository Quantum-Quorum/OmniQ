"""
Basic Circuit class for OmniQ debugger demo
"""

class Circuit:
    def __init__(self, num_qubits):
        self.num_qubits = num_qubits
        self.gates = []
    
    def h(self, qubit):
        """Add Hadamard gate"""
        self.gates.append(('H', qubit))
        return self
    
    def x(self, qubit):
        """Add X gate"""
        self.gates.append(('X', qubit))
        return self
    
    def y(self, qubit):
        """Add Y gate"""
        self.gates.append(('Y', qubit))
        return self
    
    def z(self, qubit):
        """Add Z gate"""
        self.gates.append(('Z', qubit))
        return self
    
    def cx(self, control, target):
        """Add CNOT gate"""
        self.gates.append(('CNOT', control, target))
        return self
    
    def swap(self, qubit1, qubit2):
        """Add SWAP gate"""
        self.gates.append(('SWAP', qubit1, qubit2))
        return self
    
    def rx(self, qubit, angle):
        """Add RX gate"""
        self.gates.append(('RX', qubit, angle))
        return self
    
    def ry(self, qubit, angle):
        """Add RY gate"""
        self.gates.append(('RY', qubit, angle))
        return self
    
    def rz(self, qubit, angle):
        """Add RZ gate"""
        self.gates.append(('RZ', qubit, angle))
        return self
    
    def phase(self, qubit, angle):
        """Add Phase gate"""
        self.gates.append(('PHASE', qubit, angle))
        return self
    
    def debug(self):
        """Open debugger (like df.head())"""
        from .debugger import show_debugger
        return show_debugger(self)
    
    def show(self):
        """Open debugger (like df.show())"""
        return self.debug()
    
    def __str__(self):
        return f"Circuit({self.num_qubits} qubits, {len(self.gates)} gates)"
    
    def __repr__(self):
        return self.__str__()
