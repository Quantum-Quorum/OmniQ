import subprocess
import sys
import os
from pathlib import Path

class QuantumDebugger:
    def __init__(self, circuit=None):
        self.circuit = circuit
        self.debugger_path = self._find_debugger()
    
    def _find_debugger(self):
        """Find the debugger executable"""
        # Look in common locations
        possible_paths = [
            Path(__file__).parent.parent.parent / "omniq-debugger" / "build" / "omniq-debugger",
            Path(__file__).parent.parent.parent / "omniq-debugger" / "omniq-debugger",
            Path.home() / ".local" / "bin" / "omniq-debugger",
            "/usr/local/bin/omniq-debugger"
        ]
        
        for path in possible_paths:
            if path.exists() and os.access(path, os.X_OK):
                return str(path)
        
        raise FileNotFoundError("OmniQ debugger not found. Please build it first.")
    
    def show(self, circuit=None):
        """Show the quantum debugger GUI (like df.head())"""
        if circuit is not None:
            self.circuit = circuit
        
        try:
            subprocess.Popen([self.debugger_path], 
                           stdout=subprocess.DEVNULL, 
                           stderr=subprocess.DEVNULL)
            print("🚀 OmniQ Quantum Debugger opened!")
            print("   • Use the GUI to inspect quantum states")
            print("   • Drag and drop gates to build circuits")
            print("   • Step through quantum operations")
        except Exception as e:
            print(f"❌ Failed to open debugger: {e}")
            print("💡 Try building the debugger first: cd omniq-debugger && ./build.sh")
    
    def debug(self, circuit=None):
        """Alias for show() - debug the circuit"""
        return self.show(circuit)
    
    def inspect(self, circuit=None):
        """Another alias for show()"""
        return self.show(circuit)

# Convenience function
def show_debugger(circuit=None):
    """Quick function to show debugger (like df.head())"""
    debugger = QuantumDebugger(circuit)
    return debugger.show()

# Add debugger methods to Circuit class
def add_debugger_to_circuit():
    """Add debugger methods to Circuit class"""
    try:
        from .circuit import Circuit
        
        def debug(self):
            """Show debugger for this circuit"""
            debugger = QuantumDebugger(self)
            return debugger.show()
        
        def show(self):
            """Show debugger for this circuit"""
            return self.debug()
        
        Circuit.debug = debug
        Circuit.show = show
        
    except ImportError:
        pass  # Circuit class might not be available yet 