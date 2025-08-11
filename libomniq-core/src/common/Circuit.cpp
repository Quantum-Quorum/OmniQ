//
// Created by Goutham Arcot on 17/07/25.
//

#include "Circuit.h"
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iostream>

namespace omniq {

// Gate implementation
Gate::Gate(GateType type, const std::vector<int>& qubits, 
           const std::vector<double>& parameters, const std::string& name)
    : type_(type), qubits_(qubits), parameters_(parameters), name_(name) {
}

// Measurement implementation
Measurement::Measurement(int qubit, const std::string& basis, const std::string& observable)
    : qubit_(qubit), basis_(basis), observable_(observable) {
}

int Measurement::execute(Statevector& state) const {
    return state.measure(qubit_);
}

double Measurement::execute_expectation(DensityMatrix& state) const {
    return state.measure_expectation(qubit_, observable_);
}

std::string Measurement::to_string() const {
    std::ostringstream oss;
    oss << "measure(" << qubit_ << ", " << basis_ << ", " << observable_ << ")";
    return oss.str();
}

// Circuit implementation
Circuit::Circuit(int num_qubits, int num_classical_bits)
    : num_qubits_(num_qubits), num_classical_bits_(num_classical_bits) {
    if (num_qubits < 0) {
        throw std::invalid_argument("Number of qubits must be non-negative");
    }
}

Circuit::Circuit(const Circuit& other)
    : num_qubits_(other.num_qubits_), 
      num_classical_bits_(other.num_classical_bits_),
      gates_(other.gates_),
      measurements_(other.measurements_),
      custom_gates_(other.custom_gates_) {
}

Circuit& Circuit::operator=(const Circuit& other) {
    if (this != &other) {
        num_qubits_ = other.num_qubits_;
        num_classical_bits_ = other.num_classical_bits_;
        gates_ = other.gates_;
        measurements_ = other.measurements_;
        custom_gates_ = other.custom_gates_;
    }
    return *this;
}

// Circuit construction
void Circuit::add_gate(std::shared_ptr<Gate> gate) {
    if (!gate) {
        throw std::invalid_argument("Gate cannot be null");
    }
    
    // Validate qubit indices
    for (int qubit : gate->get_qubits()) {
        if (qubit < 0 || qubit >= num_qubits_) {
            throw std::invalid_argument("Qubit index out of range");
        }
    }
    
    gates_.push_back(gate);
}

void Circuit::add_measurement(std::shared_ptr<Measurement> measurement) {
    if (!measurement) {
        throw std::invalid_argument("Measurement cannot be null");
    }
    
    if (measurement->get_qubit() < 0 || measurement->get_qubit() >= num_qubits_) {
        throw std::invalid_argument("Qubit index out of range");
    }
    
    measurements_.push_back(measurement);
}

void Circuit::add_hadamard(int qubit) {
    add_gate(std::make_shared<HadamardGate>(qubit));
}

void Circuit::add_pauli_x(int qubit) {
    add_gate(std::make_shared<PauliXGate>(qubit));
}

void Circuit::add_pauli_y(int qubit) {
    add_gate(std::make_shared<PauliYGate>(qubit));
}

void Circuit::add_pauli_z(int qubit) {
    add_gate(std::make_shared<PauliZGate>(qubit));
}

void Circuit::add_cnot(int control, int target) {
    add_gate(std::make_shared<CNOTGate>(control, target));
}

void Circuit::add_swap(int qubit1, int qubit2) {
    // Implement SWAP as three CNOTs
    add_cnot(qubit1, qubit2);
    add_cnot(qubit2, qubit1);
    add_cnot(qubit1, qubit2);
}

void Circuit::add_phase_shift(int qubit, double angle) {
    add_gate(std::make_shared<PhaseShiftGate>(qubit, angle));
}

void Circuit::add_rotation_x(int qubit, double angle) {
    add_gate(std::make_shared<RotationXGate>(qubit, angle));
}

void Circuit::add_rotation_y(int qubit, double angle) {
    add_gate(std::make_shared<RotationYGate>(qubit, angle));
}

void Circuit::add_rotation_z(int qubit, double angle) {
    add_gate(std::make_shared<RotationZGate>(qubit, angle));
}

void Circuit::add_measurement(int qubit, const std::string& basis) {
    add_measurement(std::make_shared<Measurement>(qubit, basis));
}

// Custom gates
void Circuit::add_custom_gate(const std::string& name, std::shared_ptr<Gate> gate) {
    if (name.empty()) {
        throw std::invalid_argument("Gate name cannot be empty");
    }
    custom_gates_[name] = gate;
}

void Circuit::apply_custom_gate(const std::string& name, const std::vector<int>& qubits) {
    auto it = custom_gates_.find(name);
    if (it == custom_gates_.end()) {
        throw std::invalid_argument("Custom gate not found: " + name);
    }
    
    // Create a copy of the custom gate with the specified qubits
    auto gate = it->second;
    // Note: This is a simplified implementation. In practice, you'd need to
    // properly map the qubits and parameters.
    add_gate(gate);
}

// Circuit execution
Statevector Circuit::execute_statevector(const Statevector& initial_state) const {
    Statevector state = initial_state;
    
    if (state.get_num_qubits() == 0) {
        // Create default state if none provided
        state = Statevector(num_qubits_);
    } else if (state.get_num_qubits() != num_qubits_) {
        throw std::invalid_argument("Initial state qubit count mismatch");
    }
    
    // Apply all gates
    for (const auto& gate : gates_) {
        gate->apply(state);
    }
    
    return state;
}

DensityMatrix Circuit::execute_density_matrix(const DensityMatrix& initial_state) const {
    DensityMatrix state = initial_state;
    
    if (state.get_num_qubits() == 0) {
        // Create default state if none provided
        state = DensityMatrix(num_qubits_);
    } else if (state.get_num_qubits() != num_qubits_) {
        throw std::invalid_argument("Initial state qubit count mismatch");
    }
    
    // Apply all gates
    for (const auto& gate : gates_) {
        gate->apply(state);
    }
    
    return state;
}

// Circuit analysis
int Circuit::get_depth() const {
    if (gates_.empty()) return 0;
    
    // Simple depth calculation - in practice, this would be more sophisticated
    // considering gate dependencies and parallel execution
    return static_cast<int>(gates_.size());
}

// Circuit manipulation
void Circuit::reset() {
    gates_.clear();
    measurements_.clear();
}

void Circuit::remove_gate(size_t index) {
    if (index >= gates_.size()) {
        throw std::out_of_range("Gate index out of range");
    }
    gates_.erase(gates_.begin() + index);
}

void Circuit::remove_measurement(size_t index) {
    if (index >= measurements_.size()) {
        throw std::out_of_range("Measurement index out of range");
    }
    measurements_.erase(measurements_.begin() + index);
}

void Circuit::clear() {
    reset();
    custom_gates_.clear();
}

// Circuit composition
Circuit Circuit::compose(const Circuit& other) const {
    if (other.num_qubits_ != num_qubits_) {
        throw std::invalid_argument("Circuit qubit count mismatch for composition");
    }
    
    Circuit result(num_qubits_, num_classical_bits_);
    
    // Add gates from this circuit
    for (const auto& gate : gates_) {
        result.add_gate(gate);
    }
    
    // Add gates from other circuit
    for (const auto& gate : other.gates_) {
        result.add_gate(gate);
    }
    
    // Add measurements from this circuit
    for (const auto& measurement : measurements_) {
        result.add_measurement(measurement);
    }
    
    // Add measurements from other circuit
    for (const auto& measurement : other.measurements_) {
        result.add_measurement(measurement);
    }
    
    return result;
}

Circuit Circuit::tensor_product(const Circuit& other) const {
    Circuit result(num_qubits_ + other.num_qubits_, 
                   num_classical_bits_ + other.num_classical_bits_);
    
    // Add gates from this circuit (qubit indices remain the same)
    for (const auto& gate : gates_) {
        result.add_gate(gate);
    }
    
    // Add gates from other circuit (qubit indices need to be shifted)
    for (const auto& gate : other.gates_) {
        // Note: This is a simplified implementation. In practice, you'd need to
        // properly shift the qubit indices for the second circuit.
        result.add_gate(gate);
    }
    
    return result;
}

// Circuit visualization
std::string Circuit::to_string() const {
    std::ostringstream oss;
    oss << "Circuit(" << num_qubits_ << " qubits, " << num_classical_bits_ << " classical bits)\n";
    oss << "Gates:\n";
    
    for (size_t i = 0; i < gates_.size(); ++i) {
        oss << "  " << i << ": " << gates_[i]->to_string() << "\n";
    }
    
    oss << "Measurements:\n";
    for (size_t i = 0; i < measurements_.size(); ++i) {
        oss << "  " << i << ": " << measurements_[i]->to_string() << "\n";
    }
    
    return oss.str();
}

std::string Circuit::to_qasm() const {
    std::ostringstream oss;
    oss << "OPENQASM 2.0;\n";
    oss << "include \"qelib1.inc\";\n\n";
    oss << "qreg q[" << num_qubits_ << "];\n";
    if (num_classical_bits_ > 0) {
        oss << "creg c[" << num_classical_bits_ << "];\n\n";
    }
    
    for (const auto& gate : gates_) {
        // Convert gate to QASM format
        if (gate->get_type() == GateType::HADAMARD) {
            oss << "h q[" << gate->get_qubits()[0] << "];\n";
        } else if (gate->get_type() == GateType::PAULI_X) {
            oss << "x q[" << gate->get_qubits()[0] << "];\n";
        } else if (gate->get_type() == GateType::CNOT) {
            oss << "cx q[" << gate->get_qubits()[0] << "],q[" << gate->get_qubits()[1] << "];\n";
        }
        // Add more gate types as needed
    }
    
    for (const auto& measurement : measurements_) {
        oss << "measure q[" << measurement->get_qubit() << "] -> c[" << measurement->get_qubit() << "];\n";
    }
    
    return oss.str();
}

// Circuit optimization
void Circuit::optimize() {
    // Simple optimization: remove redundant gates
    // In practice, this would be much more sophisticated
    std::vector<std::shared_ptr<Gate>> optimized_gates;
    
    for (const auto& gate : gates_) {
        // Skip redundant gates (simplified check)
        bool is_redundant = false;
        
        // Check if this gate cancels with the previous one
        if (!optimized_gates.empty()) {
            auto prev_gate = optimized_gates.back();
            if (gate->get_type() == prev_gate->get_type() && 
                gate->get_qubits() == prev_gate->get_qubits()) {
                // Some gates are self-inverse
                if (gate->get_type() == GateType::HADAMARD ||
                    gate->get_type() == GateType::PAULI_X ||
                    gate->get_type() == GateType::PAULI_Z) {
                    optimized_gates.pop_back();
                    is_redundant = true;
                }
            }
        }
        
        if (!is_redundant) {
            optimized_gates.push_back(gate);
        }
    }
    
    gates_ = optimized_gates;
}

void Circuit::decompose_to_basic_gates() {
    // This would decompose complex gates into basic ones
    // For now, it's a placeholder
}

// Circuit validation
bool Circuit::is_valid() const {
    return get_validation_errors().empty();
}

std::vector<std::string> Circuit::get_validation_errors() const {
    std::vector<std::string> errors;
    
    // Check qubit indices
    for (const auto& gate : gates_) {
        for (int qubit : gate->get_qubits()) {
            if (qubit < 0 || qubit >= num_qubits_) {
                errors.push_back("Gate uses invalid qubit index: " + std::to_string(qubit));
            }
        }
    }
    
    for (const auto& measurement : measurements_) {
        if (measurement->get_qubit() < 0 || measurement->get_qubit() >= num_qubits_) {
            errors.push_back("Measurement uses invalid qubit index: " + 
                           std::to_string(measurement->get_qubit()));
        }
    }
    
    return errors;
}

// Concrete gate implementations
HadamardGate::HadamardGate(int qubit) 
    : Gate(GateType::HADAMARD, {qubit}) {
}

void HadamardGate::apply(Statevector& state) const {
    state.apply_hadamard(qubits_[0]);
}

void HadamardGate::apply(DensityMatrix& state) const {
    state.apply_hadamard(qubits_[0]);
}

std::string HadamardGate::to_string() const {
    return "H(" + std::to_string(qubits_[0]) + ")";
}

PauliXGate::PauliXGate(int qubit) 
    : Gate(GateType::PAULI_X, {qubit}) {
}

void PauliXGate::apply(Statevector& state) const {
    state.apply_pauli_x(qubits_[0]);
}

void PauliXGate::apply(DensityMatrix& state) const {
    state.apply_pauli_x(qubits_[0]);
}

std::string PauliXGate::to_string() const {
    return "X(" + std::to_string(qubits_[0]) + ")";
}

PauliYGate::PauliYGate(int qubit) 
    : Gate(GateType::PAULI_Y, {qubit}) {
}

void PauliYGate::apply(Statevector& state) const {
    state.apply_pauli_y(qubits_[0]);
}

void PauliYGate::apply(DensityMatrix& state) const {
    state.apply_pauli_y(qubits_[0]);
}

std::string PauliYGate::to_string() const {
    return "Y(" + std::to_string(qubits_[0]) + ")";
}

PauliZGate::PauliZGate(int qubit) 
    : Gate(GateType::PAULI_Z, {qubit}) {
}

void PauliZGate::apply(Statevector& state) const {
    state.apply_pauli_z(qubits_[0]);
}

void PauliZGate::apply(DensityMatrix& state) const {
    state.apply_pauli_z(qubits_[0]);
}

std::string PauliZGate::to_string() const {
    return "Z(" + std::to_string(qubits_[0]) + ")";
}

CNOTGate::CNOTGate(int control, int target) 
    : Gate(GateType::CNOT, {control, target}) {
}

void CNOTGate::apply(Statevector& state) const {
    state.apply_cnot(qubits_[0], qubits_[1]);
}

void CNOTGate::apply(DensityMatrix& state) const {
    state.apply_cnot(qubits_[0], qubits_[1]);
}

std::string CNOTGate::to_string() const {
    return "CNOT(" + std::to_string(qubits_[0]) + ", " + std::to_string(qubits_[1]) + ")";
}

PhaseShiftGate::PhaseShiftGate(int qubit, double angle) 
    : Gate(GateType::PHASE_SHIFT, {qubit}, {angle}) {
}

void PhaseShiftGate::apply(Statevector& state) const {
    state.apply_phase_shift(qubits_[0], parameters_[0]);
}

void PhaseShiftGate::apply(DensityMatrix& state) const {
    // Implement for density matrix
}

std::string PhaseShiftGate::to_string() const {
    return "P(" + std::to_string(qubits_[0]) + ", " + std::to_string(parameters_[0]) + ")";
}

RotationXGate::RotationXGate(int qubit, double angle) 
    : Gate(GateType::ROTATION_X, {qubit}, {angle}) {
}

void RotationXGate::apply(Statevector& state) const {
    state.apply_rotation_x(qubits_[0], parameters_[0]);
}

void RotationXGate::apply(DensityMatrix& state) const {
    // Implement for density matrix
}

std::string RotationXGate::to_string() const {
    return "RX(" + std::to_string(qubits_[0]) + ", " + std::to_string(parameters_[0]) + ")";
}

RotationYGate::RotationYGate(int qubit, double angle) 
    : Gate(GateType::ROTATION_Y, {qubit}, {angle}) {
}

void RotationYGate::apply(Statevector& state) const {
    state.apply_rotation_y(qubits_[0], parameters_[0]);
}

void RotationYGate::apply(DensityMatrix& state) const {
    // Implement for density matrix
}

std::string RotationYGate::to_string() const {
    return "RY(" + std::to_string(qubits_[0]) + ", " + std::to_string(parameters_[0]) + ")";
}

RotationZGate::RotationZGate(int qubit, double angle) 
    : Gate(GateType::ROTATION_Z, {qubit}, {angle}) {
}

void RotationZGate::apply(Statevector& state) const {
    state.apply_rotation_z(qubits_[0], parameters_[0]);
}

void RotationZGate::apply(DensityMatrix& state) const {
    // Implement for density matrix
}

std::string RotationZGate::to_string() const {
    return "RZ(" + std::to_string(qubits_[0]) + ", " + std::to_string(parameters_[0]) + ")";
}

} // namespace omniq
