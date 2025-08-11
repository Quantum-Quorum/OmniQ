//
// Created by Goutham Arcot on 17/07/25.
//

#include "omniq/Circuit.h"
#include "omniq/Operators.h"
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace omniq {

Circuit::Circuit(int numQubits, int numClassicalBits)
    : numQubits_(numQubits)
    , numClassicalBits_(numClassicalBits)
    , currentStep_(0)
{
    if (numQubits <= 0) {
        throw std::invalid_argument("Number of qubits must be positive");
    }
    
    // Initialize qubit states to |0⟩
    stateVector_ = VectorXcd::Zero(static_cast<int>(std::pow(2, numQubits)));
    stateVector_(0) = 1.0; // |0...0⟩ state
    
    // Initialize classical bits to 0
    classicalBits_.resize(numClassicalBits, 0);
}

Circuit::Circuit(const Circuit& other)
    : numQubits_(other.numQubits_)
    , numClassicalBits_(other.numClassicalBits_)
    , gates_(other.gates_)
    , stateVector_(other.stateVector_)
    , classicalBits_(other.classicalBits_)
    , currentStep_(other.currentStep_)
{
}

Circuit& Circuit::operator=(const Circuit& other) {
    if (this != &other) {
        numQubits_ = other.numQubits_;
        numClassicalBits_ = other.numClassicalBits_;
        gates_ = other.gates_;
        stateVector_ = other.stateVector_;
        classicalBits_ = other.classicalBits_;
        currentStep_ = other.currentStep_;
    }
    return *this;
}

void Circuit::addGate(const Gate& gate) {
    // Validate gate
    validateGate(gate);
    
    gates_.push_back(gate);
}

void Circuit::addGate(GateType type, int targetQubit, double parameter) {
    Gate gate;
    gate.type = type;
    gate.targetQubits = {targetQubit};
    gate.parameters = {parameter};
    
    addGate(gate);
}

void Circuit::addGate(GateType type, int controlQubit, int targetQubit, double parameter) {
    Gate gate;
    gate.type = type;
    gate.controlQubits = {controlQubit};
    gate.targetQubits = {targetQubit};
    gate.parameters = {parameter};
    
    addGate(gate);
}

void Circuit::addGate(GateType type, const std::vector<int>& targetQubits, const std::vector<double>& parameters) {
    Gate gate;
    gate.type = type;
    gate.targetQubits = targetQubits;
    gate.parameters = parameters;
    
    addGate(gate);
}

void Circuit::removeGate(int index) {
    if (index < 0 || index >= static_cast<int>(gates_.size())) {
        throw std::out_of_range("Gate index out of range");
    }
    
    gates_.erase(gates_.begin() + index);
}

void Circuit::insertGate(int index, const Gate& gate) {
    if (index < 0 || index > static_cast<int>(gates_.size())) {
        throw std::out_of_range("Gate index out of range");
    }
    
    validateGate(gate);
    gates_.insert(gates_.begin() + index, gate);
}

void Circuit::clear() {
    gates_.clear();
    reset();
}

void Circuit::reset() {
    // Reset to initial state |0...0⟩
    stateVector_ = VectorXcd::Zero(static_cast<int>(std::pow(2, numQubits_)));
    stateVector_(0) = 1.0;
    
    // Reset classical bits
    std::fill(classicalBits_.begin(), classicalBits_.end(), 0);
    
    currentStep_ = 0;
}

bool Circuit::executeStep() {
    if (currentStep_ >= static_cast<int>(gates_.size())) {
        return false; // No more gates to execute
    }
    
    const Gate& gate = gates_[currentStep_];
    applyGate(gate);
    currentStep_++;
    
    return true;
}

void Circuit::executeAll() {
    while (executeStep()) {
        // Continue until all gates are executed
    }
}

void Circuit::executeToStep(int step) {
    if (step < 0 || step > static_cast<int>(gates_.size())) {
        throw std::out_of_range("Step index out of range");
    }
    
    while (currentStep_ < step) {
        executeStep();
    }
}

void Circuit::applyGate(const Gate& gate) {
    switch (gate.type) {
        case GateType::H:
            applyHadamard(gate.targetQubits[0]);
            break;
        case GateType::X:
            applyPauliX(gate.targetQubits[0]);
            break;
        case GateType::Y:
            applyPauliY(gate.targetQubits[0]);
            break;
        case GateType::Z:
            applyPauliZ(gate.targetQubits[0]);
            break;
        case GateType::CNOT:
            applyCNOT(gate.controlQubits[0], gate.targetQubits[0]);
            break;
        case GateType::SWAP:
            applySWAP(gate.targetQubits[0], gate.targetQubits[1]);
            break;
        case GateType::PHASE:
            applyPhaseShift(gate.targetQubits[0], gate.parameters[0]);
            break;
        case GateType::RX:
            applyRotationX(gate.targetQubits[0], gate.parameters[0]);
            break;
        case GateType::RY:
            applyRotationY(gate.targetQubits[0], gate.parameters[0]);
            break;
        case GateType::RZ:
            applyRotationZ(gate.targetQubits[0], gate.parameters[0]);
            break;
        case GateType::MEASURE:
            performMeasurement(gate.targetQubits[0], gate.controlQubits[0]);
            break;
        default:
            throw std::invalid_argument("Unknown gate type");
    }
}

void Circuit::applyHadamard(int qubit) {
    validateQubitIndex(qubit);
    
    MatrixXcd hadamardMatrix = createSingleQubitGate(Operators::HADAMARD, qubit);
    stateVector_ = hadamardMatrix * stateVector_;
}

void Circuit::applyPauliX(int qubit) {
    validateQubitIndex(qubit);
    
    MatrixXcd pauliXMatrix = createSingleQubitGate(Operators::PAULI_X, qubit);
    stateVector_ = pauliXMatrix * stateVector_;
}

void Circuit::applyPauliY(int qubit) {
    validateQubitIndex(qubit);
    
    MatrixXcd pauliYMatrix = createSingleQubitGate(Operators::PAULI_Y, qubit);
    stateVector_ = pauliYMatrix * stateVector_;
}

void Circuit::applyPauliZ(int qubit) {
    validateQubitIndex(qubit);
    
    MatrixXcd pauliZMatrix = createSingleQubitGate(Operators::PAULI_Z, qubit);
    stateVector_ = pauliZMatrix * stateVector_;
}

void Circuit::applyCNOT(int controlQubit, int targetQubit) {
    validateQubitIndex(controlQubit);
    validateQubitIndex(targetQubit);
    
    if (controlQubit == targetQubit) {
        throw std::invalid_argument("Control and target qubits must be different");
    }
    
    MatrixXcd cnotMatrix = createTwoQubitGate(Operators::CNOT, controlQubit, targetQubit);
    stateVector_ = cnotMatrix * stateVector_;
}

void Circuit::applySWAP(int qubit1, int qubit2) {
    validateQubitIndex(qubit1);
    validateQubitIndex(qubit2);
    
    if (qubit1 == qubit2) {
        return; // No operation needed
    }
    
    MatrixXcd swapMatrix = createTwoQubitGate(Operators::SWAP, qubit1, qubit2);
    stateVector_ = swapMatrix * stateVector_;
}

void Circuit::applyPhaseShift(int qubit, double angle) {
    validateQubitIndex(qubit);
    
    Matrix2cd phaseMatrix = Operators::phaseShift(angle);
    MatrixXcd fullMatrix = createSingleQubitGate(phaseMatrix, qubit);
    stateVector_ = fullMatrix * stateVector_;
}

void Circuit::applyRotationX(int qubit, double angle) {
    validateQubitIndex(qubit);
    
    Matrix2cd rotationMatrix = Operators::rotationX(angle);
    MatrixXcd fullMatrix = createSingleQubitGate(rotationMatrix, qubit);
    stateVector_ = fullMatrix * stateVector_;
}

void Circuit::applyRotationY(int qubit, double angle) {
    validateQubitIndex(qubit);
    
    Matrix2cd rotationMatrix = Operators::rotationY(angle);
    MatrixXcd fullMatrix = createSingleQubitGate(rotationMatrix, qubit);
    stateVector_ = fullMatrix * stateVector_;
}

void Circuit::applyRotationZ(int qubit, double angle) {
    validateQubitIndex(qubit);
    
    Matrix2cd rotationMatrix = Operators::rotationZ(angle);
    MatrixXcd fullMatrix = createSingleQubitGate(rotationMatrix, qubit);
    stateVector_ = fullMatrix * stateVector_;
}

void Circuit::performMeasurement(int qubit, int classicalBit) {
    validateQubitIndex(qubit);
    validateClassicalBitIndex(classicalBit);
    
    // Calculate measurement probabilities
    double prob0 = 0.0;
    double prob1 = 0.0;
    
    int dim = static_cast<int>(std::pow(2, numQubits_));
    int qubitMask = 1 << qubit;
    
    for (int i = 0; i < dim; ++i) {
        double amplitude = std::norm(stateVector_(i));
        if (i & qubitMask) {
            prob1 += amplitude;
        } else {
            prob0 += amplitude;
        }
    }
    
    // Normalize probabilities
    double total = prob0 + prob1;
    prob0 /= total;
    prob1 /= total;
    
    // Perform measurement (simplified - always measure |0⟩ for deterministic behavior)
    int measurementResult = 0; // For now, always measure |0⟩
    classicalBits_[classicalBit] = measurementResult;
    
    // Collapse the state vector
    for (int i = 0; i < dim; ++i) {
        if ((i & qubitMask) != (measurementResult << qubit)) {
            stateVector_(i) = 0.0;
        }
    }
    
    // Renormalize
    double norm = stateVector_.norm();
    if (norm > 1e-12) {
        stateVector_ /= norm;
    }
}

MatrixXcd Circuit::createSingleQubitGate(const Matrix2cd& gate, int qubit) {
    int dim = static_cast<int>(std::pow(2, numQubits_));
    MatrixXcd fullMatrix = MatrixXcd::Identity(dim, dim);
    
    // Create tensor product with identity matrices for other qubits
    for (int i = 0; i < numQubits_; ++i) {
        Matrix2cd currentGate = (i == qubit) ? gate : Operators::IDENTITY;
        
        if (i == 0) {
            fullMatrix = currentGate;
        } else {
            fullMatrix = Operators::tensorProduct(fullMatrix, currentGate);
        }
    }
    
    return fullMatrix;
}

MatrixXcd Circuit::createTwoQubitGate(const Matrix4cd& gate, int qubit1, int qubit2) {
    int dim = static_cast<int>(std::pow(2, numQubits_));
    MatrixXcd fullMatrix = MatrixXcd::Identity(dim, dim);
    
    // For simplicity, we'll create a direct implementation
    // This is a simplified version - in practice, you'd want a more efficient implementation
    
    // Create the full matrix by applying the gate to the appropriate qubits
    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j < dim; ++j) {
            // Extract the states of the two qubits
            int state1 = (i >> qubit1) & 1;
            int state2 = (i >> qubit2) & 1;
            int newState1 = (j >> qubit1) & 1;
            int newState2 = (j >> qubit2) & 1;
            
            // Check if other qubits are the same
            bool otherQubitsMatch = true;
            for (int k = 0; k < numQubits_; ++k) {
                if (k != qubit1 && k != qubit2) {
                    if (((i >> k) & 1) != ((j >> k) & 1)) {
                        otherQubitsMatch = false;
                        break;
                    }
                }
            }
            
            if (otherQubitsMatch) {
                int gateRow = state1 * 2 + state2;
                int gateCol = newState1 * 2 + newState2;
                fullMatrix(i, j) = gate(gateRow, gateCol);
            } else {
                fullMatrix(i, j) = 0.0;
            }
        }
    }
    
    return fullMatrix;
}

void Circuit::validateGate(const Gate& gate) {
    // Validate target qubits
    for (int qubit : gate.targetQubits) {
        validateQubitIndex(qubit);
    }
    
    // Validate control qubits
    for (int qubit : gate.controlQubits) {
        validateQubitIndex(qubit);
    }
    
    // Check for duplicate qubits
    std::vector<int> allQubits;
    allQubits.insert(allQubits.end(), gate.controlQubits.begin(), gate.controlQubits.end());
    allQubits.insert(allQubits.end(), gate.targetQubits.begin(), gate.targetQubits.end());
    
    std::sort(allQubits.begin(), allQubits.end());
    auto it = std::adjacent_find(allQubits.begin(), allQubits.end());
    if (it != allQubits.end()) {
        throw std::invalid_argument("Duplicate qubit indices in gate");
    }
}

void Circuit::validateQubitIndex(int qubit) {
    if (qubit < 0 || qubit >= numQubits_) {
        throw std::out_of_range("Qubit index out of range");
    }
}

void Circuit::validateClassicalBitIndex(int bit) {
    if (bit < 0 || bit >= numClassicalBits_) {
        throw std::out_of_range("Classical bit index out of range");
    }
}

std::string Circuit::toQASM() const {
    std::stringstream qasm;
    qasm << "OPENQASM 2.0;\n";
    qasm << "include \"qelib1.inc\";\n\n";
    qasm << "qreg q[" << numQubits_ << "];\n";
    qasm << "creg c[" << numClassicalBits_ << "];\n\n";
    
    for (const auto& gate : gates_) {
        qasm << gateToString(gate) << "\n";
    }
    
    return qasm.str();
}

std::string Circuit::gateToString(const Gate& gate) const {
    std::stringstream ss;
    
    switch (gate.type) {
        case GateType::H:
            ss << "h q[" << gate.targetQubits[0] << "]";
            break;
        case GateType::X:
            ss << "x q[" << gate.targetQubits[0] << "]";
            break;
        case GateType::Y:
            ss << "y q[" << gate.targetQubits[0] << "]";
            break;
        case GateType::Z:
            ss << "z q[" << gate.targetQubits[0] << "]";
            break;
        case GateType::CNOT:
            ss << "cx q[" << gate.controlQubits[0] << "], q[" << gate.targetQubits[0] << "]";
            break;
        case GateType::SWAP:
            ss << "swap q[" << gate.targetQubits[0] << "], q[" << gate.targetQubits[1] << "]";
            break;
        case GateType::PHASE:
            ss << "u1(" << std::fixed << std::setprecision(6) << gate.parameters[0] 
               << ") q[" << gate.targetQubits[0] << "]";
            break;
        case GateType::RX:
            ss << "rx(" << std::fixed << std::setprecision(6) << gate.parameters[0] 
               << ") q[" << gate.targetQubits[0] << "]";
            break;
        case GateType::RY:
            ss << "ry(" << std::fixed << std::setprecision(6) << gate.parameters[0] 
               << ") q[" << gate.targetQubits[0] << "]";
            break;
        case GateType::RZ:
            ss << "rz(" << std::fixed << std::setprecision(6) << gate.parameters[0] 
               << ") q[" << gate.targetQubits[0] << "]";
            break;
        case GateType::MEASURE:
            ss << "measure q[" << gate.targetQubits[0] << "] -> c[" << gate.controlQubits[0] << "]";
            break;
        default:
            ss << "// Unknown gate type";
            break;
    }
    
    return ss.str();
}

} // namespace omniq
