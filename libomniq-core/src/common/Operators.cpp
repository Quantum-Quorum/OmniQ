//
// Created by Goutham Arcot on 17/07/25.
//

#include "omniq/Operators.h"
#include <cmath>
#include <complex>
#include <stdexcept>

namespace omniq {

// Pauli matrices
const Matrix2cd Operators::PAULI_X = (Matrix2cd() << 
    std::complex<double>(0, 0), std::complex<double>(1, 0),
    std::complex<double>(1, 0), std::complex<double>(0, 0)).finished();

const Matrix2cd Operators::PAULI_Y = (Matrix2cd() << 
    std::complex<double>(0, 0), std::complex<double>(0, -1),
    std::complex<double>(0, 1), std::complex<double>(0, 0)).finished();

const Matrix2cd Operators::PAULI_Z = (Matrix2cd() << 
    std::complex<double>(1, 0), std::complex<double>(0, 0),
    std::complex<double>(0, 0), std::complex<double>(-1, 0)).finished();

const Matrix2cd Operators::IDENTITY = (Matrix2cd() << 
    std::complex<double>(1, 0), std::complex<double>(0, 0),
    std::complex<double>(0, 0), std::complex<double>(1, 0)).finished();

// Hadamard gate
const Matrix2cd Operators::HADAMARD = (Matrix2cd() << 
    std::complex<double>(1.0/std::sqrt(2), 0), std::complex<double>(1.0/std::sqrt(2), 0),
    std::complex<double>(1.0/std::sqrt(2), 0), std::complex<double>(-1.0/std::sqrt(2), 0)).finished();

// Phase gates
const Matrix2cd Operators::S_GATE = (Matrix2cd() << 
    std::complex<double>(1, 0), std::complex<double>(0, 0),
    std::complex<double>(0, 0), std::complex<double>(0, 1)).finished();

const Matrix2cd Operators::T_GATE = (Matrix2cd() << 
    std::complex<double>(1, 0), std::complex<double>(0, 0),
    std::complex<double>(0, 0), std::complex<double>(std::cos(M_PI/4), std::sin(M_PI/4))).finished();

// CNOT gate (4x4 matrix)
const Matrix4cd Operators::CNOT = (Matrix4cd() << 
    std::complex<double>(1, 0), std::complex<double>(0, 0), std::complex<double>(0, 0), std::complex<double>(0, 0),
    std::complex<double>(0, 0), std::complex<double>(1, 0), std::complex<double>(0, 0), std::complex<double>(0, 0),
    std::complex<double>(0, 0), std::complex<double>(0, 0), std::complex<double>(0, 0), std::complex<double>(1, 0),
    std::complex<double>(0, 0), std::complex<double>(0, 0), std::complex<double>(1, 0), std::complex<double>(0, 0)).finished();

// SWAP gate (4x4 matrix)
const Matrix4cd Operators::SWAP = (Matrix4cd() << 
    std::complex<double>(1, 0), std::complex<double>(0, 0), std::complex<double>(0, 0), std::complex<double>(0, 0),
    std::complex<double>(0, 0), std::complex<double>(0, 0), std::complex<double>(1, 0), std::complex<double>(0, 0),
    std::complex<double>(0, 0), std::complex<double>(1, 0), std::complex<double>(0, 0), std::complex<double>(0, 0),
    std::complex<double>(0, 0), std::complex<double>(0, 0), std::complex<double>(0, 0), std::complex<double>(1, 0)).finished();

Matrix2cd Operators::rotationX(double theta) {
    double cos_half = std::cos(theta / 2.0);
    double sin_half = std::sin(theta / 2.0);
    
    return (Matrix2cd() << 
        std::complex<double>(cos_half, 0), std::complex<double>(0, -sin_half),
        std::complex<double>(0, -sin_half), std::complex<double>(cos_half, 0)).finished();
}

Matrix2cd Operators::rotationY(double theta) {
    double cos_half = std::cos(theta / 2.0);
    double sin_half = std::sin(theta / 2.0);
    
    return (Matrix2cd() << 
        std::complex<double>(cos_half, 0), std::complex<double>(-sin_half, 0),
        std::complex<double>(sin_half, 0), std::complex<double>(cos_half, 0)).finished();
}

Matrix2cd Operators::rotationZ(double theta) {
    double cos_half = std::cos(theta / 2.0);
    double sin_half = std::sin(theta / 2.0);
    
    return (Matrix2cd() << 
        std::complex<double>(cos_half, -sin_half), std::complex<double>(0, 0),
        std::complex<double>(0, 0), std::complex<double>(cos_half, sin_half)).finished();
}

Matrix2cd Operators::phaseShift(double phi) {
    return (Matrix2cd() << 
        std::complex<double>(1, 0), std::complex<double>(0, 0),
        std::complex<double>(0, 0), std::complex<double>(std::cos(phi), std::sin(phi))).finished();
}

Matrix4cd Operators::controlledGate(const Matrix2cd& gate) {
    Matrix4cd result = Matrix4cd::Identity();
    
    // Apply the gate to the target qubit when control qubit is |1⟩
    result.block<2,2>(2,2) = gate;
    
    return result;
}

Matrix4cd Operators::controlledX() {
    return CNOT;
}

Matrix4cd Operators::controlledY() {
    return controlledGate(PAULI_Y);
}

Matrix4cd Operators::controlledZ() {
    return controlledGate(PAULI_Z);
}

Matrix4cd Operators::controlledPhase(double phi) {
    return controlledGate(phaseShift(phi));
}

Matrix4cd Operators::controlledRotationX(double theta) {
    return controlledGate(rotationX(theta));
}

Matrix4cd Operators::controlledRotationY(double theta) {
    return controlledGate(rotationY(theta));
}

Matrix4cd Operators::controlledRotationZ(double theta) {
    return controlledGate(rotationZ(theta));
}

// Multi-qubit operations
MatrixXcd Operators::tensorProduct(const MatrixXcd& A, const MatrixXcd& B) {
    int rows_A = A.rows();
    int cols_A = A.cols();
    int rows_B = B.rows();
    int cols_B = B.cols();
    
    MatrixXcd result(rows_A * rows_B, cols_A * cols_B);
    
    for (int i = 0; i < rows_A; ++i) {
        for (int j = 0; j < cols_A; ++j) {
            result.block(i * rows_B, j * cols_B, rows_B, cols_B) = A(i, j) * B;
        }
    }
    
    return result;
}

MatrixXcd Operators::createMultiQubitGate(const std::vector<Matrix2cd>& gates) {
    if (gates.empty()) {
        throw std::invalid_argument("No gates provided for multi-qubit operation");
    }
    
    MatrixXcd result = gates[0];
    for (size_t i = 1; i < gates.size(); ++i) {
        result = tensorProduct(result, gates[i]);
    }
    
    return result;
}

// Measurement operators
Matrix2cd Operators::measurementOperator(int basis, int outcome) {
    switch (basis) {
        case 0: // Computational basis (Z)
            if (outcome == 0) {
                return (Matrix2cd() << 
                    std::complex<double>(1, 0), std::complex<double>(0, 0),
                    std::complex<double>(0, 0), std::complex<double>(0, 0)).finished();
            } else {
                return (Matrix2cd() << 
                    std::complex<double>(0, 0), std::complex<double>(0, 0),
                    std::complex<double>(0, 0), std::complex<double>(1, 0)).finished();
            }
        case 1: // X basis
            if (outcome == 0) {
                return (Matrix2cd() << 
                    std::complex<double>(0.5, 0), std::complex<double>(0.5, 0),
                    std::complex<double>(0.5, 0), std::complex<double>(0.5, 0)).finished();
            } else {
                return (Matrix2cd() << 
                    std::complex<double>(0.5, 0), std::complex<double>(-0.5, 0),
                    std::complex<double>(-0.5, 0), std::complex<double>(0.5, 0)).finished();
            }
        case 2: // Y basis
            if (outcome == 0) {
                return (Matrix2cd() << 
                    std::complex<double>(0.5, 0), std::complex<double>(0, -0.5),
                    std::complex<double>(0, 0.5), std::complex<double>(0.5, 0)).finished();
            } else {
                return (Matrix2cd() << 
                    std::complex<double>(0.5, 0), std::complex<double>(0, 0.5),
                    std::complex<double>(0, -0.5), std::complex<double>(0.5, 0)).finished();
            }
        default:
            throw std::invalid_argument("Invalid measurement basis");
    }
}

// Utility functions
bool Operators::isUnitary(const MatrixXcd& matrix) {
    MatrixXcd conjugate = matrix.adjoint();
    MatrixXcd product = matrix * conjugate;
    MatrixXcd identity = MatrixXcd::Identity(matrix.rows(), matrix.cols());
    
    return (product - identity).norm() < 1e-10;
}

bool Operators::isHermitian(const MatrixXcd& matrix) {
    return (matrix - matrix.adjoint()).norm() < 1e-10;
}

double Operators::trace(const MatrixXcd& matrix) {
    return matrix.trace().real();
}

MatrixXcd Operators::partialTrace(const MatrixXcd& matrix, int qubit, int totalQubits) {
    if (qubit >= totalQubits) {
        throw std::invalid_argument("Qubit index out of range");
    }
    
    int dim = matrix.rows();
    int dimPerQubit = static_cast<int>(std::pow(2, totalQubits - 1));
    MatrixXcd result = MatrixXcd::Zero(dimPerQubit, dimPerQubit);
    
    // Perform partial trace over the specified qubit
    for (int i = 0; i < dimPerQubit; ++i) {
        for (int j = 0; j < dimPerQubit; ++j) {
            std::complex<double> sum(0, 0);
            for (int k = 0; k < 2; ++k) {
                int row = i + k * dimPerQubit;
                int col = j + k * dimPerQubit;
                sum += matrix(row, col);
            }
            result(i, j) = sum;
        }
    }
    
    return result;
}

} // namespace omniq
