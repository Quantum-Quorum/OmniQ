//
// Created by Goutham Arcot on 17/07/25.
//

#include "Statevector.h"
#include <cmath>
#include <random>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <bitset>

namespace omniq {

// QuantumState implementation
QuantumState::QuantumState(int num_qubits) 
    : num_qubits_(num_qubits), is_normalized_(false) {
    if (num_qubits < 0) {
        throw std::invalid_argument("Number of qubits must be non-negative");
    }
}

// Statevector implementation
Statevector::Statevector(int num_qubits) 
    : QuantumState(num_qubits) {
    size_t size = 1ULL << num_qubits;
    amplitudes_.resize(size, std::complex<double>(0.0, 0.0));
    if (num_qubits > 0) {
        amplitudes_[0] = std::complex<double>(1.0, 0.0); // |0...0⟩ state
    }
    is_normalized_ = true;
}

Statevector::Statevector(const std::vector<std::complex<double>>& amplitudes)
    : QuantumState(static_cast<int>(std::log2(amplitudes.size()))) {
    // Validate that the size is a power of 2
    if (amplitudes.size() == 0 || (amplitudes.size() & (amplitudes.size() - 1)) != 0) {
        throw std::invalid_argument("Amplitude vector size must be a power of 2");
    }
    
    amplitudes_ = amplitudes;
    normalize();
}

Statevector::Statevector(const Statevector& other)
    : QuantumState(other.num_qubits_), amplitudes_(other.amplitudes_) {
    is_normalized_ = other.is_normalized_;
}

Statevector& Statevector::operator=(const Statevector& other) {
    if (this != &other) {
        num_qubits_ = other.num_qubits_;
        amplitudes_ = other.amplitudes_;
        is_normalized_ = other.is_normalized_;
    }
    return *this;
}

const std::complex<double>& Statevector::operator[](size_t index) const {
    if (index >= amplitudes_.size()) {
        throw std::out_of_range("Index out of range");
    }
    return amplitudes_[index];
}

std::complex<double>& Statevector::operator[](size_t index) {
    if (index >= amplitudes_.size()) {
        throw std::out_of_range("Index out of range");
    }
    is_normalized_ = false;
    return amplitudes_[index];
}

void Statevector::normalize() {
    double norm = get_norm();
    if (norm > 1e-10) {
        for (auto& amp : amplitudes_) {
            amp /= norm;
        }
        is_normalized_ = true;
    }
}

double Statevector::get_norm() const {
    double norm_squared = 0.0;
    for (const auto& amp : amplitudes_) {
        norm_squared += std::norm(amp);
    }
    return std::sqrt(norm_squared);
}

std::string Statevector::to_string() const {
    std::ostringstream oss;
    oss << "Statevector(" << num_qubits_ << " qubits):\n";
    for (size_t i = 0; i < amplitudes_.size(); ++i) {
        if (std::abs(amplitudes_[i]) > 1e-10) {
            oss << "|" << std::bitset<32>(i).to_string().substr(32 - num_qubits_) << "⟩: ";
            oss << std::fixed << std::setprecision(6) << amplitudes_[i];
            oss << "\n";
        }
    }
    return oss.str();
}

// Quantum operations
void Statevector::apply_hadamard(int qubit) {
    if (qubit < 0 || qubit >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    
    std::vector<std::complex<double>> new_amplitudes = amplitudes_;
    size_t mask = 1ULL << qubit;
    
    for (size_t i = 0; i < amplitudes_.size(); ++i) {
        if (i & mask) continue; // Skip if this bit is already 1
        
        size_t j = i | mask; // Flip the qubit
        std::complex<double> a = amplitudes_[i];
        std::complex<double> b = amplitudes_[j];
        
        new_amplitudes[i] = (a + b) / std::sqrt(2.0);
        new_amplitudes[j] = (a - b) / std::sqrt(2.0);
    }
    
    amplitudes_ = new_amplitudes;
    is_normalized_ = false;
}

void Statevector::apply_cnot(int control, int target) {
    if (control < 0 || control >= num_qubits_ || target < 0 || target >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    if (control == target) {
        throw std::invalid_argument("Control and target qubits must be different");
    }
    
    size_t control_mask = 1ULL << control;
    size_t target_mask = 1ULL << target;
    
    for (size_t i = 0; i < amplitudes_.size(); ++i) {
        if (i & control_mask) { // If control qubit is 1
            size_t j = i ^ target_mask; // Flip target qubit
            if (j > i) { // Only swap once
                std::swap(amplitudes_[i], amplitudes_[j]);
            }
        }
    }
}

void Statevector::apply_pauli_x(int qubit) {
    if (qubit < 0 || qubit >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    
    size_t mask = 1ULL << qubit;
    for (size_t i = 0; i < amplitudes_.size(); ++i) {
        if (i & mask) continue; // Skip if this bit is already 1
        
        size_t j = i | mask; // Flip the qubit
        std::swap(amplitudes_[i], amplitudes_[j]);
    }
}

void Statevector::apply_pauli_y(int qubit) {
    if (qubit < 0 || qubit >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    
    size_t mask = 1ULL << qubit;
    for (size_t i = 0; i < amplitudes_.size(); ++i) {
        if (i & mask) continue; // Skip if this bit is already 1
        
        size_t j = i | mask; // Flip the qubit
        std::complex<double> temp = amplitudes_[i];
        amplitudes_[i] = std::complex<double>(0, -1) * amplitudes_[j];
        amplitudes_[j] = std::complex<double>(0, 1) * temp;
    }
}

void Statevector::apply_pauli_z(int qubit) {
    if (qubit < 0 || qubit >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    
    size_t mask = 1ULL << qubit;
    for (size_t i = 0; i < amplitudes_.size(); ++i) {
        if (i & mask) { // If qubit is 1, apply -1 phase
            amplitudes_[i] = -amplitudes_[i];
        }
    }
}

void Statevector::apply_phase_shift(int qubit, double angle) {
    if (qubit < 0 || qubit >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    
    std::complex<double> phase = std::exp(std::complex<double>(0, angle));
    size_t mask = 1ULL << qubit;
    
    for (size_t i = 0; i < amplitudes_.size(); ++i) {
        if (i & mask) { // If qubit is 1, apply phase
            amplitudes_[i] *= phase;
        }
    }
}

void Statevector::apply_rotation_x(int qubit, double angle) {
    if (qubit < 0 || qubit >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    
    double cos_half = std::cos(angle / 2.0);
    double sin_half = std::sin(angle / 2.0);
    std::complex<double> i_sin_half(0, -sin_half);
    
    size_t mask = 1ULL << qubit;
    std::vector<std::complex<double>> new_amplitudes = amplitudes_;
    
    for (size_t i = 0; i < amplitudes_.size(); ++i) {
        if (i & mask) continue; // Skip if this bit is already 1
        
        size_t j = i | mask; // Flip the qubit
        std::complex<double> a = amplitudes_[i];
        std::complex<double> b = amplitudes_[j];
        
        new_amplitudes[i] = cos_half * a + i_sin_half * b;
        new_amplitudes[j] = i_sin_half * a + cos_half * b;
    }
    
    amplitudes_ = new_amplitudes;
    is_normalized_ = false;
}

void Statevector::apply_rotation_y(int qubit, double angle) {
    if (qubit < 0 || qubit >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    
    double cos_half = std::cos(angle / 2.0);
    double sin_half = std::sin(angle / 2.0);
    
    size_t mask = 1ULL << qubit;
    std::vector<std::complex<double>> new_amplitudes = amplitudes_;
    
    for (size_t i = 0; i < amplitudes_.size(); ++i) {
        if (i & mask) continue; // Skip if this bit is already 1
        
        size_t j = i | mask; // Flip the qubit
        std::complex<double> a = amplitudes_[i];
        std::complex<double> b = amplitudes_[j];
        
        new_amplitudes[i] = cos_half * a - sin_half * b;
        new_amplitudes[j] = sin_half * a + cos_half * b;
    }
    
    amplitudes_ = new_amplitudes;
    is_normalized_ = false;
}

void Statevector::apply_rotation_z(int qubit, double angle) {
    if (qubit < 0 || qubit >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    
    std::complex<double> phase1 = std::exp(std::complex<double>(0, -angle / 2.0));
    std::complex<double> phase2 = std::exp(std::complex<double>(0, angle / 2.0));
    
    size_t mask = 1ULL << qubit;
    
    for (size_t i = 0; i < amplitudes_.size(); ++i) {
        if (i & mask) { // If qubit is 1
            amplitudes_[i] *= phase2;
        } else { // If qubit is 0
            amplitudes_[i] *= phase1;
        }
    }
}

// Measurements
int Statevector::measure(int qubit) {
    if (qubit < 0 || qubit >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    
    // Calculate probabilities
    double prob_0 = 0.0;
    double prob_1 = 0.0;
    size_t mask = 1ULL << qubit;
    
    for (size_t i = 0; i < amplitudes_.size(); ++i) {
        double prob = std::norm(amplitudes_[i]);
        if (i & mask) {
            prob_1 += prob;
        } else {
            prob_0 += prob;
        }
    }
    
    // Normalize probabilities
    double total_prob = prob_0 + prob_1;
    if (total_prob > 0) {
        prob_0 /= total_prob;
        prob_1 /= total_prob;
    }
    
    // Sample from distribution
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    int result = (dis(gen) < prob_0) ? 0 : 1;
    
    // Collapse the state
    std::vector<std::complex<double>> new_amplitudes = amplitudes_;
    double normalization_factor = 0.0;
    
    for (size_t i = 0; i < amplitudes_.size(); ++i) {
        bool qubit_value = (i & mask) != 0;
        if (qubit_value != result) {
            new_amplitudes[i] = 0.0;
        } else {
            normalization_factor += std::norm(amplitudes_[i]);
        }
    }
    
    // Normalize the collapsed state
    if (normalization_factor > 0) {
        normalization_factor = std::sqrt(normalization_factor);
        for (auto& amp : new_amplitudes) {
            amp /= normalization_factor;
        }
    }
    
    amplitudes_ = new_amplitudes;
    is_normalized_ = true;
    
    return result;
}

double Statevector::measure_expectation(int qubit, const std::string& observable) {
    if (qubit < 0 || qubit >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    
    if (observable == "Z" || observable == "z") {
        double expectation = 0.0;
        size_t mask = 1ULL << qubit;
        
        for (size_t i = 0; i < amplitudes_.size(); ++i) {
            double prob = std::norm(amplitudes_[i]);
            int eigenvalue = (i & mask) ? -1 : 1;
            expectation += prob * eigenvalue;
        }
        
        return expectation;
    } else {
        throw std::invalid_argument("Unsupported observable: " + observable);
    }
}

void Statevector::set_amplitudes(const std::vector<std::complex<double>>& amplitudes) {
    if (amplitudes.size() != (1ULL << num_qubits_)) {
        throw std::invalid_argument("Amplitude vector size mismatch");
    }
    amplitudes_ = amplitudes;
    is_normalized_ = false;
    normalize();
}

Statevector Statevector::tensor_product(const Statevector& other) const {
    int new_num_qubits = num_qubits_ + other.num_qubits_;
    Statevector result(new_num_qubits);
    
    size_t size1 = amplitudes_.size();
    size_t size2 = other.amplitudes_.size();
    
    for (size_t i = 0; i < size1; ++i) {
        for (size_t j = 0; j < size2; ++j) {
            size_t index = i * size2 + j;
            result.amplitudes_[index] = amplitudes_[i] * other.amplitudes_[j];
        }
    }
    
    result.is_normalized_ = true;
    return result;
}

DensityMatrix Statevector::partial_trace(const std::vector<int>& qubits_to_trace) const {
    // This is a simplified implementation
    // In practice, this would be more complex
    int remaining_qubits = num_qubits_ - qubits_to_trace.size();
    DensityMatrix result(remaining_qubits);
    
    // For now, just create a density matrix from the statevector
    size_t size = amplitudes_.size();
    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            result(i, j) = amplitudes_[i] * std::conj(amplitudes_[j]);
        }
    }
    
    return result;
}

} // namespace omniq
