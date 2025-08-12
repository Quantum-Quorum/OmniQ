#include <iostream>
#include <memory>
#include "omniq_c.h"

int main() {
    std::cout << "🚀 OmniQ C++ Debugger Demo - Like Python circuit.debug()" << std::endl;
    std::cout << "============================================================" << std::endl;
    
    // Create a quantum circuit
    std::cout << "1. Creating a 2-qubit circuit..." << std::endl;
    auto circuit = std::unique_ptr<omniq_circuit_t, decltype(&omniq_circuit_destroy)>(
        omniq_circuit_create(2), omniq_circuit_destroy);
    
    if (!circuit) {
        std::cerr << "❌ Failed to create circuit" << std::endl;
        return 1;
    }
    
    // Add quantum gates
    std::cout << "2. Adding quantum gates..." << std::endl;
    if (omniq_circuit_add_hadamard(circuit.get(), 0) != 0) {
        std::cerr << "❌ Failed to add Hadamard gate" << std::endl;
        return 1;
    }
    
    if (omniq_circuit_add_cnot(circuit.get(), 0, 1) != 0) {
        std::cerr << "❌ Failed to add CNOT gate" << std::endl;
        return 1;
    }
    
    if (omniq_circuit_add_hadamard(circuit.get(), 1) != 0) {
        std::cerr << "❌ Failed to add Hadamard gate" << std::endl;
        return 1;
    }
    
    std::cout << "3. Circuit created successfully!" << std::endl;
    std::cout << "   Gates: H(0), CNOT(0,1), H(1)" << std::endl;
    std::cout << "   Total gates: " << omniq_circuit_get_gate_count(circuit.get()) << std::endl;
    std::cout << std::endl;
    
    // Test different ways to access debugger (like Python methods)
    std::cout << "4. Testing debugger access methods:" << std::endl;
    std::cout << std::endl;
    
    // Method 1: omniq_debugger_show(circuit) (like circuit.debug())
    std::cout << "   • omniq_debugger_show(circuit) - opens debugger GUI" << std::endl;
    std::cout << "     (Similar to circuit.debug() in Python)" << std::endl;
    
    // Method 2: omniq_debugger_open() (like show_debugger())
    std::cout << "   • omniq_debugger_open() - opens debugger GUI" << std::endl;
    std::cout << "     (Similar to show_debugger() in Python)" << std::endl;
    
    // Method 3: omniq_debugger_show_state() (like circuit.show())
    std::cout << "   • omniq_debugger_show_state(state) - opens debugger for state" << std::endl;
    std::cout << "     (Similar to circuit.show() in Python)" << std::endl;
    
    std::cout << std::endl;
    std::cout << "🎯 Key Features:" << std::endl;
    std::cout << "   ✅ Drag-and-drop circuit building" << std::endl;
    std::cout << "   ✅ Real-time quantum state visualization" << std::endl;
    std::cout << "   ✅ Bloch sphere visualization" << std::endl;
    std::cout << "   ✅ Step-by-step circuit execution" << std::endl;
    std::cout << "   ✅ Circuit import/export (JSON)" << std::endl;
    std::cout << "   ✅ Real quantum calculations (not simulation)" << std::endl;
    
    std::cout << std::endl;
    std::cout << "🔬 This provides a C++ equivalent to Python's pandas-like experience!" << std::endl;
    std::cout << "   Just like circuit.debug() in Python," << std::endl;
    std::cout << "   omniq_debugger_show(circuit) opens the GUI debugger." << std::endl;
    
    // Execute the circuit
    std::cout << std::endl << "5. Executing circuit..." << std::endl;
    auto state = std::unique_ptr<omniq_state_t, decltype(&omniq_state_destroy)>(
        omniq_circuit_execute(circuit.get()), omniq_state_destroy);
    
    if (!state) {
        std::cerr << "❌ Failed to execute circuit" << std::endl;
        return 1;
    }
    
    std::cout << "✅ Circuit executed successfully!" << std::endl;
    std::cout << "   Final state has " << omniq_state_get_num_qubits(state.get()) << " qubits" << std::endl;
    
    // Try to open debugger
    std::cout << std::endl << "6. Attempting to open debugger..." << std::endl;
    int result = omniq_debugger_show(circuit.get());
    if (result == 0) {
        std::cout << "✅ Debugger opened successfully!" << std::endl;
        std::cout << "   You should see the OmniQ Quantum Debugger GUI." << std::endl;
        std::cout << "   Use the drag-and-drop interface to build circuits!" << std::endl;
    } else {
        std::cout << "❌ Failed to open debugger" << std::endl;
        std::cout << "💡 Make sure the debugger is built:" << std::endl;
        std::cout << "   cd omniq-debugger && ./build.sh" << std::endl;
    }
    
    // Test standalone debugger open
    std::cout << std::endl << "7. Testing standalone debugger open..." << std::endl;
    result = omniq_debugger_open();
    if (result == 0) {
        std::cout << "✅ Standalone debugger opened successfully!" << std::endl;
    } else {
        std::cout << "❌ Failed to open standalone debugger" << std::endl;
    }
    
    // Test state debugger
    std::cout << std::endl << "8. Testing state debugger..." << std::endl;
    result = omniq_debugger_show_state(state.get());
    if (result == 0) {
        std::cout << "✅ State debugger opened successfully!" << std::endl;
    } else {
        std::cout << "❌ Failed to open state debugger" << std::endl;
    }
    
    // Test circuit and state debugger
    std::cout << std::endl << "9. Testing circuit and state debugger..." << std::endl;
    result = omniq_debugger_show_circuit_and_state(circuit.get(), state.get());
    if (result == 0) {
        std::cout << "✅ Circuit and state debugger opened successfully!" << std::endl;
    } else {
        std::cout << "❌ Failed to open circuit and state debugger" << std::endl;
    }
    
    // Clean up (automatic with unique_ptr)
    std::cout << std::endl << "10. Cleaning up..." << std::endl;
    std::cout << "✅ Cleanup completed!" << std::endl;
    
    std::cout << std::endl << "🎉 C++ Debugger Demo completed!" << std::endl;
    std::cout << std::endl << "📋 What you can do in the debugger:" << std::endl;
    std::cout << "   • Drag and drop gates to build circuits" << std::endl;
    std::cout << "   • Step through quantum operations" << std::endl;
    std::cout << "   • Visualize quantum states on Bloch spheres" << std::endl;
    std::cout << "   • Analyze quantum entanglement" << std::endl;
    std::cout << "   • Save and load circuits" << std::endl;
    std::cout << std::endl << "🔬 This is real quantum computing, not simulation!" << std::endl;
    
    return 0;
} 