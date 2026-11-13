//
// Created for OmniQ physical qudit simulation.
//

#include "omniq/QuditStatevector.h"
#include <cmath>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <random>

namespace omniq {

QuditStatevector::QuditStatevector(int numQudits, int dimension)
    : numQudits_(numQudits)
    , dimension_(dimension)
{
    if (numQudits <= 0) {
        throw std::invalid_argument("Number of qudits must be positive");
    }
    if (dimension < 2) {
        throw std::invalid_argument("Qudit dimension must be at least 2");
    }

    int totalDim = static_cast<int>(std::pow(dimension, numQudits));
    stateVector_ = VectorXcd::Zero(totalDim);
    stateVector_(0) = 1.0; // Initialize to |0...0⟩
}

QuditStatevector::QuditStatevector(int numQudits, int dimension, const VectorXcd& amplitudes)
    : numQudits_(numQudits)
    , dimension_(dimension)
    , stateVector_(amplitudes)
{
    int expectedDim = static_cast<int>(std::pow(dimension, numQudits));
    if (amplitudes.size() != expectedDim) {
        throw std::invalid_argument("Amplitude vector size does not match total qudit Hilbert space dimension");
    }
    normalize();
}

void QuditStatevector::validateQuditIndex(int qudit) const {
    if (qudit < 0 || qudit >= numQudits_) {
        throw std::out_of_range("Qudit index out of range");
    }
}

void QuditStatevector::normalize() {
    double norm = stateVector_.norm();
    if (norm > 1e-12) {
        stateVector_ /= norm;
    }
}

double QuditStatevector::getNorm() const {
    return stateVector_.norm();
}

SparseMatrixXcd QuditStatevector::buildSingleQuditOp(const Eigen::MatrixXcd& gate, int qudit) const {
    if (gate.rows() != dimension_ || gate.cols() != dimension_) {
        throw std::invalid_argument("Gate dimensions must match qudit dimension");
    }

    int totalDim = static_cast<int>(stateVector_.size());
    std::vector<Eigen::Triplet<std::complex<double>>> triplets;
    
    // We construct the operator sparse matrix using state index decomposition
    // Index i = s_0 * d^(n-1) + ... + s_q * d^(n-1-q) + ... + s_{n-1}
    int stride = static_cast<int>(std::pow(dimension_, numQudits_ - 1 - qudit));
    int blockStride = stride * dimension_;

    for (int outer = 0; outer < totalDim; outer += blockStride) {
        for (int inner = 0; inner < stride; ++inner) {
            for (int r = 0; r < dimension_; ++r) {
                for (int c = 0; c < dimension_; ++c) {
                    std::complex<double> val = gate(r, c);
                    if (std::abs(val) > 1e-15) {
                        int row = outer + r * stride + inner;
                        int col = outer + c * stride + inner;
                        triplets.emplace_back(row, col, val);
                    }
                }
            }
        }
    }

    SparseMatrixXcd mat(totalDim, totalDim);
    mat.setFromTriplets(triplets.begin(), triplets.end());
    return mat;
}

void QuditStatevector::applyCustomSingleQuditOp(int qudit, const Eigen::MatrixXcd& op) {
    validateQuditIndex(qudit);
    SparseMatrixXcd sMat = buildSingleQuditOp(op, qudit);
    stateVector_ = sMat * stateVector_;
}

void QuditStatevector::applyAharonovCasherRZ(int qudit, double theta) {
    validateQuditIndex(qudit);
    int totalDim = static_cast<int>(stateVector_.size());
    int stride = static_cast<int>(std::pow(dimension_, numQudits_ - 1 - qudit));
    std::vector<std::complex<double>> phases(dimension_);
    for (int m = 0; m < dimension_; ++m) {
        phases[m] = std::exp(std::complex<double>(0, -m * theta));
    }
    for (int idx = 0; idx < totalDim; ++idx) {
        int m = (idx / stride) % dimension_;
        stateVector_(idx) *= phases[m];
    }
}

void QuditStatevector::applyCSUM(int controlQudit, int targetQudit) {
    validateQuditIndex(controlQudit);
    validateQuditIndex(targetQudit);
    if (controlQudit == targetQudit) {
        throw std::invalid_argument("Control and target qudits must be different");
    }

    int totalDim = static_cast<int>(stateVector_.size());
    std::vector<Eigen::Triplet<std::complex<double>>> triplets;

    int cStride = static_cast<int>(std::pow(dimension_, numQudits_ - 1 - controlQudit));
    int tStride = static_cast<int>(std::pow(dimension_, numQudits_ - 1 - targetQudit));

    for (int idx = 0; idx < totalDim; ++idx) {
        int cVal = (idx / cStride) % dimension_;
        int tVal = (idx / tStride) % dimension_;
        
        int newTVal = (tVal + cVal) % dimension_;
        int targetIdx = idx + (newTVal - tVal) * tStride;

        triplets.emplace_back(targetIdx, idx, std::complex<double>(1.0, 0.0));
    }

    SparseMatrixXcd mat(totalDim, totalDim);
    mat.setFromTriplets(triplets.begin(), triplets.end());
    stateVector_ = mat * stateVector_;
}

void QuditStatevector::applyControlledPhase(int controlQudit, int targetQudit, double phase) {
    validateQuditIndex(controlQudit);
    validateQuditIndex(targetQudit);
    if (controlQudit == targetQudit) {
        throw std::invalid_argument("Control and target qudits must be different");
    }

    int totalDim = static_cast<int>(stateVector_.size());
    int cStride = static_cast<int>(std::pow(dimension_, numQudits_ - 1 - controlQudit));
    int tStride = static_cast<int>(std::pow(dimension_, numQudits_ - 1 - targetQudit));

    for (int idx = 0; idx < totalDim; ++idx) {
        int cVal = (idx / cStride) % dimension_;
        int tVal = (idx / tStride) % dimension_;
        if (cVal > 0 && tVal > 0) {
            std::complex<double> pVal = std::exp(std::complex<double>(0.0, cVal * tVal * phase));
            stateVector_(idx) *= pVal;
        }
    }
}

double QuditStatevector::getQuditStateProbability(int qudit, int stateLevel) const {
    validateQuditIndex(qudit);
    if (stateLevel < 0 || stateLevel >= dimension_) {
        throw std::out_of_range("State level out of range for qudit dimension");
    }

    double prob = 0.0;
    int stride = static_cast<int>(std::pow(dimension_, numQudits_ - 1 - qudit));
    int totalDim = static_cast<int>(stateVector_.size());

    for (int idx = 0; idx < totalDim; ++idx) {
        int val = (idx / stride) % dimension_;
        if (val == stateLevel) {
            prob += std::norm(stateVector_(idx));
        }
    }

    return prob;
}

int QuditStatevector::measure(int qudit) {
    validateQuditIndex(qudit);
    
    std::vector<double> probs(dimension_, 0.0);
    double total = 0.0;
    for (int l = 0; l < dimension_; ++l) {
        probs[l] = getQuditStateProbability(qudit, l);
        total += probs[l];
    }
    if (total > 1e-12) {
        for (int l = 0; l < dimension_; ++l) {
            probs[l] /= total;
        }
    } else {
        probs[0] = 1.0;
    }

    // Sample state level based on Born's rule
    static thread_local std::mt19937_64 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double r = dist(rng);
    int sampledLevel = dimension_ - 1;
    double cumulative = 0.0;
    for (int l = 0; l < dimension_; ++l) {
        cumulative += probs[l];
        if (r <= cumulative) {
            sampledLevel = l;
            break;
        }
    }

    // Collapse state vector
    int stride = static_cast<int>(std::pow(dimension_, numQudits_ - 1 - qudit));
    int totalDim = static_cast<int>(stateVector_.size());

    for (int idx = 0; idx < totalDim; ++idx) {
        int val = (idx / stride) % dimension_;
        if (val != sampledLevel) {
            stateVector_(idx) = 0.0;
        }
    }
    normalize();

    return sampledLevel;
}

std::string QuditStatevector::toString() const {
    std::stringstream ss;
    ss << "|ψ_qudit⟩ = ";
    bool first = true;

    for (int i = 0; i < stateVector_.size(); ++i) {
        std::complex<double> amplitude = stateVector_(i);
        if (std::abs(amplitude) > 1e-12) {
            if (!first) ss << " + ";

            std::string stateStr = "";
            int temp = i;
            for (int j = 0; j < numQudits_; ++j) {
                int level = temp % dimension_;
                stateStr = std::to_string(level) + stateStr;
                temp /= dimension_;
            }

            ss << "(" << std::fixed << std::setprecision(4) << amplitude.real()
               << (amplitude.imag() >= 0 ? "+" : "") << amplitude.imag() << "i)|"
               << stateStr << "⟩";
            first = false;
        }
    }
    return ss.str();
}

} // namespace omniq
