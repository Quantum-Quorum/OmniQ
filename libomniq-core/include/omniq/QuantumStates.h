//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef OMNIQ_QUANTUMSTATES_H
#define OMNIQ_QUANTUMSTATES_H

#include <vector>
#include <complex>
#include <memory>
#include <string>

namespace omniq {

// Forward declarations
class Statevector;
class DensityMatrix;

/**
 * @brief Base class for quantum state representations
 */
class QuantumState {
protected:
    int num_qubits_;
    bool is_normalized_;

public:
    explicit QuantumState(int num_qubits);
    virtual ~QuantumState() = default;
    
    int get_num_qubits() const { return num_qubits_; }
    bool is_normalized() const { return is_normalized_; }
    
    virtual void normalize() = 0;
    virtual double get_norm() const = 0;
    virtual std::string to_string() const = 0;
};

/**
 * @brief Statevector representation of a quantum state
 */
class Statevector : public QuantumState {
private:
    std::vector<std::complex<double>> amplitudes_;
    
public:
    explicit Statevector(int num_qubits);
    Statevector(const std::vector<std::complex<double>>& amplitudes);
    
    // Copy constructor and assignment
    Statevector(const Statevector& other);
    Statevector& operator=(const Statevector& other);
    
    // Access amplitudes
    const std::complex<double>& operator[](size_t index) const;
    std::complex<double>& operator[](size_t index);
    
    // State operations
    void normalize() override;
    double get_norm() const override;
    std::string to_string() const override;
    
    // Quantum operations
    void apply_hadamard(int qubit);
    void apply_cnot(int control, int target);
    void apply_pauli_x(int qubit);
    void apply_pauli_y(int qubit);
    void apply_pauli_z(int qubit);
    void apply_phase_shift(int qubit, double angle);
    void apply_rotation_x(int qubit, double angle);
    void apply_rotation_y(int qubit, double angle);
    void apply_rotation_z(int qubit, double angle);
    
    // Measurements
    int measure(int qubit);
    double measure_expectation(int qubit, const std::string& observable);
    
    // Utility functions
    size_t size() const { return amplitudes_.size(); }
    const std::vector<std::complex<double>>& get_amplitudes() const { return amplitudes_; }
    void set_amplitudes(const std::vector<std::complex<double>>& amplitudes);
    
    // Tensor product
    Statevector tensor_product(const Statevector& other) const;
    
    // Partial trace
    DensityMatrix partial_trace(const std::vector<int>& qubits_to_trace) const;
};

/**
 * @brief Density matrix representation of a quantum state
 */
class DensityMatrix : public QuantumState {
private:
    std::vector<std::vector<std::complex<double>>> matrix_;
    
public:
    explicit DensityMatrix(int num_qubits);
    DensityMatrix(const std::vector<std::vector<std::complex<double>>>& matrix);
    
    // Copy constructor and assignment
    DensityMatrix(const DensityMatrix& other);
    DensityMatrix& operator=(const DensityMatrix& other);
    
    // Access matrix elements
    const std::complex<double>& operator()(size_t row, size_t col) const;
    std::complex<double>& operator()(size_t row, size_t col);
    
    // State operations
    void normalize() override;
    double get_norm() const override;
    std::string to_string() const override;
    
    // Quantum operations
    void apply_hadamard(int qubit);
    void apply_cnot(int control, int target);
    void apply_pauli_x(int qubit);
    void apply_pauli_y(int qubit);
    void apply_pauli_z(int qubit);
    
    // Measurements
    double measure_expectation(int qubit, const std::string& observable);
    
    // Utility functions
    size_t size() const { return matrix_.size(); }
    const std::vector<std::vector<std::complex<double>>>& get_matrix() const { return matrix_; }
    void set_matrix(const std::vector<std::vector<std::complex<double>>>& matrix);
    
    // Partial trace
    DensityMatrix partial_trace(const std::vector<int>& qubits_to_trace) const;
    
    // Purity calculation
    double get_purity() const;
    
    // Von Neumann entropy
    double get_entropy() const;
};

} // namespace omniq

#endif // OMNIQ_QUANTUMSTATES_H 