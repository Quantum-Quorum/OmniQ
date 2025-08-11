//
// Created by Goutham Arcot on 17/07/25.
//

#include "libomniq-core/include/omniq/Circuit.h"
#include "libomniq-core/include/omniq/Grovers.h"
#include "libomniq-core/include/omniq/QPE.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <cmath>

using namespace omniq;

void test_grovers_database_search() {
    std::cout << "=== Testing Grover's Algorithm - Database Search ===" << std::endl;
    
    // Create a simple database search oracle
    int target_value = 5; // Search for value 5
    auto oracle = grover_utils::create_database_oracle(target_value);
    
    // Create Grover's algorithm with 4 qubits (16 possible values)
    GroversAlgorithm grover(4, oracle, 1);
    
    std::cout << "Number of qubits: " << grover.get_num_qubits() << std::endl;
    std::cout << "Number of solutions: " << grover.get_num_solutions() << std::endl;
    std::cout << "Optimal iterations: " << grover.get_optimal_iterations() << std::endl;
    std::cout << "Expected success probability: " << std::fixed << std::setprecision(4) 
              << grover.get_success_probability() << std::endl;
    
    // Execute with measurements
    std::vector<int> results = grover.execute_with_measurements(100);
    
    // Count results
    std::map<int, int> result_counts;
    for (int result : results) {
        result_counts[result]++;
    }
    
    std::cout << "Measurement results (top 5):" << std::endl;
    int count = 0;
    for (const auto& pair : result_counts) {
        if (count++ < 5) {
            std::cout << "  Value " << pair.first << ": " << pair.second << " times" << std::endl;
        }
    }
    
    // Check if target was found
    if (result_counts.find(target_value) != result_counts.end()) {
        std::cout << "✓ Target value " << target_value << " found " 
                  << result_counts[target_value] << " times!" << std::endl;
    } else {
        std::cout << "✗ Target value " << target_value << " not found" << std::endl;
    }
}

void test_qpe_phase_estimation() {
    std::cout << "\n=== Testing Quantum Phase Estimation ===" << std::endl;
    
    // Create a simple phase rotation unitary
    double true_phase = 0.25; // 1/4 of 2π
    auto unitary = qpe_utils::create_phase_rotation_unitary(2.0 * M_PI * true_phase);
    
    // Create QPE with 4 precision qubits and 1 eigenstate qubit
    QPE qpe(4, 1, unitary);
    
    std::cout << "True phase: " << std::fixed << std::setprecision(4) << true_phase << std::endl;
    std::cout << "Number of precision qubits: " << qpe.get_num_precision_qubits() << std::endl;
    std::cout << "Number of eigenstate qubits: " << qpe.get_num_eigenstate_qubits() << std::endl;
    std::cout << "Total qubits: " << qpe.get_total_qubits() << std::endl;
    
    // Execute with measurements
    std::vector<double> phase_estimates = qpe.execute_with_measurements(100);
    
    // Calculate statistics
    double sum = 0.0;
    for (double phase : phase_estimates) {
        sum += phase;
    }
    
    double mean = sum / phase_estimates.size();
    
    std::cout << "Phase estimation results:" << std::endl;
    std::cout << "  Mean: " << std::fixed << std::setprecision(4) << mean << std::endl;
    std::cout << "  Error: " << std::fixed << std::setprecision(4) << std::abs(mean - true_phase) << std::endl;
    
    // Check if estimation is reasonable
    double error_threshold = 0.1; // 10% error tolerance
    if (std::abs(mean - true_phase) < error_threshold) {
        std::cout << "✓ Phase estimation successful!" << std::endl;
    } else {
        std::cout << "✗ Phase estimation failed" << std::endl;
    }
}

void test_qft() {
    std::cout << "\n=== Testing Quantum Fourier Transform ===" << std::endl;
    
    // Create a simple state
    Statevector state(2);
    state[0] = std::complex<double>(1.0, 0.0); // |00⟩ state
    
    std::cout << "Initial state:" << std::endl;
    std::cout << state.to_string() << std::endl;
    
    // Apply QFT
    Circuit qft_circuit = Circuit(2);
    auto qft_gate = std::make_shared<QFTGate>(2, false);
    qft_circuit.add_gate(qft_gate);
    
    Statevector result = qft_circuit.execute_statevector(state);
    
    std::cout << "After QFT:" << std::endl;
    std::cout << result.to_string() << std::endl;
    
    // Apply inverse QFT
    Circuit inv_qft_circuit = Circuit(2);
    auto inv_qft_gate = std::make_shared<QFTGate>(2, true);
    inv_qft_circuit.add_gate(inv_qft_gate);
    
    Statevector final_result = inv_qft_circuit.execute_statevector(result);
    
    std::cout << "After inverse QFT:" << std::endl;
    std::cout << final_result.to_string() << std::endl;
    
    // Check if we got back to the original state
    double fidelity = 0.0;
    for (size_t i = 0; i < state.size(); ++i) {
        fidelity += std::abs(std::conj(state[i]) * final_result[i]);
    }
    
    std::cout << "Fidelity with original state: " << std::fixed << std::setprecision(6) 
              << std::real(fidelity) << std::endl;
    
    if (std::real(fidelity) > 0.99) {
        std::cout << "✓ QFT and inverse QFT working correctly!" << std::endl;
    } else {
        std::cout << "✗ QFT and inverse QFT not working correctly" << std::endl;
    }
}

int main() {
    std::cout << "OmniQ Quantum Algorithms Test" << std::endl;
    std::cout << "=============================" << std::endl;
    
    try {
        test_grovers_database_search();
        test_qpe_phase_estimation();
        test_qft();
        
        std::cout << "\n=== All Algorithm Tests Completed! ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 