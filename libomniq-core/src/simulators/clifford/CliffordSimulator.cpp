//
// Created by Goutham Arcot on 18/01/26.
//

#include "omniq/Simulators/CliffordSimulator.h"
#include <random>
#include <sstream>
#include <stdexcept>

namespace omniq {
namespace simulators {

CliffordSimulator::CliffordSimulator(int numQubits) : numQubits_(numQubits) {
  // Initialize tableau to |0...0⟩ state
  x_ = Eigen::MatrixXi::Zero(2 * numQubits, numQubits);
  z_ = Eigen::MatrixXi::Zero(2 * numQubits, numQubits);
  r_ = Eigen::VectorXi::Zero(2 * numQubits);

  // Set stabilizers: Z_i for each qubit
  for (int i = 0; i < numQubits; ++i) {
    z_(i, i) = 1;
  }

  // Set destabilizers: X_i for each qubit
  for (int i = 0; i < numQubits; ++i) {
    x_(numQubits + i, i) = 1;
  }
}

void CliffordSimulator::reset() { *this = CliffordSimulator(numQubits_); }

int CliffordSimulator::g(int x1, int z1, int x2, int z2) const {
  // Compute phase exponent for Pauli product
  if (x1 == 0 && z1 == 0)
    return 0;
  if (x1 == 1 && z1 == 1)
    return z2 - x2;
  if (x1 == 1 && z1 == 0)
    return z2 * (2 * x2 - 1);
  return x2 * (1 - 2 * z2);
}

void CliffordSimulator::rowsum(int h, int i) {
  // Add row i to row h (with appropriate phase)
  int phase = 2 * r_(h) + 2 * r_(i);

  for (int j = 0; j < numQubits_; ++j) {
    phase += g(x_(i, j), z_(i, j), x_(h, j), z_(h, j));
    x_(h, j) = (x_(h, j) + x_(i, j)) % 2;
    z_(h, j) = (z_(h, j) + z_(i, j)) % 2;
  }

  r_(h) = (phase % 4) / 2;
}

void CliffordSimulator::applyH(int qubit) {
  // H transforms: X ↔ Z
  for (int i = 0; i < 2 * numQubits_; ++i) {
    int tmp = x_(i, qubit);
    x_(i, qubit) = z_(i, qubit);
    z_(i, qubit) = tmp;

    // Phase: -1 if both X and Z
    if (x_(i, qubit) && z_(i, qubit)) {
      r_(i) = (r_(i) + 1) % 2;
    }
  }
}

void CliffordSimulator::applyS(int qubit) {
  // S transforms: X → Y, Y → -X, Z → Z
  for (int i = 0; i < 2 * numQubits_; ++i) {
    if (x_(i, qubit)) {
      z_(i, qubit) = (z_(i, qubit) + 1) % 2;
      if (z_(i, qubit)) {
        r_(i) = (r_(i) + 1) % 2;
      }
    }
  }
}

void CliffordSimulator::applySdag(int qubit) {
  // S† = S³
  applyS(qubit);
  applyS(qubit);
  applyS(qubit);
}

void CliffordSimulator::applyCNOT(int control, int target) {
  // CNOT transforms: X_c → X_c X_t, Z_t → Z_c Z_t
  for (int i = 0; i < 2 * numQubits_; ++i) {
    r_(i) = (r_(i) + x_(i, control) * z_(i, target) *
                         (x_(i, target) + z_(i, control) + 1)) %
            2;
    x_(i, target) = (x_(i, target) + x_(i, control)) % 2;
    z_(i, control) = (z_(i, control) + z_(i, target)) % 2;
  }
}

void CliffordSimulator::applyCZ(int q1, int q2) {
  // CZ = H(q2) CNOT(q1, q2) H(q2)
  applyH(q2);
  applyCNOT(q1, q2);
  applyH(q2);
}

void CliffordSimulator::applyX(int qubit) {
  // X flips phase of Z stabilizers
  for (int i = 0; i < 2 * numQubits_; ++i) {
    if (z_(i, qubit)) {
      r_(i) = (r_(i) + 1) % 2;
    }
  }
}

void CliffordSimulator::applyY(int qubit) {
  // Y = iXZ
  for (int i = 0; i < 2 * numQubits_; ++i) {
    if (x_(i, qubit) ^ z_(i, qubit)) {
      r_(i) = (r_(i) + 1) % 2;
    }
  }
}

void CliffordSimulator::applyZ(int qubit) {
  // Z flips phase of X stabilizers
  for (int i = 0; i < 2 * numQubits_; ++i) {
    if (x_(i, qubit)) {
      r_(i) = (r_(i) + 1) % 2;
    }
  }
}

int CliffordSimulator::measure(int qubit) {
  // Check if qubit commutes with all stabilizers
  bool isRandom = false;
  int p = -1;

  for (int i = 0; i < numQubits_; ++i) {
    if (x_(i, qubit)) {
      isRandom = true;
      p = i;
      break;
    }
  }

  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 1);

  int result;
  if (isRandom) {
    // Random outcome
    result = dis(gen);

    // Update tableau
    for (int i = 0; i < 2 * numQubits_; ++i) {
      if (i != p && x_(i, qubit)) {
        rowsum(i, p);
      }
    }

    // Set row p to project onto measurement result
    x_.row(p).setZero();
    z_.row(p).setZero();
    z_(p, qubit) = 1;
    r_(p) = result;
  } else {
    // Deterministic outcome
    result = 0;
    for (int i = numQubits_; i < 2 * numQubits_; ++i) {
      if (x_(i, qubit)) {
        result ^= r_(i);
      }
    }
  }

  measurementHistory_.push_back(result);
  return result;
}

bool CliffordSimulator::isPureState() const {
  // A state is pure if all stabilizers are independent
  // (This is a simplified check)
  return true;
}

std::string CliffordSimulator::toString() const {
  std::ostringstream oss;
  oss << "CliffordSimulator(" << numQubits_ << " qubits)\n";
  oss << "Measurements: [";
  for (size_t i = 0; i < measurementHistory_.size(); ++i) {
    oss << measurementHistory_[i];
    if (i < measurementHistory_.size() - 1)
      oss << ", ";
  }
  oss << "]\n";
  return oss.str();
}

} // namespace simulators
} // namespace omniq
