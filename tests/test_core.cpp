//
// Created by Goutham Arcot on 17/07/25.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "libomniq-core/include/omniq/Circuit.h"
#include "libomniq-core/include/omniq/QuantumStates.h"
#include <cmath>
#include <random>

using namespace omniq;
using ::testing::DoubleNear;
using ::testing::ElementsAre;

class StatevectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up any common test data
    }
    
    void TearDown() override {
        // Clean up any test data
    }
};

class CircuitTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Set up any common test data
    }
    
    void TearDown() override {
        // Clean up any test data
    }
};

// Statevector Tests
TEST_F(StatevectorTest, ConstructorWithValidQubits) {
    Statevector state(2);
    EXPECT_EQ(state.get_num_qubits(), 2);
    EXPECT_EQ(state.size(), 4);
    EXPECT_TRUE(state.is_normalized());
}

TEST_F(StatevectorTest, ConstructorWithZeroQubits) {
    Statevector state(0);
    EXPECT_EQ(state.get_num_qubits(), 0);
    EXPECT_EQ(state.size(), 1);
}

TEST_F(StatevectorTest, ConstructorWithNegativeQubitsThrows) {
    EXPECT_THROW(Statevector state(-1), std::invalid_argument);
}

TEST_F(StatevectorTest, ConstructorWithAmplitudes) {
    std::vector<std::complex<double>> amplitudes = {
        {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}
    };
    Statevector state(amplitudes);
    EXPECT_EQ(state.get_num_qubits(), 2);
    EXPECT_EQ(state.size(), 4);
}

TEST_F(StatevectorTest, ConstructorWithInvalidAmplitudeSizeThrows) {
    std::vector<std::complex<double>> amplitudes = {1.0, 0.0, 0.0}; // Not power of 2
    EXPECT_THROW(Statevector state(amplitudes), std::invalid_argument);
}

TEST_F(StatevectorTest, HadamardGate) {
    Statevector state(1);
    state.apply_hadamard(0);
    
    // Check that |0⟩ becomes (|0⟩ + |1⟩)/√2
    auto amplitudes = state.get_amplitudes();
    EXPECT_THAT(std::abs(amplitudes[0]), DoubleNear(1.0/std::sqrt(2.0), 1e-10));
    EXPECT_THAT(std::abs(amplitudes[1]), DoubleNear(1.0/std::sqrt(2.0), 1e-10));
}

TEST_F(StatevectorTest, PauliXGate) {
    Statevector state(1);
    state.apply_pauli_x(0);
    
    // Check that |0⟩ becomes |1⟩
    auto amplitudes = state.get_amplitudes();
    EXPECT_THAT(std::abs(amplitudes[0]), DoubleNear(0.0, 1e-10));
    EXPECT_THAT(std::abs(amplitudes[1]), DoubleNear(1.0, 1e-10));
}

TEST_F(StatevectorTest, CNOTGate) {
    Statevector state(2);
    state.apply_hadamard(0);  // Create superposition on control qubit
    state.apply_cnot(0, 1);   // Apply CNOT
    
    // Check Bell state creation
    auto amplitudes = state.get_amplitudes();
    EXPECT_THAT(std::abs(amplitudes[0]), DoubleNear(1.0/std::sqrt(2.0), 1e-10));
    EXPECT_THAT(std::abs(amplitudes[1]), DoubleNear(0.0, 1e-10));
    EXPECT_THAT(std::abs(amplitudes[2]), DoubleNear(0.0, 1e-10));
    EXPECT_THAT(std::abs(amplitudes[3]), DoubleNear(1.0/std::sqrt(2.0), 1e-10));
}

TEST_F(StatevectorTest, Measurement) {
    Statevector state(1);
    state.apply_hadamard(0);
    
    // Measure multiple times and check statistics
    std::vector<int> results;
    for (int i = 0; i < 1000; ++i) {
        Statevector copy = state;
        results.push_back(copy.measure(0));
    }
    
    int zeros = std::count(results.begin(), results.end(), 0);
    int ones = std::count(results.begin(), results.end(), 1);
    
    // Should be roughly 50-50 (within 5% tolerance)
    EXPECT_NEAR(static_cast<double>(zeros) / results.size(), 0.5, 0.05);
    EXPECT_NEAR(static_cast<double>(ones) / results.size(), 0.5, 0.05);
}

TEST_F(StatevectorTest, ExpectationValue) {
    Statevector state(1);
    state.apply_hadamard(0);
    
    double expectation = state.measure_expectation(0, "Z");
    EXPECT_THAT(expectation, DoubleNear(0.0, 1e-10));
}

TEST_F(StatevectorTest, Normalization) {
    Statevector state(1);
    state[0] = std::complex<double>(2.0, 0.0);
    state[1] = std::complex<double>(2.0, 0.0);
    
    EXPECT_FALSE(state.is_normalized());
    state.normalize();
    EXPECT_TRUE(state.is_normalized());
    EXPECT_THAT(state.get_norm(), DoubleNear(1.0, 1e-10));
}

TEST_F(StatevectorTest, TensorProduct) {
    Statevector state1(1);
    state1.apply_hadamard(0);
    
    Statevector state2(1);
    state2.apply_pauli_x(0);
    
    Statevector result = state1.tensor_product(state2);
    EXPECT_EQ(result.get_num_qubits(), 2);
    EXPECT_EQ(result.size(), 4);
}

// Circuit Tests
TEST_F(CircuitTest, ConstructorWithValidQubits) {
    Circuit circuit(3, 2);
    EXPECT_EQ(circuit.get_num_qubits(), 3);
    EXPECT_EQ(circuit.get_num_classical_bits(), 2);
    EXPECT_EQ(circuit.get_gate_count(), 0);
}

TEST_F(CircuitTest, ConstructorWithNegativeQubitsThrows) {
    EXPECT_THROW(Circuit circuit(-1), std::invalid_argument);
}

TEST_F(CircuitTest, AddGates) {
    Circuit circuit(2);
    
    circuit.add_hadamard(0);
    circuit.add_cnot(0, 1);
    circuit.add_pauli_x(1);
    
    EXPECT_EQ(circuit.get_gate_count(), 3);
}

TEST_F(CircuitTest, AddGateWithInvalidQubitThrows) {
    Circuit circuit(2);
    EXPECT_THROW(circuit.add_hadamard(5), std::invalid_argument);
}

TEST_F(CircuitTest, ExecuteCircuit) {
    Circuit circuit(2);
    circuit.add_hadamard(0);
    circuit.add_cnot(0, 1);
    
    Statevector result = circuit.execute_statevector();
    EXPECT_EQ(result.get_num_qubits(), 2);
    
    // Check Bell state
    auto amplitudes = result.get_amplitudes();
    EXPECT_THAT(std::abs(amplitudes[0]), DoubleNear(1.0/std::sqrt(2.0), 1e-10));
    EXPECT_THAT(std::abs(amplitudes[3]), DoubleNear(1.0/std::sqrt(2.0), 1e-10));
}

TEST_F(CircuitTest, CircuitOptimization) {
    Circuit circuit(1);
    
    // Add redundant gates that should cancel
    circuit.add_hadamard(0);
    circuit.add_hadamard(0);
    circuit.add_pauli_x(0);
    circuit.add_pauli_x(0);
    
    EXPECT_EQ(circuit.get_gate_count(), 4);
    circuit.optimize();
    EXPECT_EQ(circuit.get_gate_count(), 0); // All gates should cancel
}

TEST_F(CircuitTest, CircuitComposition) {
    Circuit circuit1(2);
    circuit1.add_hadamard(0);
    
    Circuit circuit2(2);
    circuit2.add_cnot(0, 1);
    
    Circuit composed = circuit1.compose(circuit2);
    EXPECT_EQ(composed.get_gate_count(), 2);
}

TEST_F(CircuitTest, QASMExport) {
    Circuit circuit(2);
    circuit.add_hadamard(0);
    circuit.add_cnot(0, 1);
    circuit.add_measurement(0);
    
    std::string qasm = circuit.to_qasm();
    EXPECT_TRUE(qasm.find("OPENQASM 2.0") != std::string::npos);
    EXPECT_TRUE(qasm.find("h q[0]") != std::string::npos);
    EXPECT_TRUE(qasm.find("cx q[0],q[1]") != std::string::npos);
    EXPECT_TRUE(qasm.find("measure q[0]") != std::string::npos);
}

TEST_F(CircuitTest, CircuitValidation) {
    Circuit circuit(2);
    EXPECT_TRUE(circuit.is_valid());
    
    // Add invalid gate
    circuit.add_hadamard(5); // Invalid qubit
    EXPECT_FALSE(circuit.is_valid());
    
    auto errors = circuit.get_validation_errors();
    EXPECT_FALSE(errors.empty());
}

// Edge Cases and Stress Tests
TEST_F(StatevectorTest, LargeStatevector) {
    // Test with 10 qubits (1024 amplitudes)
    Statevector state(10);
    EXPECT_EQ(state.get_num_qubits(), 10);
    EXPECT_EQ(state.size(), 1024);
    
    // Apply some gates
    state.apply_hadamard(0);
    state.apply_cnot(0, 1);
    
    EXPECT_TRUE(state.is_normalized());
}

TEST_F(StatevectorTest, ComplexAmplitudes) {
    Statevector state(1);
    state[0] = std::complex<double>(1.0, 1.0);
    state[1] = std::complex<double>(0.0, 1.0);
    
    state.normalize();
    EXPECT_TRUE(state.is_normalized());
    
    double norm = state.get_norm();
    EXPECT_THAT(norm, DoubleNear(1.0, 1e-10));
}

TEST_F(CircuitTest, DeepCircuit) {
    Circuit circuit(1);
    
    // Add many gates
    for (int i = 0; i < 100; ++i) {
        circuit.add_hadamard(0);
    }
    
    EXPECT_EQ(circuit.get_gate_count(), 100);
    circuit.optimize();
    EXPECT_EQ(circuit.get_gate_count(), 0); // All should cancel
}

// Performance Tests
TEST_F(StatevectorTest, PerformanceTest) {
    const int num_qubits = 12; // 4096 amplitudes
    Statevector state(num_qubits);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Apply many gates
    for (int i = 0; i < 1000; ++i) {
        state.apply_hadamard(i % num_qubits);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete in reasonable time (< 1 second)
    EXPECT_LT(duration.count(), 1000);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 