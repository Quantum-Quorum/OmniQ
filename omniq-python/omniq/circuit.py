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

    def cp(self, angle, control, target):
        """Add Controlled-Phase gate"""
        self.gates.append(('CP', control, target, angle))
        return self

    def to_dict(self):
        """Convert circuit to dictionary for serialization"""
        gate_list = []
        for i, g in enumerate(self.gates):
            g_type = g[0]
            gate_data = {
                "type": g_type,
                "qubit": g[1],
                "step": i 
            }
            
            if g_type in ('CNOT', 'SWAP'):
                gate_data["target"] = g[2]
            elif g_type == 'CP':
                gate_data["target"] = g[2]
                gate_data["parameter"] = g[3]
            elif g_type in ('RX', 'RY', 'RZ', 'PHASE'):
                gate_data["parameter"] = g[2]
                
            gate_list.append(gate_data)
        
        return {
            "num_qubits": self.num_qubits,
            "gates": gate_list
        }

    def save_json(self, path):
        """Save circuit as JSON"""
        import json
        with open(path, 'w') as f:
            json.dump(self.to_dict(), f, indent=2)

    def debug(self, noise_model=None, view_mode=None):
        """Open debugger (like df.head())"""
        from .debugger import show_debugger
        return show_debugger(self, noise_model=noise_model, view_mode=view_mode)
    
    def show(self, view_mode=None):
        """Open debugger (like df.show())"""
        return self.debug(view_mode=view_mode)
    
    def __str__(self):
        return f"Circuit({self.num_qubits} qubits, {len(self.gates)} gates)"
    
    def __repr__(self):
        return self.__str__()
