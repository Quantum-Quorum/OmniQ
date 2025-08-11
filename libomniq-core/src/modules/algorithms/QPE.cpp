//
// Created by Goutham Arcot on 17/07/25.
//

#include "QPE.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>

namespace omniq {

// QPE implementation
QPE::QPE(int num_precision_qubits, int num_eigenstate_qubits, UnitaryOperator unitary)
    : num_precision_qubits_(num_precision_qubits), 
      num_eigenstate_qubits_(num_eigenstate_qubits),
      unitary_(unitary) {
    if (num_precision_qubits <= 0) {
        throw std::invalid_argument("Number of precision qubits must be positive");
    }
    if (num_eigenstate_qubits <= 0) {
        throw std::invalid_argument("Number of eigenstate qubits must be positive");
    }
    if (!unitary) {
        throw std::invalid_argument("Unitary operator cannot be null");
    }
}

void QPE::set_eigenvalues_and_states(const std::vector<double>& eigenvalues,
                                    const std::vector<std::complex<double>>& eigenstates) {
    eigenvalues_ = eigenvalues;
    eigenstates_ = eigenstates;
}

Circuit QPE::build_circuit() const {
    Circuit circuit(get_total_qubits());
    
    // Initialize precision qubits to |0⟩ and apply Hadamard
    for (int i = 0; i < num_precision_qubits_; ++i) {
        circuit.add_hadamard(i);
    }
    
    // Apply controlled unitary operations
    for (int i = 0; i < num_precision_qubits_; ++i) {
        int power = 1 << i; // 2^i
        auto controlled_unitary = std::make_shared<ControlledUnitaryGate>(
            i, num_precision_qubits_, num_eigenstate_qubits_, unitary_, power);
        circuit.add_gate(controlled_unitary);
    }
    
    // Apply inverse QFT to precision qubits
    auto inverse_qft = std::make_shared<QFTGate>(num_precision_qubits_, true);
    circuit.add_gate(inverse_qft);
    
    return circuit;
}

Circuit QPE::build_qft() const {
    Circuit circuit(num_precision_qubits_);
    auto qft_gate = std::make_shared<QFTGate>(num_precision_qubits_, false);
    circuit.add_gate(qft_gate);
    return circuit;
}

Circuit QPE::build_inverse_qft() const {
    Circuit circuit(num_precision_qubits_);
    auto inverse_qft_gate = std::make_shared<QFTGate>(num_precision_qubits_, true);
    circuit.add_gate(inverse_qft_gate);
    return circuit;
}

Statevector QPE::execute(const Statevector& initial_state) const {
    Circuit circuit = build_circuit();
    return circuit.execute_statevector(initial_state);
}

std::vector<double> QPE::execute_with_measurements(int num_shots) const {
    if (num_shots <= 0) {
        throw std::invalid_argument("Number of shots must be positive");
    }
    
    std::vector<double> phase_estimates;
    phase_estimates.reserve(num_shots);
    
    for (int shot = 0; shot < num_shots; ++shot) {
        Statevector state = execute();
        
        // Measure precision qubits
        int measurement = 0;
        for (int i = 0; i < num_precision_qubits_; ++i) {
            int bit = state.measure(i);
            measurement |= (bit << i);
        }
        
        // Convert measurement to phase estimate
        double phase = static_cast<double>(measurement) / (1 << num_precision_qubits_);
        phase_estimates.push_back(phase);
    }
    
    return phase_estimates;
}

// QFTGate implementation
QFTGate::QFTGate(int num_qubits, bool inverse)
    : Gate(GateType::CUSTOM, {}, {}, inverse ? "InverseQFT" : "QFT"), 
      num_qubits_(num_qubits), inverse_(inverse) {
}

void QFTGate::apply(Statevector& state) const {
    if (inverse_) {
        apply_inverse_qft_to_statevector(state);
    } else {
        apply_qft_to_statevector(state);
    }
}

void QFTGate::apply(DensityMatrix& state) const {
    // For density matrix, we need to apply QFT to both sides
    // This is a simplified implementation
}

std::string QFTGate::to_string() const {
    return (inverse_ ? "InverseQFT(" : "QFT(") + std::to_string(num_qubits_) + ")";
}

void QFTGate::apply_qft_to_statevector(Statevector& state) const {
    if (state.get_num_qubits() != num_qubits_) {
        throw std::invalid_argument("State qubit count mismatch");
    }
    
    std::vector<std::complex<double>> new_amplitudes(state.size());
    
    for (size_t j = 0; j < state.size(); ++j) {
        std::complex<double> sum(0.0, 0.0);
        for (size_t k = 0; k < state.size(); ++k) {
            double phase = 2.0 * M_PI * j * k / state.size();
            std::complex<double> factor = std::exp(std::complex<double>(0, phase));
            sum += state[k] * factor;
        }
        new_amplitudes[j] = sum / std::sqrt(static_cast<double>(state.size()));
    }
    
    state.set_amplitudes(new_amplitudes);
}

void QFTGate::apply_inverse_qft_to_statevector(Statevector& state) const {
    if (state.get_num_qubits() != num_qubits_) {
        throw std::invalid_argument("State qubit count mismatch");
    }
    
    std::vector<std::complex<double>> new_amplitudes(state.size());
    
    for (size_t j = 0; j < state.size(); ++j) {
        std::complex<double> sum(0.0, 0.0);
        for (size_t k = 0; k < state.size(); ++k) {
            double phase = -2.0 * M_PI * j * k / state.size();
            std::complex<double> factor = std::exp(std::complex<double>(0, phase));
            sum += state[k] * factor;
        }
        new_amplitudes[j] = sum / std::sqrt(static_cast<double>(state.size()));
    }
    
    state.set_amplitudes(new_amplitudes);
}

// ControlledUnitaryGate implementation
ControlledUnitaryGate::ControlledUnitaryGate(int control_qubit, int target_start, 
                                           int num_target_qubits, UnitaryOperator unitary, int power)
    : Gate(GateType::CUSTOM, {control_qubit}, {}, "ControlledUnitary"),
      control_qubit_(control_qubit), target_start_(target_start), 
      num_target_qubits_(num_target_qubits), unitary_(unitary), power_(power) {
    if (!unitary) {
        throw std::invalid_argument("Unitary operator cannot be null");
    }
}

void ControlledUnitaryGate::apply(Statevector& state) const {
    apply_controlled_unitary_to_statevector(state);
}

void ControlledUnitaryGate::apply(DensityMatrix& state) const {
    // For density matrix, we need to apply controlled unitary to both sides
    // This is a simplified implementation
}

std::string ControlledUnitaryGate::to_string() const {
    return "ControlledUnitary(" + std::to_string(control_qubit_) + ", " + 
           std::to_string(target_start_) + ", " + std::to_string(power_) + ")";
}

void ControlledUnitaryGate::apply_controlled_unitary_to_statevector(Statevector& state) const {
    if (state.get_num_qubits() < control_qubit_ + 1 || 
        state.get_num_qubits() < target_start_ + num_target_qubits_) {
        throw std::invalid_argument("State qubit count too small for controlled unitary");
    }
    
    // Apply the unitary operator power_ times when control qubit is 1
    size_t control_mask = 1ULL << control_qubit_;
    
    for (size_t i = 0; i < state.size(); ++i) {
        if (i & control_mask) { // If control qubit is 1
            // Create a temporary state for the target qubits
            Statevector temp_state(num_target_qubits_);
            
            // Extract the target qubit values
            for (int j = 0; j < num_target_qubits_; ++j) {
                size_t target_mask = 1ULL << (target_start_ + j);
                if (i & target_mask) {
                    temp_state[j] = std::complex<double>(1.0, 0.0);
                }
            }
            
            // Apply unitary operator power_ times
            for (int p = 0; p < power_; ++p) {
                unitary_(temp_state, target_start_);
            }
            
            // Update the original state with the transformed target qubits
            // This is a simplified implementation - in practice, you'd need to
            // properly handle the tensor product structure
        }
    }
}

// Utility functions implementation
namespace qpe_utils {

UnitaryOperator create_phase_rotation_unitary(double phase) {
    return [phase](Statevector& state, int start_qubit) {
        // Apply phase rotation to the first qubit
        if (start_qubit < state.get_num_qubits()) {
            state.apply_phase_shift(start_qubit, phase);
        }
    };
}

UnitaryOperator create_controlled_phase_rotation_unitary(double phase) {
    return [phase](Statevector& state, int start_qubit) {
        // Apply controlled phase rotation
        if (start_qubit + 1 < state.get_num_qubits()) {
            // Apply phase shift to target qubit when control is 1
            // This is a simplified implementation
            state.apply_phase_shift(start_qubit + 1, phase);
        }
    };
}

double phase_to_eigenvalue(double phase_measurement, int num_precision_qubits) {
    // Convert phase measurement to eigenvalue
    // Phase is normalized to [0, 1), eigenvalue is typically in [0, 2π)
    return 2.0 * M_PI * phase_measurement;
}

double eigenvalue_to_phase(double eigenvalue, int num_precision_qubits) {
    // Convert eigenvalue to phase
    // Normalize eigenvalue to [0, 1)
    double phase = eigenvalue / (2.0 * M_PI);
    return phase - std::floor(phase); // Ensure phase is in [0, 1)
}

double estimate_precision(int num_precision_qubits) {
    // Precision in bits is approximately the number of precision qubits
    return static_cast<double>(num_precision_qubits);
}

double calculate_success_probability(int num_precision_qubits, double true_phase) {
    // Success probability depends on how well the true phase can be represented
    // with the given number of precision qubits
    double phase_error = std::abs(true_phase - eigenvalue_to_phase(true_phase, num_precision_qubits));
    double success_prob = std::cos(M_PI * phase_error * (1 << num_precision_qubits));
    return std::pow(success_prob, 2);
}

} // namespace qpe_utils

} // namespace omniq
