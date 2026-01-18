//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef OMNIQ_NOISE_DEPOLARIZINGCHANNEL_H
#define OMNIQ_NOISE_DEPOLARIZINGCHANNEL_H

#include "omniq/Noise/NoiseChannel.h"

namespace omniq {
namespace noise {

/**
 * @brief Depolarizing channel: replaces state with maximally mixed state
 *
 * The depolarizing channel with error probability p is defined by:
 * ρ → (1-p)ρ + p/3(XρX + YρY + ZρZ)
 *
 * Kraus operators:
 * E_0 = √(1-p) I
 * E_1 = √(p/3) X
 * E_2 = √(p/3) Y
 * E_3 = √(p/3) Z
 */
class DepolarizingChannel : public NoiseChannel {
public:
  /**
   * @brief Construct depolarizing channel
   * @param errorProb Error probability p ∈ [0, 1]
   */
  explicit DepolarizingChannel(double errorProb);

  void apply(DensityMatrix &rho, int qubit) const override;
  std::vector<MatrixXcd> getKrausOperators() const override;
  std::string getName() const override { return "Depolarizing"; }

  std::string getDescription() const override;

  double getErrorProbability() const { return p_; }
  void setErrorProbability(double p);

private:
  double p_; // Error probability
};

} // namespace noise
} // namespace omniq

#endif // OMNIQ_NOISE_DEPOLARIZINGCHANNEL_H
