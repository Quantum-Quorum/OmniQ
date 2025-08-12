#!/usr/bin/env python3
"""
OmniQ Debugger Demo - Like pandas DataFrame methods

This demonstrates how to use the OmniQ debugger similar to:
- df.head() - shows DataFrame
- df.info() - shows DataFrame info
- df.describe() - shows DataFrame statistics

Here we have:
- circuit.debug() - opens debugger GUI
- circuit.show() - opens debugger GUI  
- show_debugger() - standalone function
"""

import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))

from omniq import Circuit, show_debugger, QuantumDebugger

def main():
    print("🚀 OmniQ Debugger Demo - Like pandas DataFrame methods")
    print("=" * 60)
    
    # Create a quantum circuit (like creating a DataFrame)
    print("1. Creating a quantum circuit...")
    circuit = Circuit(2)  # 2 qubits
    
    # Add some gates (like adding data to DataFrame)
    print("2. Adding quantum gates...")
    circuit.h(0)      # Hadamard on qubit 0
    circuit.cx(0, 1)  # CNOT from qubit 0 to 1
    circuit.h(1)      # Hadamard on qubit 1
    
    print("3. Circuit created with gates: H(0), CNOT(0,1), H(1)")
    print()
    
    # Method 1: Like df.head() - show debugger for this circuit
    print("4. Opening debugger with circuit.debug() (like df.head())...")
    try:
        circuit.debug()
        print("✅ Debugger opened successfully!")
    except Exception as e:
        print(f"❌ Failed to open debugger: {e}")
        print("💡 Make sure the debugger is built: cd omniq-debugger && ./build.sh")
    
    print()
    
    # Method 2: Like df.show() - another way to show debugger
    print("5. Opening debugger with circuit.show() (like df.show())...")
    try:
        circuit.show()
        print("✅ Debugger opened successfully!")
    except Exception as e:
        print(f"❌ Failed to open debugger: {e}")
    
    print()
    
    # Method 3: Standalone function - like pd.show(df)
    print("6. Opening debugger with show_debugger() (like pd.show(df))...")
    try:
        show_debugger(circuit)
        print("✅ Debugger opened successfully!")
    except Exception as e:
        print(f"❌ Failed to open debugger: {e}")
    
    print()
    
    # Method 4: Create debugger object and use it
    print("7. Creating debugger object and using it...")
    try:
        debugger = QuantumDebugger(circuit)
        debugger.show()
        print("✅ Debugger opened successfully!")
    except Exception as e:
        print(f"❌ Failed to open debugger: {e}")
    
    print()
    print("🎉 Demo completed!")
    print()
    print("📋 What you can do in the debugger:")
    print("   • Drag and drop gates to build circuits")
    print("   • Step through quantum operations")
    print("   • Visualize quantum states on Bloch spheres")
    print("   • Analyze quantum entanglement")
    print("   • Save and load circuits")
    print()
    print("🔬 This is real quantum computing, not simulation!")

if __name__ == "__main__":
    main() 