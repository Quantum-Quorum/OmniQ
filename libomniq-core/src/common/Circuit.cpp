//
// Created by Goutham Arcot on 17/07/25.
//

#include "omniq/Circuit.h"
#include "omniq/Operators.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace omniq {

Circuit::Circuit(int numQubits, int numClassicalBits)
    : numQubits_(numQubits), numClassicalBits_(numClassicalBits),
      currentStep_(0) {
  if (numQubits <= 0) {
    throw std::invalid_argument("Number of qubits must be positive");
  }

  // Initialize qubit states to |0⟩
  stateVector_ = VectorXcd::Zero(static_cast<int>(std::pow(2, numQubits)));
  stateVector_(0) = 1.0; // |0...0⟩ state

  // Initialize classical bits to 0
  classicalBits_.resize(numClassicalBits, 0);
}

Circuit::Circuit(const Circuit &other)
    : numQubits_(other.numQubits_), numClassicalBits_(other.numClassicalBits_),
      gates_(other.gates_), stateVector_(other.stateVector_),
      classicalBits_(other.classicalBits_), currentStep_(other.currentStep_) {}

Circuit &Circuit::operator=(const Circuit &other) {
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

void Circuit::addGate(const Gate &gate) {
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

void Circuit::addGate(GateType type, int controlQubit, int targetQubit,
                      double parameter) {
  Gate gate;
  gate.type = type;
  gate.controlQubits = {controlQubit};
  gate.targetQubits = {targetQubit};
  gate.parameters = {parameter};

  addGate(gate);
}

void Circuit::addGate(GateType type, const std::vector<int> &targetQubits,
                      const std::vector<double> &parameters) {
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

void Circuit::insertGate(int index, const Gate &gate) {
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

  const Gate &gate = gates_[currentStep_];
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

void Circuit::applyGate(const Gate &gate) {
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

  long long dim = stateVector_.size();
  long long pairDist = 1LL << qubit;
  double invSqrt2 = 1.0 / std::sqrt(2.0);

#pragma omp parallel for
  for (long long i = 0; i < dim; i += 2 * pairDist) {
    for (long long j = 0; j < pairDist; ++j) {
      long long idx0 = i + j;
      long long idx1 = idx0 + pairDist;

      std::complex<double> alpha = stateVector_(idx0);
      std::complex<double> beta = stateVector_(idx1);

      stateVector_(idx0) = invSqrt2 * (alpha + beta);
      stateVector_(idx1) = invSqrt2 * (alpha - beta);
    }
  }
}

void Circuit::applyPauliX(int qubit) {
  validateQubitIndex(qubit);

  long long dim = stateVector_.size();
  long long pairDist = 1LL << qubit;

#pragma omp parallel for
  for (long long i = 0; i < dim; i += 2 * pairDist) {
    for (long long j = 0; j < pairDist; ++j) {
      long long idx0 = i + j;
      long long idx1 = idx0 + pairDist;

      std::swap(stateVector_(idx0), stateVector_(idx1));
    }
  }
}

void Circuit::applyPauliY(int qubit) {
  validateQubitIndex(qubit);

  long long dim = stateVector_.size();
  long long pairDist = 1LL << qubit;
  std::complex<double> imagUnit(0.0, 1.0);

#pragma omp parallel for
  for (long long i = 0; i < dim; i += 2 * pairDist) {
    for (long long j = 0; j < pairDist; ++j) {
      long long idx0 = i + j;
      long long idx1 = idx0 + pairDist;

      std::complex<double> alpha = stateVector_(idx0);
      std::complex<double> beta = stateVector_(idx1);

      stateVector_(idx0) = -imagUnit * beta;
      stateVector_(idx1) = imagUnit * alpha;
    }
  }
}

void Circuit::applyPauliZ(int qubit) {
  validateQubitIndex(qubit);

  long long dim = stateVector_.size();
  long long pairDist = 1LL << qubit;

#pragma omp parallel for
  for (long long i = 0; i < dim; i += 2 * pairDist) {
    for (long long j = 0; j < pairDist; ++j) {
      long long idx1 = i + j + pairDist;
      stateVector_(idx1) = -stateVector_(idx1);
    }
  }
}

void Circuit::applyCNOT(int controlQubit, int targetQubit) {
  validateQubitIndex(controlQubit);
  validateQubitIndex(targetQubit);

  if (controlQubit == targetQubit) {
    throw std::invalid_argument("Control and target qubits must be different");
  }

  long long dim = stateVector_.size();
  long long controlMask = 1LL << controlQubit;

  // We only modify states where the control qubit is 1

  // Approach: Iterate over all states, check if control is 1.
  // Optimization: Iterate explicitly to hit pairs.
  // simpler to just iterate all and branch inside for clarity first, or better
  // Loop structure? Let's use a general loop and check bit. Since we need to
  // swap pairs for target, we can iterate over pairs defined by targetQubit,
  // check controlQubit.

  long long pairDist = 1LL << targetQubit;

#pragma omp parallel for
  for (long long i = 0; i < dim; i += 2 * pairDist) {
    for (long long j = 0; j < pairDist; ++j) {
      long long idx0 = i + j;
      // idx0 has 0 at targetQubit position

      if ((idx0 & controlMask) != 0) {
        // Control is 1, so perform X on target (swap idx0 and idx1)
        long long idx1 = idx0 + pairDist;
        std::swap(stateVector_(idx0), stateVector_(idx1));
      }
    }
  }
}

void Circuit::applySWAP(int qubit1, int qubit2) {
  validateQubitIndex(qubit1);
  validateQubitIndex(qubit2);

  if (qubit1 == qubit2)
    return;

  long long dim = stateVector_.size();
  long long mask1 = 1LL << qubit1;
  long long mask2 = 1LL << qubit2;

#pragma omp parallel for
  for (long long i = 0; i < dim; ++i) {
    // We only need to swap if bits differ (i.e. one is 0 and other is 1)
    // If bits are same (00 or 11), amplitude stays in same index
    // To avoid double swapping, we can iterate and only swap if i <
    // swap_target? Or simpler: iterate all, check if bit1 != bit2. Actually,
    // efficiently: Identify indices where bit at qubit1 != bit at qubit2 For
    // such index k, let l be k with bits flipped. Swap(k, l). To do each pair
    // once: enforce k < l.

    // Let's stick to a simpler parallel loop over all elements, but be careful.
    // Better: iterate such that we touch unique pairs.
    // It's effectively iterating over all bits except q1, q2...
    // For simplicity with OpenMP, let's just do a masked check and ensure i <
    // target.

    bool b1 = (i & mask1) != 0;
    bool b2 = (i & mask2) != 0;

    if (b1 != b2) {
      long long target =
          i ^ mask1 ^
          mask2; // Flip both bits (0->1, 1->0) to maintain difference?
      // Wait. If bits differ (0,1) -> (1,0).
      // idx i: ...0...1...
      // idx j: ...1...0...
      // j = i - mask2 + mask1 (if q2 is 1).
      // Actually, j = i ^ mask1 ^ mask2 simply flips both bits.
      // If i has (0,1), j has (1,0). Correct.

      if (i < target) {
        std::swap(stateVector_(i), stateVector_(target));
      }
    }
  }
}

void Circuit::applyPhaseShift(int qubit, double angle) {
  validateQubitIndex(qubit);

  long long dim = stateVector_.size();
  long long pairDist = 1LL << qubit;
  std::complex<double> phase(std::cos(angle), std::sin(angle));

#pragma omp parallel for
  for (long long i = 0; i < dim; i += 2 * pairDist) {
    for (long long j = 0; j < pairDist; ++j) {
      long long idx1 = i + j + pairDist; // State with qubit=1
      stateVector_(idx1) *= phase;
    }
  }
}

void Circuit::applyRotationX(int qubit, double angle) {
  validateQubitIndex(qubit);

  long long dim = stateVector_.size();
  long long pairDist = 1LL << qubit;
  double c = std::cos(angle / 2.0);
  double s = std::sin(angle / 2.0);
  std::complex<double> mi_s(0.0, -s); // -i * sin(theta/2)

#pragma omp parallel for
  for (long long i = 0; i < dim; i += 2 * pairDist) {
    for (long long j = 0; j < pairDist; ++j) {
      long long idx0 = i + j;
      long long idx1 = idx0 + pairDist;

      std::complex<double> alpha = stateVector_(idx0);
      std::complex<double> beta = stateVector_(idx1);

      stateVector_(idx0) = c * alpha + mi_s * beta;
      stateVector_(idx1) = mi_s * alpha + c * beta;
    }
  }
}

void Circuit::applyRotationY(int qubit, double angle) {
  validateQubitIndex(qubit);

  long long dim = stateVector_.size();
  long long pairDist = 1LL << qubit;
  double c = std::cos(angle / 2.0);
  double s = std::sin(angle / 2.0);

#pragma omp parallel for
  for (long long i = 0; i < dim; i += 2 * pairDist) {
    for (long long j = 0; j < pairDist; ++j) {
      long long idx0 = i + j;
      long long idx1 = idx0 + pairDist;

      std::complex<double> alpha = stateVector_(idx0);
      std::complex<double> beta = stateVector_(idx1);

      stateVector_(idx0) = c * alpha - s * beta;
      stateVector_(idx1) = s * alpha + c * beta;
    }
  }
}

void Circuit::applyRotationZ(int qubit, double angle) {
  validateQubitIndex(qubit);

  long long dim = stateVector_.size();
  long long pairDist = 1LL << qubit;
  std::complex<double> phase_neg(std::cos(-angle / 2.0),
                                 std::sin(-angle / 2.0));
  std::complex<double> phase_pos(std::cos(angle / 2.0), std::sin(angle / 2.0));

#pragma omp parallel for
  for (long long i = 0; i < dim; i += 2 * pairDist) {
    for (long long j = 0; j < pairDist; ++j) {
      long long idx0 = i + j;
      long long idx1 = idx0 + pairDist;

      stateVector_(idx0) *= phase_neg;
      stateVector_(idx1) *= phase_pos;
    }
  }
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

  // Perform measurement (simplified - always measure |0⟩ for deterministic
  // behavior)
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

MatrixXcd Circuit::createSingleQubitGate(const Matrix2cd &gate, int qubit) {
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

MatrixXcd Circuit::createTwoQubitGate(const Matrix4cd &gate, int qubit1,
                                      int qubit2) {
  int dim = static_cast<int>(std::pow(2, numQubits_));
  MatrixXcd fullMatrix = MatrixXcd::Identity(dim, dim);

  // For simplicity, we'll create a direct implementation
  // This is a simplified version - in practice, you'd want a more efficient
  // implementation

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

void Circuit::validateGate(const Gate &gate) {
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
  allQubits.insert(allQubits.end(), gate.controlQubits.begin(),
                   gate.controlQubits.end());
  allQubits.insert(allQubits.end(), gate.targetQubits.begin(),
                   gate.targetQubits.end());

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

  for (const auto &gate : gates_) {
    qasm << gateToString(gate) << "\n";
  }

  return qasm.str();
}

std::string Circuit::gateToString(const Gate &gate) const {
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
    ss << "cx q[" << gate.controlQubits[0] << "], q[" << gate.targetQubits[0]
       << "]";
    break;
  case GateType::SWAP:
    ss << "swap q[" << gate.targetQubits[0] << "], q[" << gate.targetQubits[1]
       << "]";
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
    ss << "measure q[" << gate.targetQubits[0] << "] -> c["
       << gate.controlQubits[0] << "]";
    break;
  default:
    ss << "// Unknown gate type";
    break;
  }

  return ss.str();
}

} // namespace omniq
