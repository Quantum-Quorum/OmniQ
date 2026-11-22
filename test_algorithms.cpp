#include "omniq/Circuit.h"
#include "omniq/Grovers.h"
#include "omniq/QPE.h"
#include "omniq/Statevector.h"
#include <gtest/gtest.h>

// 1. Validate Grover's algorithm on 2 qubits
TEST(AlgorithmsTest, GroverSearch2Qubit) {
    // Oracle marking |11> (index 3)
    auto oracle = [](const std::vector<int> &qubits) {
        return qubits.size() >= 2 && qubits[0] == 1 && qubits[1] == 1;
    };

    omniq::GroversAlgorithm grover(2, oracle, 1);
    EXPECT_EQ(grover.get_num_qubits(), 2);
    EXPECT_EQ(grover.get_optimal_iterations(), 1);

    // Theoretical probability of finding |11> with 1 iteration in N=4 is 1.0 (100%)
    EXPECT_NEAR(grover.get_success_probability(), 1.0, 1e-4);

    auto measurements = grover.execute_with_measurements(200);
    EXPECT_EQ(measurements.size(), 200u);

    int count3 = 0;
    for (int outcome : measurements) {
        if (outcome == 3) {
            count3++;
        }
    }
    // Expected: virtually all 200 shots measure state 3 (|11>)
    EXPECT_GT(count3, 190);
}

// 2. Validate Grover's algorithm on 3 qubits
TEST(AlgorithmsTest, GroverSearch3Qubit) {
    // Oracle marking |101> (index 5)
    auto oracle = [](const std::vector<int> &qubits) {
        return qubits.size() >= 3 && qubits[0] == 1 && qubits[1] == 0 && qubits[2] == 1;
    };

    omniq::GroversAlgorithm grover(3, oracle, 1);
    EXPECT_EQ(grover.get_num_qubits(), 3);
    EXPECT_EQ(grover.get_optimal_iterations(), 2);

    // Probability of finding solution in N=8 with 2 iterations is > 90%
    EXPECT_GT(grover.get_success_probability(), 0.90);

    auto measurements = grover.execute_with_measurements(200);
    int count5 = 0;
    for (int outcome : measurements) {
        if (outcome == 5) {
            count5++;
        }
    }
    EXPECT_GT(count5, 170);
}

// 3. Validate Quantum Phase Estimation structure
TEST(AlgorithmsTest, QPEStructure) {
    auto dummyUnitary = [](omniq::Statevector &sv, int power) {
        // Simple phase shift application
        (void)sv;
        (void)power;
    };

    omniq::QPE qpe(3, 1, dummyUnitary);
    EXPECT_EQ(qpe.get_num_precision_qubits(), 3);
    EXPECT_EQ(qpe.get_num_eigenstate_qubits(), 1);
    EXPECT_EQ(qpe.get_total_qubits(), 4);
}
