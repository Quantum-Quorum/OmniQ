//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef OMNIQ_NOISE_NOISEMODEL_H
#define OMNIQ_NOISE_NOISEMODEL_H

#include "omniq/Noise/AmplitudeDampingChannel.h"
#include "omniq/Noise/DepolarizingChannel.h"
#include "omniq/Noise/NoiseChannel.h"
#include "omniq/Noise/PhaseDampingChannel.h"
#include <map>
#include <memory>
#include <string>

namespace omniq {
namespace noise {

/**
 * @brief Configurable noise model for realistic quantum simulation
 *
 * Combines multiple noise sources:
 * - Gate errors (depolarizing noise after each gate)
 * - Idle errors (T1/T2 decoherence during idle periods)
 * - Measurement errors (readout fidelity)
 */
class NoiseModel {
public:
  NoiseModel();

  // Hardware parameters (typical superconducting qubit values)
  struct HardwareParams {
    double T1 = 50e-6;                  // Relaxation time (50 μs)
    double T2 = 70e-6;                  // Dephasing time (70 μs)
    double singleQubitGateTime = 50e-9; // Single-qubit gate time (50 ns)
    double twoQubitGateTime = 200e-9;   // Two-qubit gate time (200 ns)
    double readoutFidelity = 0.95;      // Measurement fidelity
  };

  // Gate fidelities (1.0 = perfect, 0.0 = completely noisy)
  struct GateFidelities {
    double singleQubit = 0.9999; // 99.99% fidelity
    double twoQubit = 0.99;      // 99% fidelity
    double measurement = 0.95;   // 95% fidelity
  };

  /**
   * @brief Set hardware parameters
   */
  void setHardwareParams(const HardwareParams &params);
  const HardwareParams &getHardwareParams() const { return hwParams_; }

  /**
   * @brief Set gate fidelities
   */
  void setGateFidelities(const GateFidelities &fidelities);
  const GateFidelities &getGateFidelities() const { return gateFidelities_; }

  /**
   * @brief Apply noise after a gate operation
   * @param rho Density matrix
   * @param gateName Gate type ("H", "CNOT", etc.)
   * @param qubits Qubits involved in the gate
   */
  void applyGateNoise(DensityMatrix &rho, const std::string &gateName,
                      const std::vector<int> &qubits) const;

  /**
   * @brief Apply decoherence noise during idle time
   * @param rho Density matrix
   * @param qubit Target qubit
   * @param idleTime Duration of idle period (seconds)
   */
  void applyIdleNoise(DensityMatrix &rho, int qubit, double idleTime) const;

  /**
   * @brief Apply measurement error
   * @param measurementResult Original measurement (0 or 1)
   * @return Potentially flipped result based on readout fidelity
   */
  int applyMeasurementNoise(int measurementResult) const;

  /**
   * @brief Enable/disable noise simulation
   */
  void setEnabled(bool enabled) { enabled_ = enabled; }
  bool isEnabled() const { return enabled_; }

  /**
   * @brief Create preset noise models
   */
  static NoiseModel createIdealModel();   // No noise
  static NoiseModel createTypicalModel(); // Typical superconducting qubits
  static NoiseModel createNoisyModel();   // Higher noise for testing

  std::string toString() const;

private:
  HardwareParams hwParams_;
  GateFidelities gateFidelities_;
  bool enabled_ = true;

  double calculateDepolarizingError(double fidelity) const;
};

} // namespace noise
} // namespace omniq

#endif // OMNIQ_NOISE_NOISEMODEL_H
