//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef OMNIQ_NOISE_AMPLITUDEDAMPINGCHANNEL_H
#define OMNIQ_NOISE_AMPLITUDEDAMPINGCHANNEL_H

#include "omniq/Noise/NoiseChannel.h"

namespace omniq {
namespace noise {

/**
 * @brief Amplitude damping channel: models T1 relaxation (|1⟩ → |0⟩)
 *
 * Models energy dissipation to the environment. The damping parameter γ
 * relates to T1 relaxation time: γ = 1 - exp(-t/T1)
 *
 * Kraus operators:
 * E_0 = [[1, 0], [0, √(1-γ)]]
 * E_1 = [[0, √γ], [0, 0]]
 */
class AmplitudeDampingChannel : public NoiseChannel {
public:
  /**
   * @brief Construct amplitude damping channel
   * @param gamma Damping parameter γ ∈ [0, 1]
   */
  explicit AmplitudeDampingChannel(double gamma);

  /**
   * @brief Construct from T1 time and gate duration
   * @param T1 Relaxation time (seconds)
   * @param gateTime Gate duration (seconds)
   */
  static AmplitudeDampingChannel fromT1(double T1, double gateTime);

  void apply(DensityMatrix &rho, int qubit) const override;
  std::vector<MatrixXcd> getKrausOperators() const override;
  std::string getName() const override { return "AmplitudeDamping"; }

  std::string getDescription() const override;

  double getGamma() const { return gamma_; }
  void setGamma(double gamma);

private:
  double gamma_; // Damping parameter
};

} // namespace noise
} // namespace omniq

#endif // OMNIQ_NOISE_AMPLITUDEDAMPINGCHANNEL_H
