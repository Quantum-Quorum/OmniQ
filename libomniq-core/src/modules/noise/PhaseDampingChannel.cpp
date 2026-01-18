//
// Created by Goutham Arcot on 18/01/26.
//

#include "omniq/Noise/PhaseDampingChannel.h"
#include "omniq/Operators.h"
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace omniq {
namespace noise {

PhaseDampingChannel::PhaseDampingChannel(double lambda) : lambda_(lambda) {
  if (lambda_ < 0.0 || lambda_ > 1.0) {
    throw std::invalid_argument("Lambda must be in [0, 1]");
  }
}

PhaseDampingChannel PhaseDampingChannel::fromT2(double T2, double gateTime) {
  double lambda = 1.0 - std::exp(-gateTime / T2);
  return PhaseDampingChannel(lambda);
}

void PhaseDampingChannel::setLambda(double lambda) {
  if (lambda < 0.0 || lambda > 1.0) {
    throw std::invalid_argument("Lambda must be in [0, 1]");
  }
  lambda_ = lambda;
}

std::vector<MatrixXcd> PhaseDampingChannel::getKrausOperators() const {
  std::vector<MatrixXcd> kraus;

  // E_0 = √(1-λ) I
  kraus.push_back(std::sqrt(1.0 - lambda_) * Operators::IDENTITY);

  // E_1 = √λ Z
  kraus.push_back(std::sqrt(lambda_) * Operators::PAULI_Z);

  return kraus;
}

void PhaseDampingChannel::apply(DensityMatrix &rho, int qubit) const {
  applyKrausOperators(rho, qubit, getKrausOperators());
}

std::string PhaseDampingChannel::getDescription() const {
  std::ostringstream oss;
  oss << "PhaseDamping(λ=" << lambda_ << ")";
  return oss.str();
}

} // namespace noise
} // namespace omniq
