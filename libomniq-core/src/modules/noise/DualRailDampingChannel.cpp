//
// Created for OmniQ Dual-Rail Magnon Decay Noise Simulation.
//

#include "omniq/Noise/DualRailDampingChannel.h"
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace omniq {
namespace noise {

DualRailDampingChannel::DualRailDampingChannel(double gamma) : gamma_(gamma) {
    if (gamma_ < 0.0 || gamma_ > 1.0) {
        throw std::invalid_argument("Gamma must be in [0, 1]");
    }
}

DualRailDampingChannel DualRailDampingChannel::fromT1(double T1, double gateTime) {
    double gamma = 1.0 - std::exp(-gateTime / T1);
    return DualRailDampingChannel(gamma);
}

std::vector<MatrixXcd> DualRailDampingChannel::getKrausOperators() const {
    std::vector<MatrixXcd> kraus;

    // Basis for 2 rails (Mode A, Mode B):
    // 0: |0,0>, 1: |0,1> (Logical 1), 2: |1,0> (Logical 0), 3: |1,1>

    // K_0: No decay
    MatrixXcd K0 = MatrixXcd::Zero(4, 4);
    K0(0, 0) = 1.0;
    K0(1, 1) = std::sqrt(1.0 - gamma_);
    K0(2, 2) = std::sqrt(1.0 - gamma_);
    K0(3, 3) = 1.0 - gamma_;
    kraus.push_back(K0);

    // K_1: Decay from Rail A (|1,0> -> |0,0>)
    MatrixXcd K1 = MatrixXcd::Zero(4, 4);
    K1(0, 2) = std::sqrt(gamma_);
    K1(1, 3) = std::sqrt(gamma_ * (1.0 - gamma_));
    kraus.push_back(K1);

    // K_2: Decay from Rail B (|0,1> -> |0,0>)
    MatrixXcd K2 = MatrixXcd::Zero(4, 4);
    K2(0, 1) = std::sqrt(gamma_);
    K2(2, 3) = std::sqrt(gamma_ * (1.0 - gamma_));
    kraus.push_back(K2);

    return kraus;
}

void DualRailDampingChannel::apply(DensityMatrix &rho, int qubit) const {
    applyKrausOperators(rho, qubit, getKrausOperators());
}

std::string DualRailDampingChannel::getDescription() const {
    std::ostringstream oss;
    oss << "DualRailDamping(γ=" << gamma_ << ")";
    return oss.str();
}

} // namespace noise
} // namespace omniq
