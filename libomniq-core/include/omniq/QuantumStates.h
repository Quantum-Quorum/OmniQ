//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef OMNIQ_QUANTUMSTATES_H
#define OMNIQ_QUANTUMSTATES_H

#include <Eigen/Dense>
#include <vector>
#include <complex>
#include <random>

namespace omniq {

using MatrixXcd = Eigen::MatrixXcd;
using VectorXcd = Eigen::VectorXcd;

// Quantum state analysis functions
double calculatePurity(const MatrixXcd& densityMatrix);
double calculateFidelity(const VectorXcd& state1, const VectorXcd& state2);
double calculateFidelity(const MatrixXcd& rho1, const MatrixXcd& rho2);
double calculateVonNeumannEntropy(const MatrixXcd& densityMatrix);
double calculateConcurrence(const MatrixXcd& densityMatrix);
double calculateNegativity(const MatrixXcd& densityMatrix);
double calculateCoherence(const MatrixXcd& densityMatrix);

// Partial transpose operation
MatrixXcd partialTranspose(const MatrixXcd& densityMatrix, int qubit, int totalQubits);

// Standard quantum states
VectorXcd createBellState(int bellIndex);
VectorXcd createGHZState(int numQubits);
VectorXcd createWState(int numQubits);

// Density matrix operations
MatrixXcd createDensityMatrix(const VectorXcd& stateVector);

// Measurement operations
VectorXcd measureState(const VectorXcd& state, int basis, std::mt19937& rng);
std::vector<double> performStateTomography(const VectorXcd& state, int numMeasurements);

// Expectation values
std::vector<double> calculateExpectationValues(const VectorXcd& state, const std::vector<MatrixXcd>& observables);

// Entanglement detection
bool isEntangled(const MatrixXcd& densityMatrix);

} // namespace omniq

#endif // OMNIQ_QUANTUMSTATES_H 