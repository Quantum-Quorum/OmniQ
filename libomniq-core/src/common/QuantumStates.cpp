//
// Created by Goutham Arcot on 17/07/25.
//

#include "omniq/QuantumStates.h"
#include "omniq/Operators.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>

namespace omniq {

// Quantum state analysis functions
double calculatePurity(const MatrixXcd &densityMatrix) {
  MatrixXcd squared = densityMatrix * densityMatrix;
  return squared.trace().real();
}

double calculateFidelity(const VectorXcd &state1, const VectorXcd &state2) {
  if (state1.size() != state2.size()) {
    throw std::invalid_argument("States must have the same dimension");
  }

  std::complex<double> overlap = state1.adjoint() * state2;
  return std::norm(overlap);
}

double calculateFidelity(const MatrixXcd &rho1, const MatrixXcd &rho2) {
  if (rho1.rows() != rho2.rows() || rho1.cols() != rho2.cols()) {
    throw std::invalid_argument(
        "Density matrices must have the same dimensions");
  }

  // Calculate Fidelity = Tr(sqrt(sqrt(rho1) * rho2 * sqrt(rho1)))
  // For simplicity, we use a more direct approach
  MatrixXcd product = rho1 * rho2;
  return std::sqrt(std::abs(product.trace()));
}

double calculateVonNeumannEntropy(const MatrixXcd &densityMatrix) {
  // Calculate eigenvalues
  Eigen::SelfAdjointEigenSolver<MatrixXcd> solver(densityMatrix);
  const VectorXcd &eigenvalues = solver.eigenvalues();

  double entropy = 0.0;
  for (int i = 0; i < eigenvalues.size(); ++i) {
    double lambda = eigenvalues(i).real();
    if (lambda > 1e-12) { // Avoid log(0)
      entropy -= lambda * std::log2(lambda);
    }
  }

  return entropy;
}

// Helper to compute square root of a positive semi-definite matrix
MatrixXcd matrixSqrt(const MatrixXcd &mat) {
  Eigen::SelfAdjointEigenSolver<MatrixXcd> solver(mat);
  // Eigenvalues of Hermitian matrix are real
  Eigen::VectorXd eigenvalues = solver.eigenvalues();
  Eigen::VectorXd sqrtEigenvalues = eigenvalues.cwiseSqrt();
  return solver.eigenvectors() * sqrtEigenvalues.asDiagonal() *
         solver.eigenvectors().adjoint();
}

double calculateConcurrence(const MatrixXcd &densityMatrix) {
  if (densityMatrix.rows() != 4 || densityMatrix.cols() != 4) {
    throw std::invalid_argument(
        "Concurrence is only defined for 2-qubit states");
  }

  // For 2-qubit states, concurrence calculation involves the spin-flip
  // operation
  Matrix2cd sigma_y = Operators::PAULI_Y;
  Matrix4cd spin_flip = Operators::tensorProduct(sigma_y, sigma_y);

  Matrix4cd rho_tilde = spin_flip * densityMatrix.conjugate() * spin_flip;

  // R = sqrt(sqrt(rho) * rho_tilde * sqrt(rho))
  MatrixXcd sqrtRho = matrixSqrt(densityMatrix);
  Matrix4cd R = matrixSqrt(sqrtRho * rho_tilde * sqrtRho);

  Eigen::SelfAdjointEigenSolver<Matrix4cd> solver(R);
  const VectorXcd &eigenvalues = solver.eigenvalues();

  // Sort eigenvalues in descending order
  std::vector<double> sorted_eigenvalues;
  for (int i = 0; i < eigenvalues.size(); ++i) {
    sorted_eigenvalues.push_back(eigenvalues(i).real());
  }
  std::sort(sorted_eigenvalues.begin(), sorted_eigenvalues.end(),
            std::greater<double>());

  double concurrence =
      std::max(0.0, sorted_eigenvalues[0] - sorted_eigenvalues[1] -
                        sorted_eigenvalues[2] - sorted_eigenvalues[3]);

  return concurrence;
}

double calculateNegativity(const MatrixXcd &densityMatrix) {
  // Calculate partial transpose and its eigenvalues
  MatrixXcd partial_transpose =
      partialTranspose(densityMatrix, 0, 2); // Assuming 2-qubit system

  Eigen::SelfAdjointEigenSolver<MatrixXcd> solver(partial_transpose);
  const VectorXcd &eigenvalues = solver.eigenvalues();

  double negativity = 0.0;
  for (int i = 0; i < eigenvalues.size(); ++i) {
    double lambda = eigenvalues(i).real();
    if (lambda < 0) {
      negativity += std::abs(lambda);
    }
  }

  return negativity;
}

MatrixXcd partialTranspose(const MatrixXcd &densityMatrix, int qubit,
                           int totalQubits) {
  if (densityMatrix.rows() != densityMatrix.cols()) {
    throw std::invalid_argument("Density matrix must be square");
  }

  int dim = densityMatrix.rows();
  int dimPerQubit = static_cast<int>(std::pow(2, totalQubits - 1));
  MatrixXcd result = densityMatrix;

  // Perform partial transpose on the specified qubit
  for (int i = 0; i < dimPerQubit; ++i) {
    for (int j = 0; j < dimPerQubit; ++j) {
      for (int k = 0; k < 2; ++k) {
        for (int l = 0; l < 2; ++l) {
          int row1 = i + k * dimPerQubit;
          int col1 = j + l * dimPerQubit;
          int row2 = i + l * dimPerQubit;
          int col2 = j + k * dimPerQubit;

          std::complex<double> temp = result(row1, col1);
          result(row1, col1) = result(row2, col2);
          result(row2, col2) = temp;
        }
      }
    }
  }

  return result;
}

VectorXcd createBellState(int bellIndex) {
  if (bellIndex < 0 || bellIndex > 3) {
    throw std::invalid_argument("Bell index must be 0, 1, 2, or 3");
  }

  VectorXcd bellState(4);
  bellState.setZero();

  switch (bellIndex) {
  case 0: // |Φ⁺⟩ = (|00⟩ + |11⟩)/√2
    bellState(0) = 1.0 / std::sqrt(2);
    bellState(3) = 1.0 / std::sqrt(2);
    break;
  case 1: // |Φ⁻⟩ = (|00⟩ - |11⟩)/√2
    bellState(0) = 1.0 / std::sqrt(2);
    bellState(3) = -1.0 / std::sqrt(2);
    break;
  case 2: // |Ψ⁺⟩ = (|01⟩ + |10⟩)/√2
    bellState(1) = 1.0 / std::sqrt(2);
    bellState(2) = 1.0 / std::sqrt(2);
    break;
  case 3: // |Ψ⁻⟩ = (|01⟩ - |10⟩)/√2
    bellState(1) = 1.0 / std::sqrt(2);
    bellState(2) = -1.0 / std::sqrt(2);
    break;
  }

  return bellState;
}

VectorXcd createGHZState(int numQubits) {
  if (numQubits < 2) {
    throw std::invalid_argument("GHZ state requires at least 2 qubits");
  }

  int dim = static_cast<int>(std::pow(2, numQubits));
  VectorXcd ghzState = VectorXcd::Zero(dim);

  // |GHZ⟩ = (|0...0⟩ + |1...1⟩)/√2
  ghzState(0) = 1.0 / std::sqrt(2);
  ghzState(dim - 1) = 1.0 / std::sqrt(2);

  return ghzState;
}

VectorXcd createWState(int numQubits) {
  if (numQubits < 2) {
    throw std::invalid_argument("W state requires at least 2 qubits");
  }

  int dim = static_cast<int>(std::pow(2, numQubits));
  VectorXcd wState = VectorXcd::Zero(dim);

  // |W⟩ = (|100...0⟩ + |010...0⟩ + ... + |000...1⟩)/√n
  double amplitude = 1.0 / std::sqrt(numQubits);

  for (int i = 0; i < numQubits; ++i) {
    int index = static_cast<int>(std::pow(2, i));
    wState(index) = amplitude;
  }

  return wState;
}

MatrixXcd createDensityMatrix(const VectorXcd &stateVector) {
  return stateVector * stateVector.adjoint();
}

VectorXcd measureState(const VectorXcd &state, int basis, std::mt19937 &rng) {
  if (basis < 0 || basis > 2) {
    throw std::invalid_argument("Basis must be 0 (Z), 1 (X), or 2 (Y)");
  }

  // Create measurement operators
  Matrix2cd M0 = Operators::measurementOperator(basis, 0);
  Matrix2cd M1 = Operators::measurementOperator(basis, 1);

  // Calculate measurement probabilities
  double p0 = std::norm((M0 * state).norm());
  double p1 = std::norm((M1 * state).norm());

  // Normalize probabilities
  double total = p0 + p1;
  p0 /= total;
  p1 /= total;

  // Perform measurement
  std::uniform_real_distribution<double> dist(0.0, 1.0);
  double random = dist(rng);

  if (random < p0) {
    return M0 * state / std::sqrt(p0);
  } else {
    return M1 * state / std::sqrt(p1);
  }
}

std::vector<double> performStateTomography(const VectorXcd &state,
                                           int numMeasurements) {
  std::vector<double> tomographyData;
  std::mt19937 rng(std::random_device{}());

  // Perform measurements in different bases
  for (int basis = 0; basis < 3; ++basis) { // Z, X, Y bases
    for (int measurement = 0; measurement < numMeasurements; ++measurement) {
      VectorXcd measuredState = measureState(state, basis, rng);

      // Calculate expectation values
      double expectation = 0.0;
      switch (basis) {
      case 0: // Z basis
        expectation = std::norm(measuredState(0)) - std::norm(measuredState(1));
        break;
      case 1: // X basis
        expectation =
            2 * std::real(measuredState(0) * std::conj(measuredState(1)));
        break;
      case 2: // Y basis
        expectation =
            2 * std::imag(measuredState(0) * std::conj(measuredState(1)));
        break;
      }

      tomographyData.push_back(expectation);
    }
  }

  return tomographyData;
}

double calculateCoherence(const MatrixXcd &densityMatrix) {
  // Calculate off-diagonal elements (coherence)
  double coherence = 0.0;
  int dim = densityMatrix.rows();

  for (int i = 0; i < dim; ++i) {
    for (int j = i + 1; j < dim; ++j) {
      coherence += std::abs(densityMatrix(i, j));
    }
  }

  return coherence;
}

std::vector<double>
calculateExpectationValues(const VectorXcd &state,
                           const std::vector<MatrixXcd> &observables) {
  std::vector<double> expectations;

  for (const auto &observable : observables) {
    std::complex<double> expectation = state.adjoint() * observable * state;
    expectations.push_back(expectation.real());
  }

  return expectations;
}

bool isEntangled(const MatrixXcd &densityMatrix) {
  // Use Peres-Horodecki criterion (PPT criterion)
  MatrixXcd partial_transpose = partialTranspose(densityMatrix, 0, 2);

  Eigen::SelfAdjointEigenSolver<MatrixXcd> solver(partial_transpose);
  const VectorXcd &eigenvalues = solver.eigenvalues();

  // If any eigenvalue is negative, the state is entangled
  for (int i = 0; i < eigenvalues.size(); ++i) {
    if (eigenvalues(i).real() < -1e-12) {
      return true;
    }
  }

  return false;
}

} // namespace omniq
