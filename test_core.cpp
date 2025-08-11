//
// Created by Goutham Arcot on 17/07/25.
//

#include "libomniq-core/include/omniq/Circuit.h"
#include "libomniq-core/include/omniq/QuantumStates.h"
#include <iostream>
#include <iomanip>

using namespace omniq;

void test_bell_state() {
    std::cout << "=== Testing Bell State Creation ===" << std::endl;
    
    // Create a 2-qubit circuit
    Circuit circuit(2);
    
    // Apply Hadamard to first qubit
    circuit.add_hadamard(0);
    
    // Apply CNOT with control=0, target=1
    circuit.add_cnot(0, 1);
    
    // Execute the circuit
    Statevector result = circuit.execute_statevector();
    
    std::cout << "Bell state result:" << std::endl;
    std::cout << result.to_string() << std::endl;
    
    // Expected: (|00⟩ + |11⟩)/√2
    std::cout << "Expected: (|00⟩ + |11⟩)/√2" << std::endl;
}

void test_measurement() {
    std::cout << "\n=== Testing Measurement ===" << std::endl;
    
    // Create a 1-qubit circuit
    Circuit circuit(1);
    
    // Apply Hadamard to create superposition
    circuit.add_hadamard(0);
    
    // Execute the circuit
    Statevector state = circuit.execute_statevector();
    
    std::cout << "State before measurement:" << std::endl;
    std::cout << state.to_string() << std::endl;
    
    // Measure the qubit
    int result = state.measure(0);
    std::cout << "Measurement result: " << result << std::endl;
    
    std::cout << "State after measurement:" << std::endl;
    std::cout << state.to_string() << std::endl;
}

void test_expectation() {
    std::cout << "\n=== Testing Expectation Values ===" << std::endl;
    
    // Create a 1-qubit circuit
    Circuit circuit(1);
    
    // Apply rotation to create a state
    circuit.add_rotation_x(0, M_PI / 4);
    
    // Execute the circuit
    Statevector state = circuit.execute_statevector();
    
    std::cout << "State:" << std::endl;
    std::cout << state.to_string() << std::endl;
    
    // Calculate expectation value of Z
    double expectation = state.measure_expectation(0, "Z");
    std::cout << "⟨Z⟩ = " << std::fixed << std::setprecision(6) << expectation << std::endl;
}

void test_circuit_operations() {
    std::cout << "\n=== Testing Circuit Operations ===" << std::endl;
    
    Circuit circuit1(2);
    circuit1.add_hadamard(0);
    circuit1.add_cnot(0, 1);
    
    Circuit circuit2(2);
    circuit2.add_pauli_x(0);
    circuit2.add_pauli_z(1);
    
    // Compose circuits
    Circuit composed = circuit1.compose(circuit2);
    
    std::cout << "Circuit 1:" << std::endl;
    std::cout << circuit1.to_string() << std::endl;
    
    std::cout << "Circuit 2:" << std::endl;
    std::cout << circuit2.to_string() << std::endl;
    
    std::cout << "Composed circuit:" << std::endl;
    std::cout << composed.to_string() << std::endl;
    
    std::cout << "QASM output:" << std::endl;
    std::cout << composed.to_qasm() << std::endl;
}

void test_density_matrix() {
    std::cout << "\n=== Testing Density Matrix ===" << std::endl;
    
    // Create a 1-qubit density matrix
    DensityMatrix rho(1);
    
    // Apply some operations
    rho.apply_hadamard(0);
    
    std::cout << "Density matrix:" << std::endl;
    std::cout << rho.to_string() << std::endl;
    
    // Calculate purity
    double purity = rho.get_purity();
    std::cout << "Purity: " << std::fixed << std::setprecision(6) << purity << std::endl;
    
    // Calculate entropy
    double entropy = rho.get_entropy();
    std::cout << "Entropy: " << std::fixed << std::setprecision(6) << entropy << std::endl;
}

int main() {
    std::cout << "OmniQ Core Library Test" << std::endl;
    std::cout << "=======================" << std::endl;
    
    try {
        test_bell_state();
        test_measurement();
        test_expectation();
        test_circuit_operations();
        test_density_matrix();
        
        std::cout << "\n=== All Tests Passed! ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 