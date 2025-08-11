//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef OMNIQ_STATEVECTOR_H
#define OMNIQ_STATEVECTOR_H

#include <Eigen/Dense>
#include <vector>
#include <string>
#include <complex>

namespace omniq {

using Matrix2cd = Eigen::Matrix2cd;
using Matrix4cd = Eigen::Matrix4cd;
using MatrixXcd = Eigen::MatrixXcd;
using VectorXcd = Eigen::VectorXcd;

class Statevector {
private:
    int numQubits_;
    VectorXcd stateVector_;

    // Helper methods
    void validateQubitIndex(int qubit) const;
    MatrixXcd createSingleQubitGate(const Matrix2cd& gate, int qubit) const;
    MatrixXcd createTwoQubitGate(const Matrix4cd& gate, int qubit1, int qubit2) const;

public:
    // Constructors
    explicit Statevector(int numQubits);
    explicit Statevector(const VectorXcd& amplitudes);
    Statevector(const Statevector& other);
    Statevector& operator=(const Statevector& other);

    // State operations
    void normalize();
    double getNorm() const;
    std::string toString() const;

    // Quantum operations
    void applyHadamard(int qubit);
    void applyCNOT(int control, int target);
    void applyPauliX(int qubit);
    void applyPauliY(int qubit);
    void applyPauliZ(int qubit);
    void applyPhaseShift(int qubit, double angle);
    void applyRotationX(int qubit, double angle);
    void applyRotationY(int qubit, double angle);
    void applyRotationZ(int qubit, double angle);

    // Measurements
    int measure(int qubit);
    double measureExpectation(int qubit, const std::string& observable);

    // State analysis
    Statevector tensorProduct(const Statevector& other) const;
    MatrixXcd partialTrace(const std::vector<int>& qubitsToTrace) const;
    double getQubitProbability(int qubit, int value) const;
    std::complex<double> getQubitAmplitude(int qubit, int value) const;

    // Accessors
    int getNumQubits() const { return numQubits_; }
    const VectorXcd& getStateVector() const { return stateVector_; }
    VectorXcd& getStateVector() { return stateVector_; }
};

} // namespace omniq

#endif // OMNIQ_STATEVECTOR_H 