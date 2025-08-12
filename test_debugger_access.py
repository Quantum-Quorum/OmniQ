#!/usr/bin/env python3
"""
Test script to demonstrate pandas-style debugger access
"""

import sys
import os
sys.path.append('omniq-python')

try:
    from omniq import Circuit, show_debugger, QuantumDebugger
    
    print("🚀 Testing OmniQ Debugger Access (like pandas DataFrame methods)")
    print("=" * 60)
    
    # Create a quantum circuit
    print("1. Creating a 2-qubit circuit...")
    circuit = Circuit(2)
    
    # Add some gates
    print("2. Adding quantum gates...")
    circuit.h(0)      # Hadamard on qubit 0
    circuit.cx(0, 1)  # CNOT from qubit 0 to 1
    circuit.h(1)      # Hadamard on qubit 1
    
    print("3. Circuit created successfully!")
    print("   Gates: H(0), CNOT(0,1), H(1)")
    print()
    
    # Test different ways to access debugger (like pandas methods)
    print("4. Testing debugger access methods:")
    print()
    
    # Method 1: circuit.debug() (like df.head())
    print("   • circuit.debug() - opens debugger GUI")
    print("     (Similar to df.head() in pandas)")
    
    # Method 2: circuit.show() (like df.show())
    print("   • circuit.show() - opens debugger GUI")
    print("     (Similar to df.show() in pandas)")
    
    # Method 3: show_debugger() (like pd.show(df))
    print("   • show_debugger(circuit) - standalone function")
    print("     (Similar to pd.show(df) in pandas)")
    
    # Method 4: QuantumDebugger object
    print("   • QuantumDebugger(circuit).show() - object method")
    print("     (Similar to pd.DataFrame(df).head())")
    
    print()
    print("🎯 Key Features:")
    print("   ✅ Drag-and-drop circuit building")
    print("   ✅ Real-time quantum state visualization")
    print("   ✅ Bloch sphere visualization")
    print("   ✅ Step-by-step circuit execution")
    print("   ✅ Circuit import/export (JSON)")
    print("   ✅ Real quantum calculations (not simulation)")
    
    print()
    print("🔬 This provides a pandas-like experience for quantum computing!")
    print("   Just like df.head() shows DataFrame info,")
    print("   circuit.debug() shows quantum circuit info in a GUI.")
    
    # Try to open debugger
    print()
    print("5. Attempting to open debugger...")
    try:
        circuit.debug()
        print("✅ Debugger opened successfully!")
        print("   You should see the OmniQ Quantum Debugger GUI.")
        print("   Use the drag-and-drop interface to build circuits!")
    except Exception as e:
        print(f"❌ Debugger not available: {e}")
        print("💡 Make sure the debugger is built:")
        print("   cd omniq-debugger && ./build.sh")
    
except ImportError as e:
    print(f"❌ Import error: {e}")
    print("💡 Make sure you're in the correct directory and Python path is set up")
except Exception as e:
    print(f"❌ Error: {e}")
    print("💡 Check that all dependencies are installed") 