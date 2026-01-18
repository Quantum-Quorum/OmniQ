//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef OMNIQ_NOISE_PHASEDAMPINGCHANNEL_H
#define OMNIQ_NOISE_PHASEDAMPINGCHANNEL_H

#include "omniq/Noise/NoiseChannel.h"

namespace omniq {
namespace noise {

/**
 * @brief Phase damping channel: models T2 dephasing (loss of coherence)
 *
 * Models pure dephasing without energy relaxation. The dephasing parameter λ
 * relates to T2 time: λ = 1 - exp(-t/T2)
 *
 * Kraus operators:
 * E_0 = √(1-λ) I
 * E_1 = √λ Z
 */
class PhaseDampingChannel : public NoiseChannel {
public:
  /**
   * @brief Construct phase damping channel
   * @param lambda Dephasing parameter λ ∈ [0, 1]
   */
  explicit PhaseDampingChannel(double lambda);

  /**
   * @brief Construct from T2 time and gate duration
   * @param T2 Dephasing time (seconds)
   * @param gateTime Gate duration (seconds)
   */
  static PhaseDampingChannel fromT2(double T2, double gateTime);

  void apply(DensityMatrix &rho, int qubit) const override;
  std::vector<MatrixXcd> getKrausOperators() const override;
  std::string getName() const override { return "PhaseDamping"; }

  std::string getDescription() const override;

  double getLambda() const { return lambda_; }
  void setLambda(double lambda);

private:
  double lambda_; // Dephasing parameter
};

} // namespace noise
} // namespace omniq

#endif // OMNIQ_NOISE_PHASEDAMPINGCHANNEL_H
