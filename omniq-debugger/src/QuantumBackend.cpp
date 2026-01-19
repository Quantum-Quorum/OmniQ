//
// Created by Goutham Arcot on 17/07/25.
//

#include "CoreInterface.h"
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace omniq {

QuantumState::QuantumState(int numQubits)
    : numQubits_(numQubits),
      stateVector_(static_cast<int>(std::pow(2, numQubits))) {
  if (numQubits <= 0) {
    throw std::invalid_argument("Number of qubits must be positive");
  }

  stateVector_.setZero();
  stateVector_(0) = 1.0;
}

void QuantumState::reset() {
  stateVector_.setZero();
  stateVector_(0) = 1.0;
}

void QuantumState::applyGate(GateType type, int qubit, double parameter) {
  if (qubit < 0 || qubit >= numQubits_) {
    throw std::out_of_range("Qubit index out of range");
  }

  MatrixXcd gateMatrix = createSingleQubitGate(type, qubit, parameter);
  stateVector_ = gateMatrix * stateVector_;
}

void QuantumState::applyGate(GateType type, int control, int target,
                             double parameter) {
  if (control < 0 || control >= numQubits_ || target < 0 ||
      target >= numQubits_) {
    throw std::out_of_range("Qubit index out of range");
  }

  if (control == target) {
    throw std::invalid_argument("Control and target qubits must be different");
  }

  MatrixXcd gateMatrix = createTwoQubitGate(type, control, target, parameter);
  stateVector_ = gateMatrix * stateVector_;
}

double QuantumState::getQubitProbability(int qubit, int value) const {
  if (qubit < 0 || qubit >= numQubits_ || (value != 0 && value != 1)) {
    return 0.0;
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

std::complex<double> QuantumState::getQubitAmplitude(int qubit,
                                                     int value) const {
  if (qubit < 0 || qubit >= numQubits_ || (value != 0 && value != 1)) {
    return std::complex<double>(0.0, 0.0);
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

MatrixXcd QuantumState::createSingleQubitGate(GateType type, int qubit,
                                              double parameter) {
  int dim = static_cast<int>(std::pow(2, numQubits_));
  MatrixXcd fullMatrix = MatrixXcd::Identity(dim, dim);

  Matrix2cd gateMatrix;
  switch (type) {
  case GateType::H:
    gateMatrix = (Matrix2cd() << 1.0 / std::sqrt(2), 1.0 / std::sqrt(2),
                  1.0 / std::sqrt(2), -1.0 / std::sqrt(2))
                     .finished();
    break;
  case GateType::X:
    gateMatrix = (Matrix2cd() << 0.0, 1.0, 1.0, 0.0).finished();
    break;
  case GateType::Y:
    gateMatrix = (Matrix2cd() << 0.0, std::complex<double>(0, -1),
                  std::complex<double>(0, 1), 0.0)
                     .finished();
    break;
  case GateType::Z:
    gateMatrix = (Matrix2cd() << 1.0, 0.0, 0.0, -1.0).finished();
    break;
  case GateType::PHASE:
    gateMatrix = (Matrix2cd() << 1.0, 0.0, 0.0,
                  std::exp(std::complex<double>(0, parameter)))
                     .finished();
    break;
  case GateType::RX:
    gateMatrix = (Matrix2cd() << std::cos(parameter / 2),
                  std::complex<double>(0, -std::sin(parameter / 2)),
                  std::complex<double>(0, -std::sin(parameter / 2)),
                  std::cos(parameter / 2))
                     .finished();
    break;
  case GateType::RY:
    gateMatrix =
        (Matrix2cd() << std::cos(parameter / 2), -std::sin(parameter / 2),
         std::sin(parameter / 2), std::cos(parameter / 2))
            .finished();
    break;
  case GateType::RZ:
    gateMatrix =
        (Matrix2cd() << std::exp(std::complex<double>(0, -parameter / 2)), 0.0,
         0.0, std::exp(std::complex<double>(0, parameter / 2)))
            .finished();
    break;
  default:
    throw std::invalid_argument("Unsupported gate type for single qubit");
  }

  for (int i = 0; i < dim; ++i) {
    for (int j = 0; j < dim; ++j) {
      bool otherQubitsMatch = true;
      for (int k = 0; k < numQubits_; ++k) {
        if (k != qubit) {
          if (((i >> k) & 1) != ((j >> k) & 1)) {
            otherQubitsMatch = false;
            break;
          }
        }
      }

      if (otherQubitsMatch) {
        int qubitStateI = (i >> qubit) & 1;
        int qubitStateJ = (j >> qubit) & 1;
        fullMatrix(i, j) = gateMatrix(qubitStateI, qubitStateJ);
      } else {
        fullMatrix(i, j) = 0.0;
      }
    }
  }

  return fullMatrix;
}

MatrixXcd QuantumState::createTwoQubitGate(GateType type, int qubit1,
                                           int qubit2, double parameter) {
  int dim = static_cast<int>(std::pow(2, numQubits_));
  MatrixXcd fullMatrix = MatrixXcd::Identity(dim, dim);

  Matrix4cd gateMatrix;
  switch (type) {
  case GateType::CNOT:
    gateMatrix = (Matrix4cd() << 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
                  0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0)
                     .finished();
    break;
  case GateType::SWAP:
    gateMatrix = (Matrix4cd() << 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
                  1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0)
                     .finished();
    break;
  case GateType::CP:
    gateMatrix =
        (Matrix4cd() << 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,
         0.0, 0.0, 0.0, 0.0, std::exp(std::complex<double>(0, parameter)))
            .finished();
    break;
  default:
    throw std::invalid_argument("Unsupported gate type for two qubits");
  }

  for (int i = 0; i < dim; ++i) {
    for (int j = 0; j < dim; ++j) {
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
        int qubit1StateI = (i >> qubit1) & 1;
        int qubit2StateI = (i >> qubit2) & 1;
        int qubit1StateJ = (j >> qubit1) & 1;
        int qubit2StateJ = (j >> qubit2) & 1;

        int gateRow = qubit1StateI * 2 + qubit2StateI;
        int gateCol = qubit1StateJ * 2 + qubit2StateJ;
        fullMatrix(i, j) = gateMatrix(gateRow, gateCol);
      } else {
        fullMatrix(i, j) = 0.0;
      }
    }
  }

  return fullMatrix;
}

// Quantum circuit implementation
QuantumCircuit::QuantumCircuit(int numQubits)
    : numQubits_(numQubits), currentStep_(0) {
  if (numQubits <= 0) {
    throw std::invalid_argument("Number of qubits must be positive");
  }
}

void QuantumCircuit::addGate(GateType type, int qubit, double parameter) {
  gates_.emplace_back(type, qubit, -1,
                      parameter); // -1 indicates single qubit gate
}

void QuantumCircuit::addGate(GateType type, int control, int target,
                             double parameter) {
  gates_.emplace_back(type, control, target, parameter);
}

bool QuantumCircuit::executeStep(QuantumState &state) {
  if (currentStep_ >= static_cast<int>(gates_.size())) {
    return false; // No more gates to execute
  }

  const auto &gate = gates_[currentStep_];
  GateType type = std::get<0>(gate);
  int qubit1 = std::get<1>(gate);
  int qubit2 = std::get<2>(gate);
  double parameter = std::get<3>(gate);

  if (qubit2 == -1) {
    // Single qubit gate
    state.applyGate(type, qubit1, parameter);
  } else {
    // Two qubit gate
    state.applyGate(type, qubit1, qubit2, parameter);
  }

  currentStep_++;
  return true;
}

void QuantumCircuit::reset() { currentStep_ = 0; }

std::string QuantumCircuit::getGateDescription(int step) const {
  if (step < 0 || step >= static_cast<int>(gates_.size())) {
    return "Invalid step";
  }

  const auto &gate = gates_[step];
  GateType type = std::get<0>(gate);
  int qubit1 = std::get<1>(gate);
  int qubit2 = std::get<2>(gate);
  double parameter = std::get<3>(gate);

  std::stringstream ss;
  switch (type) {
  case GateType::H:
    ss << "H(" << qubit1 << ")";
    break;
  case GateType::X:
    ss << "X(" << qubit1 << ")";
    break;
  case GateType::Y:
    ss << "Y(" << qubit1 << ")";
    break;
  case GateType::Z:
    ss << "Z(" << qubit1 << ")";
    break;
  case GateType::CNOT:
    ss << "CNOT(" << qubit1 << ", " << qubit2 << ")";
    break;
  case GateType::SWAP:
    ss << "SWAP(" << qubit1 << ", " << qubit2 << ")";
    break;
  case GateType::PHASE:
    ss << "PHASE(" << qubit1 << ", " << std::fixed << std::setprecision(3)
       << parameter << ")";
    break;
  case GateType::RX:
    ss << "RX(" << qubit1 << ", " << std::fixed << std::setprecision(3)
       << parameter << ")";
    break;
  case GateType::RY:
    ss << "RY(" << qubit1 << ", " << std::fixed << std::setprecision(3)
       << parameter << ")";
    break;
  case GateType::RZ:
    ss << "RZ(" << qubit1 << ", " << std::fixed << std::setprecision(3)
       << parameter << ")";
    break;
  case GateType::CP:
    ss << "CP(" << qubit1 << ", " << qubit2 << ", " << std::fixed
       << std::setprecision(3) << parameter << ")";
    break;
  }

  return ss.str();
}

} // namespace omniq