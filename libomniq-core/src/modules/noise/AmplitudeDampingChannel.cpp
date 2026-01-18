//
// Created by Goutham Arcot on 18/01/26.
//

#include "omniq/Noise/AmplitudeDampingChannel.h"
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace omniq {
namespace noise {

AmplitudeDampingChannel::AmplitudeDampingChannel(double gamma) : gamma_(gamma) {
  if (gamma_ < 0.0 || gamma_ > 1.0) {
    throw std::invalid_argument("Gamma must be in [0, 1]");
  }
}

AmplitudeDampingChannel AmplitudeDampingChannel::fromT1(double T1,
                                                        double gateTime) {
  double gamma = 1.0 - std::exp(-gateTime / T1);
  return AmplitudeDampingChannel(gamma);
}

void AmplitudeDampingChannel::setGamma(double gamma) {
  if (gamma < 0.0 || gamma > 1.0) {
    throw std::invalid_argument("Gamma must be in [0, 1]");
  }
  gamma_ = gamma;
}

std::vector<MatrixXcd> AmplitudeDampingChannel::getKrausOperators() const {
  std::vector<MatrixXcd> kraus;

  // E_0 = [[1, 0], [0, √(1-γ)]]
  MatrixXcd E0(2, 2);
  E0 << 1.0, 0.0, 0.0, std::sqrt(1.0 - gamma_);
  kraus.push_back(E0);

  // E_1 = [[0, √γ], [0, 0]]
  MatrixXcd E1(2, 2);
  E1 << 0.0, std::sqrt(gamma_), 0.0, 0.0;
  kraus.push_back(E1);

  return kraus;
}

void AmplitudeDampingChannel::apply(DensityMatrix &rho, int qubit) const {
  applyKrausOperators(rho, qubit, getKrausOperators());
}

std::string AmplitudeDampingChannel::getDescription() const {
  std::ostringstream oss;
  oss << "AmplitudeDamping(γ=" << gamma_ << ")";
  return oss.str();
}

} // namespace noise
} // namespace omniq
