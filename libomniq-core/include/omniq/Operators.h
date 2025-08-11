//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef OMNIQ_OPERATORS_H
#define OMNIQ_OPERATORS_H

#include <Eigen/Dense>
#include <vector>
#include <complex>

namespace omniq {

using Matrix2cd = Eigen::Matrix2cd;
using Matrix4cd = Eigen::Matrix4cd;
using MatrixXcd = Eigen::MatrixXcd;
using VectorXcd = Eigen::VectorXcd;

class Operators {
public:
    // Standard quantum gates
    static const Matrix2cd PAULI_X;
    static const Matrix2cd PAULI_Y;
    static const Matrix2cd PAULI_Z;
    static const Matrix2cd IDENTITY;
    static const Matrix2cd HADAMARD;
    static const Matrix2cd S_GATE;
    static const Matrix2cd T_GATE;
    
    // Two-qubit gates
    static const Matrix4cd CNOT;
    static const Matrix4cd SWAP;
    
    // Rotation gates
    static Matrix2cd rotationX(double theta);
    static Matrix2cd rotationY(double theta);
    static Matrix2cd rotationZ(double theta);
    static Matrix2cd phaseShift(double phi);
    
    // Controlled gates
    static Matrix4cd controlledGate(const Matrix2cd& gate);
    static Matrix4cd controlledX();
    static Matrix4cd controlledY();
    static Matrix4cd controlledZ();
    static Matrix4cd controlledPhase(double phi);
    static Matrix4cd controlledRotationX(double theta);
    static Matrix4cd controlledRotationY(double theta);
    static Matrix4cd controlledRotationZ(double theta);
    
    // Multi-qubit operations
    static MatrixXcd tensorProduct(const MatrixXcd& A, const MatrixXcd& B);
    static MatrixXcd createMultiQubitGate(const std::vector<Matrix2cd>& gates);
    
    // Measurement operators
    static Matrix2cd measurementOperator(int basis, int outcome);
    
    // Utility functions
    static bool isUnitary(const MatrixXcd& matrix);
    static bool isHermitian(const MatrixXcd& matrix);
    static double trace(const MatrixXcd& matrix);
    static MatrixXcd partialTrace(const MatrixXcd& matrix, int qubit, int totalQubits);
};

} // namespace omniq

#endif // OMNIQ_OPERATORS_H 