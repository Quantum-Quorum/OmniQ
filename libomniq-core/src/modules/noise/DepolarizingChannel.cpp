//
// Created by Goutham Arcot on 18/01/26.
//

#include "omniq/Noise/DepolarizingChannel.h"
#include "omniq/Operators.h"
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace omniq {
namespace noise {

DepolarizingChannel::DepolarizingChannel(double errorProb) : p_(errorProb) {
  if (p_ < 0.0 || p_ > 1.0) {
    throw std::invalid_argument("Error probability must be in [0, 1]");
  }
}

void DepolarizingChannel::setErrorProbability(double p) {
  if (p < 0.0 || p > 1.0) {
    throw std::invalid_argument("Error probability must be in [0, 1]");
  }
  p_ = p;
}

std::vector<MatrixXcd> DepolarizingChannel::getKrausOperators() const {
  std::vector<MatrixXcd> kraus;

  // E_0 = √(1-p) I
  kraus.push_back(std::sqrt(1.0 - p_) * Operators::IDENTITY);

  // E_1 = √(p/3) X
  kraus.push_back(std::sqrt(p_ / 3.0) * Operators::PAULI_X);

  // E_2 = √(p/3) Y
  kraus.push_back(std::sqrt(p_ / 3.0) * Operators::PAULI_Y);

  // E_3 = √(p/3) Z
  kraus.push_back(std::sqrt(p_ / 3.0) * Operators::PAULI_Z);

  return kraus;
}

void DepolarizingChannel::apply(DensityMatrix &rho, int qubit) const {
  applyKrausOperators(rho, qubit, getKrausOperators());
}

std::string DepolarizingChannel::getDescription() const {
  std::ostringstream oss;
  oss << "Depolarizing(p=" << p_ << ")";
  return oss.str();
}

} // namespace noise
} // namespace omniq
