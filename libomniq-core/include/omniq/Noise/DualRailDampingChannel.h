//
// Created for OmniQ Dual-Rail Magnon Decay Noise Simulation.
//

#ifndef OMNIQ_NOISE_DUALRAILDAMPINGCHANNEL_H
#define OMNIQ_NOISE_DUALRAILDAMPINGCHANNEL_H

#include "omniq/Noise/NoiseChannel.h"

namespace omniq {
namespace noise {

/**
 * @brief Dual-Rail Magnon Damping Channel: models dissipation to the vacuum state |0,0⟩
 *
 * Models energy loss where magnons in Rail A (|1,0⟩) or Rail B (|0,1⟩) relax
 * and bleed out into non-computational vacuum state |0,0⟩.
 */
class DualRailDampingChannel : public NoiseChannel {
public:
    explicit DualRailDampingChannel(double gamma);

    static DualRailDampingChannel fromT1(double T1, double gateTime);

    void apply(DensityMatrix &rho, int qubit) const override;
    std::vector<MatrixXcd> getKrausOperators() const override;
    std::string getName() const override { return "DualRailDamping"; }
    std::string getDescription() const override;

    double getGamma() const { return gamma_; }
    void setGamma(double gamma) { gamma_ = gamma; }

private:
    double gamma_; // Damping parameter gamma in [0, 1]
};

} // namespace noise
} // namespace omniq

#endif // OMNIQ_NOISE_DUALRAILDAMPINGCHANNEL_H
