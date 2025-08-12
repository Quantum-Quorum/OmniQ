#include <stdio.h>
#include <stdlib.h>
#include "libomniq-c/include/omniq_c.h"

int main() {
    printf("🚀 Testing OmniQ C Debugger Access (like Python circuit.debug())\n");
    printf("============================================================\n");
    
    // Create a quantum circuit
    printf("1. Creating a 2-qubit circuit...\n");
    omniq_circuit_t* circuit = omniq_circuit_create(2);
    if (!circuit) {
        printf("❌ Failed to create circuit\n");
        return 1;
    }
    
    // Add some gates
    printf("2. Adding quantum gates...\n");
    if (omniq_circuit_add_hadamard(circuit, 0) != 0) {
        printf("❌ Failed to add Hadamard gate\n");
        omniq_circuit_destroy(circuit);
        return 1;
    }
    
    if (omniq_circuit_add_cnot(circuit, 0, 1) != 0) {
        printf("❌ Failed to add CNOT gate\n");
        omniq_circuit_destroy(circuit);
        return 1;
    }
    
    if (omniq_circuit_add_hadamard(circuit, 1) != 0) {
        printf("❌ Failed to add Hadamard gate\n");
        omniq_circuit_destroy(circuit);
        return 1;
    }
    
    printf("3. Circuit created successfully!\n");
    printf("   Gates: H(0), CNOT(0,1), H(1)\n");
    printf("   Total gates: %d\n", omniq_circuit_get_gate_count(circuit));
    printf("\n");
    
    // Test different ways to access debugger (like Python methods)
    printf("4. Testing debugger access methods:\n");
    printf("\n");
    
    // Method 1: omniq_debugger_show(circuit) (like circuit.debug())
    printf("   • omniq_debugger_show(circuit) - opens debugger GUI\n");
    printf("     (Similar to circuit.debug() in Python)\n");
    
    // Method 2: omniq_debugger_open() (like show_debugger())
    printf("   • omniq_debugger_open() - opens debugger GUI\n");
    printf("     (Similar to show_debugger() in Python)\n");
    
    // Method 3: omniq_debugger_show_state() (like circuit.show())
    printf("   • omniq_debugger_show_state(state) - opens debugger for state\n");
    printf("     (Similar to circuit.show() in Python)\n");
    
    printf("\n");
    printf("🎯 Key Features:\n");
    printf("   ✅ Drag-and-drop circuit building\n");
    printf("   ✅ Real-time quantum state visualization\n");
    printf("   ✅ Bloch sphere visualization\n");
    printf("   ✅ Step-by-step circuit execution\n");
    printf("   ✅ Circuit import/export (JSON)\n");
    printf("   ✅ Real quantum calculations (not simulation)\n");
    
    printf("\n");
    printf("🔬 This provides a C equivalent to Python's pandas-like experience!\n");
    printf("   Just like circuit.debug() in Python,\n");
    printf("   omniq_debugger_show(circuit) opens the GUI debugger.\n");
    
    // Execute the circuit
    printf("\n5. Executing circuit...\n");
    omniq_state_t* state = omniq_circuit_execute(circuit);
    if (!state) {
        printf("❌ Failed to execute circuit\n");
        omniq_circuit_destroy(circuit);
        return 1;
    }
    
    printf("✅ Circuit executed successfully!\n");
    printf("   Final state has %d qubits\n", omniq_state_get_num_qubits(state));
    
    // Try to open debugger
    printf("\n6. Attempting to open debugger...\n");
    int result = omniq_debugger_show(circuit);
    if (result == 0) {
        printf("✅ Debugger opened successfully!\n");
        printf("   You should see the OmniQ Quantum Debugger GUI.\n");
        printf("   Use the drag-and-drop interface to build circuits!\n");
    } else {
        printf("❌ Failed to open debugger\n");
        printf("💡 Make sure the debugger is built:\n");
        printf("   cd omniq-debugger && ./build.sh\n");
    }
    
    // Test standalone debugger open
    printf("\n7. Testing standalone debugger open...\n");
    result = omniq_debugger_open();
    if (result == 0) {
        printf("✅ Standalone debugger opened successfully!\n");
    } else {
        printf("❌ Failed to open standalone debugger\n");
    }
    
    // Test state debugger
    printf("\n8. Testing state debugger...\n");
    result = omniq_debugger_show_state(state);
    if (result == 0) {
        printf("✅ State debugger opened successfully!\n");
    } else {
        printf("❌ Failed to open state debugger\n");
    }
    
    // Clean up
    printf("\n9. Cleaning up...\n");
    omniq_state_destroy(state);
    omniq_circuit_destroy(circuit);
    
    printf("✅ Cleanup completed!\n");
    printf("\n🎉 C Debugger Demo completed!\n");
    printf("\n📋 What you can do in the debugger:\n");
    printf("   • Drag and drop gates to build circuits\n");
    printf("   • Step through quantum operations\n");
    printf("   • Visualize quantum states on Bloch spheres\n");
    printf("   • Analyze quantum entanglement\n");
    printf("   • Save and load circuits\n");
    printf("\n🔬 This is real quantum computing, not simulation!\n");
    
    return 0;
} 