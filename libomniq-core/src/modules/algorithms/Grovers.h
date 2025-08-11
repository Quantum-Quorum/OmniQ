//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef GROVERS_H
#define GROVERS_H

#include <vector>
#include <functional>
#include <memory>
#include "../../../include/omniq/Circuit.h"

namespace omniq {

/**
 * @brief Oracle function type for Grover's algorithm
 */
using OracleFunction = std::function<bool(const std::vector<int>&)>;

/**
 * @brief Grover's algorithm implementation
 */
class GroversAlgorithm {
private:
    int num_qubits_;
    int num_solutions_;
    OracleFunction oracle_;
    int num_iterations_;
    
public:
    /**
     * @brief Constructor for Grover's algorithm
     * @param num_qubits Number of qubits in the search space
     * @param oracle Oracle function that marks solutions
     * @param num_solutions Number of solutions (optional, for optimal iteration count)
     */
    GroversAlgorithm(int num_qubits, OracleFunction oracle, int num_solutions = 1);
    
    /**
     * @brief Set the number of iterations
     * @param iterations Number of Grover iterations
     */
    void set_iterations(int iterations);
    
    /**
     * @brief Calculate optimal number of iterations
     * @return Optimal number of iterations
     */
    int get_optimal_iterations() const;
    
    /**
     * @brief Build the complete Grover circuit
     * @return Quantum circuit implementing Grover's algorithm
     */
    Circuit build_circuit() const;
    
    /**
     * @brief Build just the oracle circuit
     * @return Oracle circuit
     */
    Circuit build_oracle() const;
    
    /**
     * @brief Build just the diffusion operator
     * @return Diffusion circuit
     */
    Circuit build_diffusion() const;
    
    /**
     * @brief Execute Grover's algorithm
     * @param initial_state Initial quantum state (optional)
     * @return Final quantum state
     */
    Statevector execute(const Statevector& initial_state = Statevector(0)) const;
    
    /**
     * @brief Execute with multiple shots and return measurement results
     * @param num_shots Number of measurement shots
     * @return Vector of measurement results
     */
    std::vector<int> execute_with_measurements(int num_shots = 1000) const;
    
    /**
     * @brief Get success probability
     * @return Probability of measuring a solution
     */
    double get_success_probability() const;
    
    /**
     * @brief Get the number of qubits
     * @return Number of qubits
     */
    int get_num_qubits() const { return num_qubits_; }
    
    /**
     * @brief Get the number of solutions
     * @return Number of solutions
     */
    int get_num_solutions() const { return num_solutions_; }
    
    /**
     * @brief Get the number of iterations
     * @return Number of iterations
     */
    int get_num_iterations() const { return num_iterations_; }
};

/**
 * @brief Oracle gate for Grover's algorithm
 */
class OracleGate : public Gate {
private:
    OracleFunction oracle_;
    int num_qubits_;
    
public:
    OracleGate(int num_qubits, OracleFunction oracle);
    
    void apply(Statevector& state) const override;
    void apply(DensityMatrix& state) const override;
    std::string to_string() const override;
    
private:
    void apply_oracle_to_statevector(Statevector& state) const;
};

/**
 * @brief Diffusion operator for Grover's algorithm
 */
class DiffusionGate : public Gate {
private:
    int num_qubits_;
    
public:
    explicit DiffusionGate(int num_qubits);
    
    void apply(Statevector& state) const override;
    void apply(DensityMatrix& state) const override;
    std::string to_string() const override;
    
private:
    void apply_diffusion_to_statevector(Statevector& state) const;
};

/**
 * @brief Utility functions for Grover's algorithm
 */
namespace grover_utils {
    
    /**
     * @brief Create a simple oracle for database search
     * @param target_value The value to search for
     * @return Oracle function
     */
    OracleFunction create_database_oracle(int target_value);
    
    /**
     * @brief Create an oracle for SAT problems
     * @param clauses Vector of clauses (each clause is a vector of literals)
     * @param num_variables Number of variables
     * @return Oracle function
     */
    OracleFunction create_sat_oracle(const std::vector<std::vector<int>>& clauses, int num_variables);
    
    /**
     * @brief Create an oracle for graph coloring
     * @param edges Vector of edges (pairs of vertices)
     * @param num_vertices Number of vertices
     * @param num_colors Number of colors
     * @return Oracle function
     */
    OracleFunction create_graph_coloring_oracle(const std::vector<std::pair<int, int>>& edges, 
                                               int num_vertices, int num_colors);
    
    /**
     * @brief Calculate optimal number of iterations for given parameters
     * @param num_qubits Number of qubits
     * @param num_solutions Number of solutions
     * @return Optimal number of iterations
     */
    int calculate_optimal_iterations(int num_qubits, int num_solutions);
    
    /**
     * @brief Estimate success probability
     * @param num_qubits Number of qubits
     * @param num_solutions Number of solutions
     * @param num_iterations Number of iterations
     * @return Estimated success probability
     */
    double estimate_success_probability(int num_qubits, int num_solutions, int num_iterations);
}

} // namespace omniq

#endif // GROVERS_H
