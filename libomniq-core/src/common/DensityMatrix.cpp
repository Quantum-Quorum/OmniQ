//
// Created by Goutham Arcot on 17/07/25.
//

#include "omniq/DensityMatrix.h"
#include "omniq/Operators.h"
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace omniq {

DensityMatrix::DensityMatrix(int numQubits)
    : numQubits_(numQubits)
    , densityMatrix_(static_cast<int>(std::pow(2, numQubits)), static_cast<int>(std::pow(2, numQubits)))
{
    if (numQubits <= 0) {
        throw std::invalid_argument("Number of qubits must be positive");
    }
    
    // Initialize to |0...0⟩⟨0...0| state
    densityMatrix_.setZero();
    densityMatrix_(0, 0) = 1.0;
}

DensityMatrix::DensityMatrix(const MatrixXcd& matrix)
    : numQubits_(static_cast<int>(std::log2(matrix.rows())))
    , densityMatrix_(matrix)
{
    if (matrix.rows() != matrix.cols() || matrix.rows() == 0 || 
        (matrix.rows() & (matrix.rows() - 1)) != 0) {
        throw std::invalid_argument("Density matrix must be square with size power of 2");
    }
    
    normalize();
}

DensityMatrix::DensityMatrix(const Statevector& statevector)
    : numQubits_(statevector.getNumQubits())
    , densityMatrix_(statevector.getStateVector() * statevector.getStateVector().adjoint())
{
}

DensityMatrix::DensityMatrix(const DensityMatrix& other)
    : numQubits_(other.numQubits_)
    , densityMatrix_(other.densityMatrix_)
{
}

DensityMatrix& DensityMatrix::operator=(const DensityMatrix& other) {
    if (this != &other) {
        numQubits_ = other.numQubits_;
        densityMatrix_ = other.densityMatrix_;
    }
    return *this;
}

void DensityMatrix::normalize() {
    double trace = densityMatrix_.trace().real();
    if (trace > 1e-12) {
        densityMatrix_ /= trace;
    }
}

double DensityMatrix::getTrace() const {
    return densityMatrix_.trace().real();
}

double DensityMatrix::getPurity() const {
    MatrixXcd squared = densityMatrix_ * densityMatrix_;
    return squared.trace().real();
}

double DensityMatrix::getVonNeumannEntropy() const {
    Eigen::SelfAdjointEigenSolver<MatrixXcd> solver(densityMatrix_);
    const VectorXcd& eigenvalues = solver.eigenvalues();
    
    double entropy = 0.0;
    for (int i = 0; i < eigenvalues.size(); ++i) {
        double lambda = eigenvalues(i).real();
        if (lambda > 1e-12) {
            entropy -= lambda * std::log2(lambda);
        }
    }
    
    return entropy;
}

std::string DensityMatrix::toString() const {
    std::stringstream ss;
    ss << "Density Matrix (" << numQubits_ << " qubits):\n";
    ss << std::fixed << std::setprecision(6);
    
    for (int i = 0; i < densityMatrix_.rows(); ++i) {
        for (int j = 0; j < densityMatrix_.cols(); ++j) {
            std::complex<double> element = densityMatrix_(i, j);
            if (std::abs(element) > 1e-12) {
                ss << "(" << element.real();
                if (element.imag() >= 0) {
                    ss << "+" << element.imag() << "i";
                } else {
                    ss << element.imag() << "i";
                }
                ss << ") ";
            } else {
                ss << "(0.000000+0.000000i) ";
            }
        }
        ss << "\n";
    }
    
    return ss.str();
}

void DensityMatrix::applyHadamard(int qubit) {
    validateQubitIndex(qubit);
    
    MatrixXcd hadamardMatrix = createSingleQubitGate(Operators::HADAMARD, qubit);
    densityMatrix_ = hadamardMatrix * densityMatrix_ * hadamardMatrix.adjoint();
}

void DensityMatrix::applyCNOT(int control, int target) {
    validateQubitIndex(control);
    validateQubitIndex(target);
    
    if (control == target) {
        throw std::invalid_argument("Control and target qubits must be different");
    }
    
    MatrixXcd cnotMatrix = createTwoQubitGate(Operators::CNOT, control, target);
    densityMatrix_ = cnotMatrix * densityMatrix_ * cnotMatrix.adjoint();
}

void DensityMatrix::applyPauliX(int qubit) {
    validateQubitIndex(qubit);
    
    MatrixXcd pauliXMatrix = createSingleQubitGate(Operators::PAULI_X, qubit);
    densityMatrix_ = pauliXMatrix * densityMatrix_ * pauliXMatrix.adjoint();
}

void DensityMatrix::applyPauliY(int qubit) {
    validateQubitIndex(qubit);
    
    MatrixXcd pauliYMatrix = createSingleQubitGate(Operators::PAULI_Y, qubit);
    densityMatrix_ = pauliYMatrix * densityMatrix_ * pauliYMatrix.adjoint();
}

void DensityMatrix::applyPauliZ(int qubit) {
    validateQubitIndex(qubit);
    
    MatrixXcd pauliZMatrix = createSingleQubitGate(Operators::PAULI_Z, qubit);
    densityMatrix_ = pauliZMatrix * densityMatrix_ * pauliZMatrix.adjoint();
}

void DensityMatrix::applyPhaseShift(int qubit, double angle) {
    validateQubitIndex(qubit);
    
    Matrix2cd phaseMatrix = Operators::phaseShift(angle);
    MatrixXcd fullMatrix = createSingleQubitGate(phaseMatrix, qubit);
    densityMatrix_ = fullMatrix * densityMatrix_ * fullMatrix.adjoint();
}

void DensityMatrix::applyRotationX(int qubit, double angle) {
    validateQubitIndex(qubit);
    
    Matrix2cd rotationMatrix = Operators::rotationX(angle);
    MatrixXcd fullMatrix = createSingleQubitGate(rotationMatrix, qubit);
    densityMatrix_ = fullMatrix * densityMatrix_ * fullMatrix.adjoint();
}

void DensityMatrix::applyRotationY(int qubit, double angle) {
    validateQubitIndex(qubit);
    
    Matrix2cd rotationMatrix = Operators::rotationY(angle);
    MatrixXcd fullMatrix = createSingleQubitGate(rotationMatrix, qubit);
    densityMatrix_ = fullMatrix * densityMatrix_ * fullMatrix.adjoint();
}

void DensityMatrix::applyRotationZ(int qubit, double angle) {
    validateQubitIndex(qubit);
    
    Matrix2cd rotationMatrix = Operators::rotationZ(angle);
    MatrixXcd fullMatrix = createSingleQubitGate(rotationMatrix, qubit);
    densityMatrix_ = fullMatrix * densityMatrix_ * fullMatrix.adjoint();
}

double DensityMatrix::measureExpectation(int qubit, const std::string& observable) {
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
    std::complex<double> expectation = (fullMatrix * densityMatrix_).trace();
    
    return expectation.real();
}

MatrixXcd DensityMatrix::createSingleQubitGate(const Matrix2cd& gate, int qubit) const {
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

MatrixXcd DensityMatrix::createTwoQubitGate(const Matrix4cd& gate, int qubit1, int qubit2) const {
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

void DensityMatrix::validateQubitIndex(int qubit) const {
    if (qubit < 0 || qubit >= numQubits_) {
        throw std::out_of_range("Qubit index out of range");
    }
}

DensityMatrix DensityMatrix::tensorProduct(const DensityMatrix& other) const {
    int newNumQubits = numQubits_ + other.numQubits_;
    MatrixXcd newDensityMatrix = Operators::tensorProduct(densityMatrix_, other.densityMatrix_);
    
    DensityMatrix result(newNumQubits);
    result.densityMatrix_ = newDensityMatrix;
    
    return result;
}

MatrixXcd DensityMatrix::partialTrace(const std::vector<int>& qubitsToTrace) const {
    // Sort qubits to trace in descending order to avoid index shifting
    std::vector<int> sortedQubits = qubitsToTrace;
    std::sort(sortedQubits.begin(), sortedQubits.end(), std::greater<int>());
    
    // Validate qubit indices
    for (int qubit : sortedQubits) {
        if (qubit < 0 || qubit >= numQubits_) {
            throw std::out_of_range("Qubit index out of range for partial trace");
        }
    }
    
    MatrixXcd result = densityMatrix_;
    
    // Perform partial trace
    for (int qubit : sortedQubits) {
        result = Operators::partialTrace(result, qubit, numQubits_);
    }
    
    return result;
}

double DensityMatrix::getQubitProbability(int qubit, int value) const {
    validateQubitIndex(qubit);
    
    if (value != 0 && value != 1) {
        throw std::invalid_argument("Qubit value must be 0 or 1");
    }
    
    // Create projection operator for the measurement
    Matrix2cd projection;
    if (value == 0) {
        projection = (Matrix2cd() << 1.0, 0.0, 0.0, 0.0).finished();
    } else {
        projection = (Matrix2cd() << 0.0, 0.0, 0.0, 1.0).finished();
    }
    
    MatrixXcd fullProjection = createSingleQubitGate(projection, qubit);
    std::complex<double> probability = (fullProjection * densityMatrix_).trace();
    
    return probability.real();
}

bool DensityMatrix::isPure() const {
    double purity = getPurity();
    return std::abs(purity - 1.0) < 1e-12;
}

bool DensityMatrix::isMixed() const {
    return !isPure();
}

bool DensityMatrix::isValid() const {
    // Check if trace is 1
    double trace = getTrace();
    if (std::abs(trace - 1.0) > 1e-12) {
        return false;
    }
    
    // Check if Hermitian
    MatrixXcd difference = densityMatrix_ - densityMatrix_.adjoint();
    if (difference.norm() > 1e-12) {
        return false;
    }
    
    // Check if positive semidefinite (all eigenvalues >= 0)
    Eigen::SelfAdjointEigenSolver<MatrixXcd> solver(densityMatrix_);
    const VectorXcd& eigenvalues = solver.eigenvalues();
    
    for (int i = 0; i < eigenvalues.size(); ++i) {
        if (eigenvalues(i).real() < -1e-12) {
            return false;
        }
    }
    
    return true;
}

} // namespace omniq
