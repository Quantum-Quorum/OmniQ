//
// Created by Goutham Arcot on 17/07/25.
//

#include "omniq/Grovers.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <stdexcept>

namespace omniq {

// GroversAlgorithm implementation
GroversAlgorithm::GroversAlgorithm(int num_qubits, OracleFunction oracle, int num_solutions)
    : num_qubits_(num_qubits), num_solutions_(num_solutions), oracle_(oracle) {
    if (num_qubits <= 0) {
        throw std::invalid_argument("Number of qubits must be positive");
    }
    if (num_solutions <= 0) {
        throw std::invalid_argument("Number of solutions must be positive");
    }
    if (!oracle) {
        throw std::invalid_argument("Oracle function cannot be null");
    }
    
    num_iterations_ = get_optimal_iterations();
}

void GroversAlgorithm::set_iterations(int iterations) {
    if (iterations < 0) {
        throw std::invalid_argument("Number of iterations must be non-negative");
    }
    num_iterations_ = iterations;
}

int GroversAlgorithm::get_optimal_iterations() const {
    return grover_utils::calculate_optimal_iterations(num_qubits_, num_solutions_);
}

Circuit GroversAlgorithm::build_circuit() const {
    Circuit circuit(num_qubits_);
    
    // Initialize all qubits to |0⟩ and apply Hadamard to create uniform superposition
    for (int i = 0; i < num_qubits_; ++i) {
        circuit.add_hadamard(i);
    }
    
    // Apply Grover iterations
    for (int iteration = 0; iteration < num_iterations_; ++iteration) {
        // Apply oracle
        auto oracle_gate = std::make_shared<OracleGate>(num_qubits_, oracle_);
        circuit.add_gate(oracle_gate);
        
        // Apply diffusion operator
        auto diffusion_gate = std::make_shared<DiffusionGate>(num_qubits_);
        circuit.add_gate(diffusion_gate);
    }
    
    return circuit;
}

Circuit GroversAlgorithm::build_oracle() const {
    Circuit circuit(num_qubits_);
    auto oracle_gate = std::make_shared<OracleGate>(num_qubits_, oracle_);
    circuit.add_gate(oracle_gate);
    return circuit;
}

Circuit GroversAlgorithm::build_diffusion() const {
    Circuit circuit(num_qubits_);
    auto diffusion_gate = std::make_shared<DiffusionGate>(num_qubits_);
    circuit.add_gate(diffusion_gate);
    return circuit;
}

Statevector GroversAlgorithm::execute(const Statevector& initial_state) const {
    Circuit circuit = build_circuit();
    return circuit.execute_statevector(initial_state);
}

std::vector<int> GroversAlgorithm::execute_with_measurements(int num_shots) const {
    if (num_shots <= 0) {
        throw std::invalid_argument("Number of shots must be positive");
    }
    
    std::vector<int> results;
    results.reserve(num_shots);
    
    for (int shot = 0; shot < num_shots; ++shot) {
        Statevector state = execute();
        
        // Measure all qubits
        int result = 0;
        for (int i = 0; i < num_qubits_; ++i) {
            int bit = state.measure(i);
            result |= (bit << i);
        }
        results.push_back(result);
    }
    
    return results;
}

double GroversAlgorithm::get_success_probability() const {
    return grover_utils::estimate_success_probability(num_qubits_, num_solutions_, num_iterations_);
}

// OracleGate implementation
OracleGate::OracleGate(int num_qubits, OracleFunction oracle)
    : Gate(GateType::CUSTOM, {}, {}, "Oracle"), num_qubits_(num_qubits), oracle_(oracle) {
    if (!oracle) {
        throw std::invalid_argument("Oracle function cannot be null");
    }
}

void OracleGate::apply(Statevector& state) const {
    apply_oracle_to_statevector(state);
}

void OracleGate::apply(DensityMatrix& state) const {
    // For density matrix, we need to apply the oracle to both sides
    // This is a simplified implementation
    // In practice, you'd need to properly handle the density matrix transformation
}

std::string OracleGate::to_string() const {
    return "Oracle(" + std::to_string(num_qubits_) + " qubits)";
}

void OracleGate::apply_oracle_to_statevector(Statevector& state) const {
    if (state.get_num_qubits() != num_qubits_) {
        throw std::invalid_argument("State qubit count mismatch");
    }
    
    // Apply phase kickback: flip the phase of solution states
    for (size_t i = 0; i < state.size(); ++i) {
        // Convert index to binary representation
        std::vector<int> bit_string;
        for (int j = 0; j < num_qubits_; ++j) {
            bit_string.push_back((i >> j) & 1);
        }
        
        // Check if this state is a solution
        if (oracle_(bit_string)) {
            // Flip the phase
            state[i] = -state[i];
        }
    }
}

// DiffusionGate implementation
DiffusionGate::DiffusionGate(int num_qubits)
    : Gate(GateType::CUSTOM, {}, {}, "Diffusion"), num_qubits_(num_qubits) {
}

void DiffusionGate::apply(Statevector& state) const {
    apply_diffusion_to_statevector(state);
}

void DiffusionGate::apply(DensityMatrix& state) const {
    // For density matrix, we need to apply the diffusion to both sides
    // This is a simplified implementation
}

std::string DiffusionGate::to_string() const {
    return "Diffusion(" + std::to_string(num_qubits_) + " qubits)";
}

void DiffusionGate::apply_diffusion_to_statevector(Statevector& state) const {
    if (state.get_num_qubits() != num_qubits_) {
        throw std::invalid_argument("State qubit count mismatch");
    }
    
    // Apply Hadamard gates to all qubits
    for (int i = 0; i < num_qubits_; ++i) {
        state.apply_hadamard(i);
    }
    
    // Apply X gates to all qubits
    for (int i = 0; i < num_qubits_; ++i) {
        state.apply_pauli_x(i);
    }
    
    // Apply multi-controlled Z gate (controlled on all qubits being 1)
    // This is equivalent to flipping the phase of the |0...0⟩ state
    for (size_t i = 0; i < state.size(); ++i) {
        if (i == 0) { // |0...0⟩ state
            state[i] = -state[i];
        }
    }
    
    // Apply X gates to all qubits again
    for (int i = 0; i < num_qubits_; ++i) {
        state.apply_pauli_x(i);
    }
    
    // Apply Hadamard gates to all qubits again
    for (int i = 0; i < num_qubits_; ++i) {
        state.apply_hadamard(i);
    }
}

// Utility functions implementation
namespace grover_utils {

OracleFunction create_database_oracle(int target_value) {
    return [target_value](const std::vector<int>& input) -> bool {
        // Convert binary input to integer
        int value = 0;
        for (size_t i = 0; i < input.size(); ++i) {
            value |= (input[i] << i);
        }
        return value == target_value;
    };
}

OracleFunction create_sat_oracle(const std::vector<std::vector<int>>& clauses, int num_variables) {
    return [clauses, num_variables](const std::vector<int>& input) -> bool {
        // Check if the assignment satisfies all clauses
        for (const auto& clause : clauses) {
            bool clause_satisfied = false;
            for (int literal : clause) {
                int var = std::abs(literal) - 1; // Variables are 1-indexed in DIMACS format
                if (var >= num_variables) continue;
                
                bool var_value = (var < input.size()) ? input[var] : false;
                bool literal_value = (literal > 0) ? var_value : !var_value;
                
                if (literal_value) {
                    clause_satisfied = true;
                    break;
                }
            }
            if (!clause_satisfied) {
                return false; // This clause is not satisfied
            }
        }
        return true; // All clauses are satisfied
    };
}

OracleFunction create_graph_coloring_oracle(const std::vector<std::pair<int, int>>& edges, 
                                          int num_vertices, int num_colors) {
    return [edges, num_vertices, num_colors](const std::vector<int>& input) -> bool {
        // Check if the coloring is valid (no adjacent vertices have the same color)
        for (const auto& edge : edges) {
            int v1 = edge.first;
            int v2 = edge.second;
            
            if (v1 >= num_vertices || v2 >= num_vertices) continue;
            
            // Calculate color indices in the input
            int color1_idx = v1 * num_colors;
            int color2_idx = v2 * num_colors;
            
            // Find the assigned colors
            int color1 = -1, color2 = -1;
            for (int c = 0; c < num_colors; ++c) {
                if (color1_idx + c < input.size() && input[color1_idx + c]) {
                    color1 = c;
                }
                if (color2_idx + c < input.size() && input[color2_idx + c]) {
                    color2 = c;
                }
            }
            
            // Check if adjacent vertices have the same color
            if (color1 == color2 && color1 != -1) {
                return false; // Invalid coloring
            }
        }
        return true; // Valid coloring
    };
}

int calculate_optimal_iterations(int num_qubits, int num_solutions) {
    if (num_qubits <= 0 || num_solutions <= 0) {
        return 0;
    }
    
    int N = 1 << num_qubits; // Total number of states
    double theta = std::asin(std::sqrt(static_cast<double>(num_solutions) / N));
    
    // Optimal number of iterations is approximately π/(4θ)
    int optimal_iterations = static_cast<int>(std::round(M_PI / (4.0 * theta)));
    
    return std::max(1, optimal_iterations);
}

double estimate_success_probability(int num_qubits, int num_solutions, int num_iterations) {
    if (num_qubits <= 0 || num_solutions <= 0 || num_iterations < 0) {
        return 0.0;
    }
    
    int N = 1 << num_qubits; // Total number of states
    double theta = std::asin(std::sqrt(static_cast<double>(num_solutions) / N));
    
    // Success probability after k iterations
    double success_prob = std::pow(std::sin((2.0 * num_iterations + 1.0) * theta), 2);
    
    return success_prob;
}

} // namespace grover_utils

} // namespace omniq
