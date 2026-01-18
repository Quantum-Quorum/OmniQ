//
// Created by Goutham Arcot on 18/01/26.
//

#include "omniq/QEC/Stabilizer.h"
#include <sstream>
#include <stdexcept>

namespace omniq {
namespace qec {

Stabilizer::Stabilizer() {}

Stabilizer::Stabilizer(const std::vector<PauliOperator> &paulis)
    : paulis_(paulis) {}

PauliOperator Stabilizer::getPauli(int qubitIndex) const {
  if (qubitIndex < 0 || qubitIndex >= static_cast<int>(paulis_.size())) {
    throw std::out_of_range("Qubit index out of range");
  }
  return paulis_[qubitIndex];
}

std::vector<int> Stabilizer::getSupportQubits() const {
  std::vector<int> support;
  for (size_t i = 0; i < paulis_.size(); ++i) {
    if (paulis_[i] != PauliOperator::I) {
      support.push_back(static_cast<int>(i));
    }
  }
  return support;
}

int Stabilizer::getWeight() const {
  int weight = 0;
  for (const auto &p : paulis_) {
    if (p != PauliOperator::I)
      weight++;
  }
  return weight;
}

bool Stabilizer::isXType() const {
  for (const auto &p : paulis_) {
    if (p == PauliOperator::Z)
      return false;
  }
  return true;
}

bool Stabilizer::isZType() const {
  for (const auto &p : paulis_) {
    if (p == PauliOperator::X)
      return false;
  }
  return true;
}

std::string Stabilizer::toString() const {
  std::ostringstream oss;
  for (const auto &p : paulis_) {
    switch (p) {
    case PauliOperator::I:
      oss << "I";
      break;
    case PauliOperator::X:
      oss << "X";
      break;
    case PauliOperator::Y:
      oss << "Y";
      break;
    case PauliOperator::Z:
      oss << "Z";
      break;
    }
  }
  return oss.str();
}

} // namespace qec
} // namespace omniq
