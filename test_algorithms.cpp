//
// Created by Goutham Arcot on 17/07/25.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "libomniq-core/include/omniq/Circuit.h"
#include "libomniq-core/src/modules/algorithms/Grovers.h"
#include "libomniq-core/src/modules/algorithms/QPE.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <cmath>

using namespace omniq;
using ::testing::DoubleNear;
using ::testing::ElementsAre;

TEST(GroversAlgorithmTest, DatabaseSearch) {
    // Create a simple database search oracle
    int target_value = 5; // Search for value 5
    auto oracle = grover_utils::create_database_oracle(target_value);
    
    // Create Grover's algorithm with 4 qubits (16 possible values)
    GroversAlgorithm grover(4, oracle, 1);
    
    EXPECT_EQ(grover.get_num_qubits(), 4);
    EXPECT_EQ(grover.get_num_solutions(), 1);
    EXPECT_GT(grover.get_optimal_iterations(), 0);
    EXPECT_GT(grover.get_success_probability(), 0.0);
    EXPECT_LE(grover.get_success_probability(), 1.0);
    
    // Execute with measurements
    std::vector<int> results = grover.execute_with_measurements(100);
    
    // Count results
    std::map<int, int> result_counts;
    for (int result : results) {
        result_counts[result]++;
    }
    
    // Check if target was found (should be found with high probability)
    EXPECT_GT(result_counts[target_value], 0);
}

TEST(GroversAlgorithmTest, SATProblem) {
    // Create a simple SAT problem: (x1 OR x2) AND (NOT x1 OR x2)
    std::vector<std::vector<int>> clauses = {
        {1, 2},    // x1 OR x2
        {-1, 2}    // NOT x1 OR x2
    };
    int num_variables = 2;
    
    auto oracle = grover_utils::create_sat_oracle(clauses, num_variables);
    
    // Create Grover's algorithm
    GroversAlgorithm grover(num_variables, oracle, 1);
    
    EXPECT_EQ(grover.get_num_qubits(), num_variables);
    EXPECT_GT(grover.get_optimal_iterations(), 0);
    
    // Execute with measurements
    std::vector<int> results = grover.execute_with_measurements(50);
    
    // Count results
    std::map<int, int> result_counts;
    for (int result : results) {
        result_counts[result]++;
    }
    
    // Should find at least one solution
    EXPECT_GT(result_counts.size(), 0);
}

TEST(QPETest, PhaseEstimation) {
    // Create a simple phase rotation unitary
    double true_phase = 0.25; // 1/4 of 2π
    auto unitary = qpe_utils::create_phase_rotation_unitary(2.0 * M_PI * true_phase);
    
    // Create QPE with 4 precision qubits and 1 eigenstate qubit
    QPE qpe(4, 1, unitary);
    
    EXPECT_EQ(qpe.get_num_precision_qubits(), 4);
    EXPECT_EQ(qpe.get_num_eigenstate_qubits(), 1);
    EXPECT_EQ(qpe.get_total_qubits(), 5);
    
    // Execute with measurements
    std::vector<double> phase_estimates = qpe.execute_with_measurements(100);
    
    // Calculate statistics
    double sum = 0.0;
    for (double phase : phase_estimates) {
        sum += phase;
    }
    
    double mean = sum / phase_estimates.size();
    
    // Check if estimation is reasonable (within 10% of true phase)
    double error_threshold = 0.1;
    EXPECT_LT(std::abs(mean - true_phase), error_threshold);
}

TEST(QFTTest, QFTAndInverse) {
    // Create a simple state
    Statevector state(2);
    state[0] = std::complex<double>(1.0, 0.0); // |00⟩ state
    
    // Apply QFT
    Circuit qft_circuit = Circuit(2);
    auto qft_gate = std::make_shared<QFTGate>(2, false);
    qft_circuit.add_gate(qft_gate);
    
    Statevector result = qft_circuit.execute_statevector(state);
    
    // Apply inverse QFT
    Circuit inv_qft_circuit = Circuit(2);
    auto inv_qft_gate = std::make_shared<QFTGate>(2, true);
    inv_qft_circuit.add_gate(inv_qft_gate);
    
    Statevector final_result = inv_qft_circuit.execute_statevector(result);
    
    // Check if we got back to the original state
    double fidelity = 0.0;
    for (size_t i = 0; i < state.size(); ++i) {
        fidelity += std::abs(std::conj(state[i]) * final_result[i]);
    }
    
    // Should have high fidelity (> 0.99)
    EXPECT_GT(std::real(fidelity), 0.99);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 