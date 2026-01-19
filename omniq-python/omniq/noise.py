"""
Noise models for OmniQ simulations.
"""

class NoiseModel:
    def __init__(self, name="Custom Noise"):
        self.name = name
        self.channels = []
    
    @staticmethod
    def createTypicalModel():
        """Create a typical noise model with T1=50us, T2=70us"""
        model = NoiseModel("Typical Hardware Noise")
        model.add_depolarizing_noise(0.001)
        model.add_relaxation_noise(50.0, 70.0)
        return model
    
    def add_depolarizing_noise(self, probability):
        self.channels.append(('DEPOLARIZING', probability))
        return self
    
    def add_relaxation_noise(self, t1, t2):
        self.channels.append(('RELAXATION', t1, t2))
        return self
    
    def to_dict(self):
        return {
            "name": self.name,
            "channels": self.channels
        }
    
    def __str__(self):
        return f"NoiseModel(name='{self.name}', channels={len(self.channels)})"
