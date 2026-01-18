//
// Created by Goutham Arcot on 18/01/26.
//

#include "omniq/QEC/SurfaceCode.h"
#include <cmath>
#include <stdexcept>

namespace omniq {
namespace qec {

SurfaceCode::SurfaceCode(int distance) : distance_(distance) {
  if (distance % 2 == 0 || distance < 3) {
    throw std::invalid_argument("Surface code distance must be odd and >= 3");
  }

  initializeLattice();
  constructStabilizers();
}

void SurfaceCode::initializeLattice() {
  // Rotated surface code layout
  // For distance d, we have d^2 data qubits
  // and (d^2 - 1) ancilla qubits

  int d = distance_;
  int qubitIndex = 0;

  // Place data qubits on checkerboard pattern
  for (int row = 0; row < d; ++row) {
    for (int col = 0; col < d; ++col) {
      dataQubits_.push_back(QubitPosition(row, col, qubitIndex++));
      positionToDataQubit_[{row, col}] =
          static_cast<int>(dataQubits_.size()) - 1;
    }
  }

  // Place X-ancilla qubits (plaquettes - measure XXXX)
  // Located at centers of "white" squares on checkerboard
  for (int row = 0; row < d - 1; ++row) {
    for (int col = 0; col < d - 1; ++col) {
      if ((row + col) % 2 == 0) {
        xAncillaQubits_.push_back(QubitPosition(row, col, qubitIndex++));
        positionToXAncilla_[{row, col}] =
            static_cast<int>(xAncillaQubits_.size()) - 1;
      }
    }
  }

  // Place Z-ancilla qubits (vertices - measure ZZZZ)
  // Located at centers of "black" squares on checkerboard
  for (int row = 0; row < d - 1; ++row) {
    for (int col = 0; col < d - 1; ++col) {
      if ((row + col) % 2 == 1) {
        zAncillaQubits_.push_back(QubitPosition(row, col, qubitIndex++));
        positionToZAncilla_[{row, col}] =
            static_cast<int>(zAncillaQubits_.size()) - 1;
      }
    }
  }
}

void SurfaceCode::constructStabilizers() {
  int d = distance_;
  int numDataQubits = d * d;

  // X-stabilizers (plaquettes)
  int xStabIdx = 0;
  for (const auto &ancilla : xAncillaQubits_) {
    std::vector<PauliOperator> paulis(numDataQubits, PauliOperator::I);

    // Apply X to 4 neighboring data qubits (top, right, bottom, left)
    std::vector<std::pair<int, int>> neighbors = {
        {ancilla.row, ancilla.col},
        {ancilla.row, ancilla.col + 1},
        {ancilla.row + 1, ancilla.col},
        {ancilla.row + 1, ancilla.col + 1}};

    for (const auto &[r, c] : neighbors) {
      if (r >= 0 && r < d && c >= 0 && c < d) {
        auto it = positionToDataQubit_.find({r, c});
        if (it != positionToDataQubit_.end()) {
          int dataIdx = it->second;
          paulis[dataQubits_[dataIdx].globalIndex] = PauliOperator::X;
        }
      }
    }

    StabilizerInfo info;
    info.index = xStabIdx++;
    info.row = ancilla.row;
    info.col = ancilla.col;
    info.isXType = true;
    info.op = Stabilizer(paulis);
    xStabilizers_.push_back(info);
  }

  // Z-stabilizers (vertices)
  int zStabIdx = 0;
  for (const auto &ancilla : zAncillaQubits_) {
    std::vector<PauliOperator> paulis(numDataQubits, PauliOperator::I);

    // Apply Z to 4 neighboring data qubits
    std::vector<std::pair<int, int>> neighbors = {
        {ancilla.row, ancilla.col},
        {ancilla.row, ancilla.col + 1},
        {ancilla.row + 1, ancilla.col},
        {ancilla.row + 1, ancilla.col + 1}};

    for (const auto &[r, c] : neighbors) {
      if (r >= 0 && r < d && c >= 0 && c < d) {
        auto it = positionToDataQubit_.find({r, c});
        if (it != positionToDataQubit_.end()) {
          int dataIdx = it->second;
          paulis[dataQubits_[dataIdx].globalIndex] = PauliOperator::Z;
        }
      }
    }

    StabilizerInfo info;
    info.index = zStabIdx++;
    info.row = ancilla.row;
    info.col = ancilla.col;
    info.isXType = false;
    info.op = Stabilizer(paulis);
    zStabilizers_.push_back(info);
  }
}

Syndrome SurfaceCode::measureSyndromes(const Statevector &state) {
  // For now, return a dummy syndrome
  // Real implementation would measure stabilizer expectation values
  int numStabilizers =
      static_cast<int>(xStabilizers_.size() + zStabilizers_.size());
  Syndrome syndrome(numStabilizers);
  syndrome.setCodeDistance(distance_);

  // TODO: Implement actual stabilizer measurement via expectation values
  // For now, all stabilizers are satisfied

  return syndrome;
}

void SurfaceCode::applyCorrection(const std::vector<int> &errorChain,
                                  Statevector &state) {
  // Apply X gates to qubits in error chain
  for (int qubitIdx : errorChain) {
    state.applyPauliX(qubitIdx);
  }
}

std::vector<int> SurfaceCode::getStabilizerSupport(int stabilizerIndex,
                                                   bool isXType) const {
  const auto &stabilizers = isXType ? xStabilizers_ : zStabilizers_;
  if (stabilizerIndex < 0 ||
      stabilizerIndex >= static_cast<int>(stabilizers.size())) {
    throw std::out_of_range("Stabilizer index out of range");
  }

  return stabilizers[stabilizerIndex].op.getSupportQubits();
}

Statevector SurfaceCode::prepareLogicalZero() const {
  int numQubits = distance_ * distance_;
  Statevector state(numQubits);
  // Logical |0⟩ is |00...0⟩
  return state;
}

Statevector SurfaceCode::prepareLogicalPlus() const {
  int numQubits = distance_ * distance_;
  Statevector state(numQubits);
  // Apply Hadamard to all qubits for |++...+⟩
  for (int i = 0; i < numQubits; ++i) {
    state.applyHadamard(i);
  }
  return state;
}

bool SurfaceCode::isLogicalZero(const Statevector &state) const {
  // Check if all Z-stabilizers are satisfied
  // This is a simplified check
  Syndrome syndrome = const_cast<SurfaceCode *>(this)->measureSyndromes(state);
  return syndrome.isAllZero();
}

} // namespace qec
} // namespace omniq
