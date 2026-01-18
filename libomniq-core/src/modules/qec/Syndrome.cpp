//
// Created by Goutham Arcot on 18/01/26.
//

#include "omniq/QEC/Syndrome.h"
#include <sstream>
#include <stdexcept>

namespace omniq {
namespace qec {

Syndrome::Syndrome() : codeDistance_(0) {}

Syndrome::Syndrome(int numStabilizers)
    : measurements_(numStabilizers, 1), codeDistance_(0) {
  // Initialize all to +1 (satisfied)
}

int Syndrome::getMeasurement(int index) const {
  if (index < 0 || index >= static_cast<int>(measurements_.size())) {
    throw std::out_of_range("Syndrome index out of range");
  }
  return measurements_[index];
}

void Syndrome::setMeasurement(int index, int value) {
  if (index < 0 || index >= static_cast<int>(measurements_.size())) {
    throw std::out_of_range("Syndrome index out of range");
  }
  if (value != 1 && value != -1) {
    throw std::invalid_argument("Syndrome measurement must be +1 or -1");
  }
  measurements_[index] = value;
}

void Syndrome::setMeasurements(const std::vector<int> &measurements) {
  measurements_ = measurements;
}

int Syndrome::countViolations() const {
  int count = 0;
  for (int m : measurements_) {
    if (m == -1)
      count++;
  }
  return count;
}

std::vector<int> Syndrome::getViolatedStabilizers() const {
  std::vector<int> violated;
  for (size_t i = 0; i < measurements_.size(); ++i) {
    if (measurements_[i] == -1) {
      violated.push_back(static_cast<int>(i));
    }
  }
  return violated;
}

bool Syndrome::isAllZero() const {
  for (int m : measurements_) {
    if (m == -1)
      return false;
  }
  return true;
}

std::string Syndrome::toString() const {
  std::ostringstream oss;
  oss << "Syndrome(violations=" << countViolations() << ", [";
  for (size_t i = 0; i < measurements_.size(); ++i) {
    oss << (measurements_[i] == 1 ? "+" : "-");
    if (i < measurements_.size() - 1)
      oss << " ";
  }
  oss << "])";
  return oss.str();
}

} // namespace qec
} // namespace omniq
