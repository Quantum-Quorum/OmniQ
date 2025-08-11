//
// Created by Goutham Arcot on 17/07/25.
//

#include "DensityMatrix.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <random>

namespace omniq {

DensityMatrix::DensityMatrix(int num_qubits) 
    : QuantumState(num_qubits) {
    size_t size = 1ULL << num_qubits;
    matrix_.resize(size, std::vector<std::complex<double>>(size, std::complex<double>(0.0, 0.0)));
    if (num_qubits > 0) {
        matrix_[0][0] = std::complex<double>(1.0, 0.0); // |0...0⟩⟨0...0|
    }
    is_normalized_ = true;
}

DensityMatrix::DensityMatrix(const std::vector<std::vector<std::complex<double>>>& matrix)
    : QuantumState(static_cast<int>(std::log2(matrix.size()))) {
    // Validate matrix dimensions
    if (matrix.size() == 0 || (matrix.size() & (matrix.size() - 1)) != 0) {
        throw std::invalid_argument("Matrix size must be a power of 2");
    }
    
    for (const auto& row : matrix) {
        if (row.size() != matrix.size()) {
            throw std::invalid_argument("Matrix must be square");
        }
    }
    
    matrix_ = matrix;
    normalize();
}

DensityMatrix::DensityMatrix(const DensityMatrix& other)
    : QuantumState(other.num_qubits_), matrix_(other.matrix_) {
    is_normalized_ = other.is_normalized_;
}

DensityMatrix& DensityMatrix::operator=(const DensityMatrix& other) {
    if (this != &other) {
        num_qubits_ = other.num_qubits_;
        matrix_ = other.matrix_;
        is_normalized_ = other.is_normalized_;
    }
    return *this;
}

const std::complex<double>& DensityMatrix::operator()(size_t row, size_t col) const {
    if (row >= matrix_.size() || col >= matrix_.size()) {
        throw std::out_of_range("Matrix index out of range");
    }
    return matrix_[row][col];
}

std::complex<double>& DensityMatrix::operator()(size_t row, size_t col) {
    if (row >= matrix_.size() || col >= matrix_.size()) {
        throw std::out_of_range("Matrix index out of range");
    }
    is_normalized_ = false;
    return matrix_[row][col];
}

void DensityMatrix::normalize() {
    double trace = 0.0;
    for (size_t i = 0; i < matrix_.size(); ++i) {
        trace += std::real(matrix_[i][i]);
    }
    
    if (std::abs(trace) > 1e-10) {
        for (auto& row : matrix_) {
            for (auto& element : row) {
                element /= trace;
            }
        }
        is_normalized_ = true;
    }
}

double DensityMatrix::get_norm() const {
    double trace = 0.0;
    for (size_t i = 0; i < matrix_.size(); ++i) {
        trace += std::real(matrix_[i][i]);
    }
    return trace;
}

std::string DensityMatrix::to_string() const {
    std::ostringstream oss;
    oss << "DensityMatrix(" << num_qubits_ << " qubits):\n";
    oss << std::fixed << std::setprecision(4);
    
    for (size_t i = 0; i < std::min(size_t(8), matrix_.size()); ++i) {
        for (size_t j = 0; j < std::min(size_t(8), matrix_.size()); ++j) {
            oss << std::setw(10) << matrix_[i][j] << " ";
        }
        if (matrix_.size() > 8) oss << "...";
        oss << "\n";
    }
    if (matrix_.size() > 8) {
        oss << "...\n";
    }
    
    return oss.str();
}

// Quantum operations for density matrix
void DensityMatrix::apply_hadamard(int qubit) {
    if (qubit < 0 || qubit >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    
    size_t mask = 1ULL << qubit;
    size_t size = matrix_.size();
    std::vector<std::vector<std::complex<double>>> new_matrix = matrix_;
    
    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            bool i_bit = (i & mask) != 0;
            bool j_bit = (j & mask) != 0;
            
            if (i_bit == j_bit) {
                // Same bit value - no change
                new_matrix[i][j] = matrix_[i][j];
            } else {
                // Different bit values - apply Hadamard transformation
                size_t i_flipped = i ^ mask;
                size_t j_flipped = j ^ mask;
                
                if (i_bit) {
                    // i has 1, j has 0
                    new_matrix[i][j] = (matrix_[i][j] - matrix_[i_flipped][j]) / std::sqrt(2.0);
                } else {
                    // i has 0, j has 1
                    new_matrix[i][j] = (matrix_[i][j] + matrix_[i_flipped][j]) / std::sqrt(2.0);
                }
            }
        }
    }
    
    matrix_ = new_matrix;
    is_normalized_ = false;
}

void DensityMatrix::apply_cnot(int control, int target) {
    if (control < 0 || control >= num_qubits_ || target < 0 || target >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    if (control == target) {
        throw std::invalid_argument("Control and target qubits must be different");
    }
    
    size_t control_mask = 1ULL << control;
    size_t target_mask = 1ULL << target;
    size_t size = matrix_.size();
    
    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            bool i_control = (i & control_mask) != 0;
            bool j_control = (j & control_mask) != 0;
            
            if (i_control && j_control) {
                // Both control qubits are 1, flip target
                size_t i_flipped = i ^ target_mask;
                size_t j_flipped = j ^ target_mask;
                std::swap(matrix_[i][j], matrix_[i_flipped][j_flipped]);
            }
        }
    }
}

void DensityMatrix::apply_pauli_x(int qubit) {
    if (qubit < 0 || qubit >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    
    size_t mask = 1ULL << qubit;
    size_t size = matrix_.size();
    
    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            if ((i & mask) != (j & mask)) {
                // Different bit values, swap
                size_t i_flipped = i ^ mask;
                size_t j_flipped = j ^ mask;
                std::swap(matrix_[i][j], matrix_[i_flipped][j_flipped]);
            }
        }
    }
}

void DensityMatrix::apply_pauli_y(int qubit) {
    if (qubit < 0 || qubit >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    
    size_t mask = 1ULL << qubit;
    size_t size = matrix_.size();
    std::vector<std::vector<std::complex<double>>> new_matrix = matrix_;
    
    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            bool i_bit = (i & mask) != 0;
            bool j_bit = (j & mask) != 0;
            
            if (i_bit != j_bit) {
                size_t i_flipped = i ^ mask;
                size_t j_flipped = j ^ mask;
                
                if (i_bit) {
                    // i has 1, j has 0
                    new_matrix[i][j] = std::complex<double>(0, -1) * matrix_[i_flipped][j];
                    new_matrix[i_flipped][j] = std::complex<double>(0, 1) * matrix_[i][j_flipped];
                } else {
                    // i has 0, j has 1
                    new_matrix[i][j] = std::complex<double>(0, 1) * matrix_[i_flipped][j];
                    new_matrix[i_flipped][j] = std::complex<double>(0, -1) * matrix_[i][j_flipped];
                }
            }
        }
    }
    
    matrix_ = new_matrix;
}

void DensityMatrix::apply_pauli_z(int qubit) {
    if (qubit < 0 || qubit >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    
    size_t mask = 1ULL << qubit;
    size_t size = matrix_.size();
    
    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            bool i_bit = (i & mask) != 0;
            bool j_bit = (j & mask) != 0;
            
            if (i_bit != j_bit) {
                matrix_[i][j] = -matrix_[i][j];
            }
        }
    }
}

// Measurements
double DensityMatrix::measure_expectation(int qubit, const std::string& observable) {
    if (qubit < 0 || qubit >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    
    if (observable == "Z" || observable == "z") {
        double expectation = 0.0;
        size_t mask = 1ULL << qubit;
        
        for (size_t i = 0; i < matrix_.size(); ++i) {
            double prob = std::real(matrix_[i][i]);
            int eigenvalue = (i & mask) ? -1 : 1;
            expectation += prob * eigenvalue;
        }
        
        return expectation;
    } else {
        throw std::invalid_argument("Unsupported observable: " + observable);
    }
}

void DensityMatrix::set_matrix(const std::vector<std::vector<std::complex<double>>>& matrix) {
    if (matrix.size() != (1ULL << num_qubits_) || 
        (matrix.size() > 0 && matrix[0].size() != matrix.size())) {
        throw std::invalid_argument("Matrix size mismatch");
    }
    matrix_ = matrix;
    is_normalized_ = false;
    normalize();
}

DensityMatrix DensityMatrix::partial_trace(const std::vector<int>& qubits_to_trace) const {
    // This is a simplified implementation
    // In practice, this would be more complex
    int remaining_qubits = num_qubits_ - qubits_to_trace.size();
    DensityMatrix result(remaining_qubits);
    
    // For now, just create a simple density matrix
    // In practice, you'd need to properly trace out the specified qubits
    size_t new_size = 1ULL << remaining_qubits;
    for (size_t i = 0; i < new_size; ++i) {
        for (size_t j = 0; j < new_size; ++j) {
            if (i == j) {
                result(i, j) = std::complex<double>(1.0 / new_size, 0.0);
            } else {
                result(i, j) = std::complex<double>(0.0, 0.0);
            }
        }
    }
    
    return result;
}

double DensityMatrix::get_purity() const {
    double purity = 0.0;
    size_t size = matrix_.size();
    
    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            purity += std::norm(matrix_[i][j]);
        }
    }
    
    return purity;
}

double DensityMatrix::get_entropy() const {
    // Calculate von Neumann entropy: S = -Tr(ρ log ρ)
    // This is a simplified implementation
    double entropy = 0.0;
    size_t size = matrix_.size();
    
    // For simplicity, we'll use the diagonal elements as eigenvalues
    for (size_t i = 0; i < size; ++i) {
        double eigenvalue = std::real(matrix_[i][i]);
        if (eigenvalue > 1e-10) {
            entropy -= eigenvalue * std::log2(eigenvalue);
        }
    }
    
    return entropy;
}

} // namespace omniq
