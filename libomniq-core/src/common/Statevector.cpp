//
// Created by Goutham Arcot on 17/07/25.
//

#include "omniq/Statevector.h"
#include "omniq/Operators.h"
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace omniq {

Statevector::Statevector(int numQubits)
    : numQubits_(numQubits)
    , stateVector_(static_cast<int>(std::pow(2, numQubits)))
{
    if (numQubits <= 0) {
        throw std::invalid_argument("Number of qubits must be positive");
    }
    
    // Initialize to |0...0⟩ state
    stateVector_.setZero();
    stateVector_(0) = 1.0;
}

Statevector::Statevector(const VectorXcd& amplitudes)
    : numQubits_(static_cast<int>(std::log2(amplitudes.size())))
    , stateVector_(amplitudes)
{
    if (amplitudes.size() == 0 || (amplitudes.size() & (amplitudes.size() - 1)) != 0) {
        throw std::invalid_argument("State vector size must be a power of 2");
    }
    
    normalize();
}

Statevector::Statevector(const Statevector& other)
    : numQubits_(other.numQubits_)
    , stateVector_(other.stateVector_)
{
}

Statevector& Statevector::operator=(const Statevector& other) {
    if (this != &other) {
        numQubits_ = other.numQubits_;
        stateVector_ = other.stateVector_;
    }
    return *this;
}

void Statevector::normalize() {
    double norm = stateVector_.norm();
    if (norm > 1e-12) {
        stateVector_ /= norm;
    }
}

double Statevector::getNorm() const {
    return stateVector_.norm();
}

std::string Statevector::toString() const {
    std::stringstream ss;
    ss << "|ψ⟩ = ";
    
    bool first = true;
    for (int i = 0; i < stateVector_.size(); ++i) {
        std::complex<double> amplitude = stateVector_(i);
        if (std::abs(amplitude) > 1e-12) {
            if (!first) {
                ss << " + ";
            }
            
            // Convert index to binary representation
            std::string binary = "";
            int temp = i;
            for (int j = 0; j < numQubits_; ++j) {
                binary = (temp % 2 == 0 ? "0" : "1") + binary;
                temp /= 2;
            }
            
            ss << "(" << std::fixed << std::setprecision(6) << amplitude.real();
            if (amplitude.imag() >= 0) {
                ss << "+" << amplitude.imag() << "i";
            } else {
                ss << amplitude.imag() << "i";
            }
            ss << ")|" << binary << "⟩";
            
            first = false;
        }
    }
    
    return ss.str();
}

void Statevector::applyHadamard(int qubit) {
    validateQubitIndex(qubit);
    
    MatrixXcd hadamardMatrix = createSingleQubitGate(Operators::HADAMARD, qubit);
    stateVector_ = hadamardMatrix * stateVector_;
}

void Statevector::applyCNOT(int control, int target) {
    validateQubitIndex(control);
    validateQubitIndex(target);
    
    if (control == target) {
        throw std::invalid_argument("Control and target qubits must be different");
    }
    
    MatrixXcd cnotMatrix = createTwoQubitGate(Operators::CNOT, control, target);
    stateVector_ = cnotMatrix * stateVector_;
}

void Statevector::applyPauliX(int qubit) {
    validateQubitIndex(qubit);
    
    MatrixXcd pauliXMatrix = createSingleQubitGate(Operators::PAULI_X, qubit);
    stateVector_ = pauliXMatrix * stateVector_;
}

void Statevector::applyPauliY(int qubit) {
    validateQubitIndex(qubit);
    
    MatrixXcd pauliYMatrix = createSingleQubitGate(Operators::PAULI_Y, qubit);
    stateVector_ = pauliYMatrix * stateVector_;
}

void Statevector::applyPauliZ(int qubit) {
    validateQubitIndex(qubit);
    
    MatrixXcd pauliZMatrix = createSingleQubitGate(Operators::PAULI_Z, qubit);
    stateVector_ = pauliZMatrix * stateVector_;
}

void Statevector::applyPhaseShift(int qubit, double angle) {
    validateQubitIndex(qubit);
    
    Matrix2cd phaseMatrix = Operators::phaseShift(angle);
    MatrixXcd fullMatrix = createSingleQubitGate(phaseMatrix, qubit);
    stateVector_ = fullMatrix * stateVector_;
}

void Statevector::applyRotationX(int qubit, double angle) {
    validateQubitIndex(qubit);
    
    Matrix2cd rotationMatrix = Operators::rotationX(angle);
    MatrixXcd fullMatrix = createSingleQubitGate(rotationMatrix, qubit);
    stateVector_ = fullMatrix * stateVector_;
}

void Statevector::applyRotationY(int qubit, double angle) {
    validateQubitIndex(qubit);
    
    Matrix2cd rotationMatrix = Operators::rotationY(angle);
    MatrixXcd fullMatrix = createSingleQubitGate(rotationMatrix, qubit);
    stateVector_ = fullMatrix * stateVector_;
}

void Statevector::applyRotationZ(int qubit, double angle) {
    validateQubitIndex(qubit);
    
    Matrix2cd rotationMatrix = Operators::rotationZ(angle);
    MatrixXcd fullMatrix = createSingleQubitGate(rotationMatrix, qubit);
    stateVector_ = fullMatrix * stateVector_;
}

int Statevector::measure(int qubit) {
    validateQubitIndex(qubit);
    
    // Calculate measurement probabilities
    double prob0 = 0.0;
    double prob1 = 0.0;
    
    int qubitMask = 1 << qubit;
    
    for (int i = 0; i < stateVector_.size(); ++i) {
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
    
    // For deterministic behavior, always return 0
    // In a real implementation, you would use a random number generator
    int measurementResult = 0;
    
    // Collapse the state vector
    for (int i = 0; i < stateVector_.size(); ++i) {
        if ((i & qubitMask) != (measurementResult << qubit)) {
            stateVector_(i) = 0.0;
        }
    }
    
    // Renormalize
    normalize();
    
    return measurementResult;
}

double Statevector::measureExpectation(int qubit, const std::string& observable) {
    validateQubitIndex(qubit);
    
    Matrix2cd obsMatrix;
    if (observable == "X") {
        obsMatrix = Operators::PAULI_X;
    } else if (observable == "Y") {
        obsMatrix = Operators::PAULI_Y;
    } else if (observable == "Z") {
        obsMatrix = Operators::PAULI_Z;
    } else {
        throw std::invalid_argument("Unknown observable: " + observable);
    }
    
    MatrixXcd fullMatrix = createSingleQubitGate(obsMatrix, qubit);
    std::complex<double> expectation = stateVector_.adjoint() * fullMatrix * stateVector_;
    
    return expectation.real();
}

MatrixXcd Statevector::createSingleQubitGate(const Matrix2cd& gate, int qubit) const {
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

MatrixXcd Statevector::createTwoQubitGate(const Matrix4cd& gate, int qubit1, int qubit2) const {
    int dim = static_cast<int>(std::pow(2, numQubits_));
    MatrixXcd fullMatrix = MatrixXcd::Identity(dim, dim);
    
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

void Statevector::validateQubitIndex(int qubit) const {
    if (qubit < 0 || qubit >= numQubits_) {
        throw std::out_of_range("Qubit index out of range");
    }
}

Statevector Statevector::tensorProduct(const Statevector& other) const {
    int newNumQubits = numQubits_ + other.numQubits_;
    VectorXcd newStateVector = Operators::tensorProduct(stateVector_, other.stateVector_);
    
    Statevector result(newNumQubits);
    result.stateVector_ = newStateVector;
    
    return result;
}

MatrixXcd Statevector::partialTrace(const std::vector<int>& qubitsToTrace) const {
    // Sort qubits to trace in descending order to avoid index shifting
    std::vector<int> sortedQubits = qubitsToTrace;
    std::sort(sortedQubits.begin(), sortedQubits.end(), std::greater<int>());
    
    // Validate qubit indices
    for (int qubit : sortedQubits) {
        if (qubit < 0 || qubit >= numQubits_) {
            throw std::out_of_range("Qubit index out of range for partial trace");
        }
    }
    
    // Create density matrix
    MatrixXcd densityMatrix = stateVector_ * stateVector_.adjoint();
    
    // Perform partial trace
    for (int qubit : sortedQubits) {
        densityMatrix = Operators::partialTrace(densityMatrix, qubit, numQubits_);
    }
    
    return densityMatrix;
}

double Statevector::getQubitProbability(int qubit, int value) const {
    validateQubitIndex(qubit);
    
    if (value != 0 && value != 1) {
        throw std::invalid_argument("Qubit value must be 0 or 1");
    }
    
    double probability = 0.0;
    int qubitMask = 1 << qubit;
    
    for (int i = 0; i < stateVector_.size(); ++i) {
        if (((i >> qubit) & 1) == value) {
            probability += std::norm(stateVector_(i));
        }
    }
    
    return probability;
}

std::complex<double> Statevector::getQubitAmplitude(int qubit, int value) const {
    validateQubitIndex(qubit);
    
    if (value != 0 && value != 1) {
        throw std::invalid_argument("Qubit value must be 0 or 1");
    }
    
    std::complex<double> amplitude = 0.0;
    int qubitMask = 1 << qubit;
    
    for (int i = 0; i < stateVector_.size(); ++i) {
        if (((i >> qubit) & 1) == value) {
            amplitude += stateVector_(i);
        }
    }
    
    return amplitude;
}

} // namespace omniq
