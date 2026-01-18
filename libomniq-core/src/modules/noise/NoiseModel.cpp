//
// Created by Goutham Arcot on 18/01/26.
//

#include "omniq/Noise/NoiseModel.h"
#include <cmath>
#include <random>
#include <sstream>

namespace omniq {
namespace noise {

NoiseModel::NoiseModel() {
  // Use default hardware parameters and gate fidelities
}

void NoiseModel::setHardwareParams(const HardwareParams &params) {
  hwParams_ = params;
}

void NoiseModel::setGateFidelities(const GateFidelities &fidelities) {
  gateFidelities_ = fidelities;
}

double NoiseModel::calculateDepolarizingError(double fidelity) const {
  // Convert fidelity to depolarizing error probability
  // F = 1 - p(1 - 1/d) for d-dimensional system
  // For qubits (d=2): p = 2(1-F)/3 (approximate)
  if (fidelity >= 1.0)
    return 0.0;
  return (4.0 / 3.0) * (1.0 - fidelity);
}

void NoiseModel::applyGateNoise(DensityMatrix &rho, const std::string &gateName,
                                const std::vector<int> &qubits) const {
  if (!enabled_)
    return;
  if (rho.getNumQubits() != 1) {
    // Multi-qubit noise not yet implemented
    return;
  }

  double fidelity;
  double gateTime;

  // Determine fidelity and gate time based on gate type
  if (gateName == "CNOT" || gateName == "CZ" || gateName == "SWAP") {
    fidelity = gateFidelities_.twoQubit;
    gateTime = hwParams_.twoQubitGateTime;
  } else {
    fidelity = gateFidelities_.singleQubit;
    gateTime = hwParams_.singleQubitGateTime;
  }

  // Apply depolarizing noise based on gate fidelity
  double depolarizingProb = calculateDepolarizingError(fidelity);
  if (depolarizingProb > 0.0) {
    DepolarizingChannel depolarizing(depolarizingProb);
    depolarizing.apply(rho, qubits[0]);
  }

  // Apply decoherence during gate execution
  applyIdleNoise(rho, qubits[0], gateTime);
}

void NoiseModel::applyIdleNoise(DensityMatrix &rho, int qubit,
                                double idleTime) const {
  if (!enabled_ || idleTime <= 0.0)
    return;
  if (rho.getNumQubits() != 1) {
    // Multi-qubit noise not yet implemented
    return;
  }

  // Apply T1 relaxation (amplitude damping)
  double gamma_T1 = 1.0 - std::exp(-idleTime / hwParams_.T1);
  if (gamma_T1 > 0.0) {
    AmplitudeDampingChannel relaxation(gamma_T1);
    relaxation.apply(rho, qubit);
  }

  // Apply T2 dephasing (phase damping)
  // Note: T2* = (1/T2 - 1/2T1)^-1 for pure dephasing
  double T2_star = 1.0 / (1.0 / hwParams_.T2 - 1.0 / (2.0 * hwParams_.T1));
  double lambda_T2 = 1.0 - std::exp(-idleTime / T2_star);
  if (lambda_T2 > 0.0) {
    PhaseDampingChannel dephasing(lambda_T2);
    dephasing.apply(rho, qubit);
  }
}

int NoiseModel::applyMeasurementNoise(int measurementResult) const {
  if (!enabled_)
    return measurementResult;

  // Apply readout error with probability (1 - fidelity)
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0.0, 1.0);

  double errorProb = 1.0 - hwParams_.readoutFidelity;
  if (dis(gen) < errorProb) {
    return 1 - measurementResult; // Flip the result
  }

  return measurementResult;
}

NoiseModel NoiseModel::createIdealModel() {
  NoiseModel model;
  model.setEnabled(false);
  return model;
}

NoiseModel NoiseModel::createTypicalModel() {
  NoiseModel model;
  // Use default parameters (already typical values)
  return model;
}

NoiseModel NoiseModel::createNoisyModel() {
  NoiseModel model;

  HardwareParams params;
  params.T1 = 20e-6;                   // Shorter relaxation time
  params.T2 = 30e-6;                   // Shorter dephasing time
  params.singleQubitGateTime = 100e-9; // Slower gates
  params.twoQubitGateTime = 400e-9;
  params.readoutFidelity = 0.90;
  model.setHardwareParams(params);

  GateFidelities fidelities;
  fidelities.singleQubit = 0.995; // 99.5%
  fidelities.twoQubit = 0.95;     // 95%
  fidelities.measurement = 0.90;  // 90%
  model.setGateFidelities(fidelities);

  return model;
}

std::string NoiseModel::toString() const {
  std::ostringstream oss;
  oss << "NoiseModel(\n";
  oss << "  T1=" << hwParams_.T1 * 1e6 << "μs, ";
  oss << "T2=" << hwParams_.T2 * 1e6 << "μs\n";
  oss << "  Single-qubit fidelity=" << gateFidelities_.singleQubit << "\n";
  oss << "  Two-qubit fidelity=" << gateFidelities_.twoQubit << "\n";
  oss << "  Readout fidelity=" << hwParams_.readoutFidelity << "\n";
  oss << "  Enabled=" << (enabled_ ? "true" : "false") << "\n";
  oss << ")";
  return oss.str();
}

} // namespace noise
} // namespace omniq
