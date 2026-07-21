//
// Created for OmniQ physical qudit simulation.
//

#ifndef OMNIQ_QUDITSTATEVECTOR_H
#define OMNIQ_QUDITSTATEVECTOR_H

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <complex>
#include <vector>
#include <string>

namespace omniq {

using SparseMatrixXcd = Eigen::SparseMatrix<std::complex<double>>;
using VectorXcd = Eigen::VectorXcd;

class QuditStatevector {
private:
    int numQudits_;
    int dimension_; // Dimension d per qudit (d >= 2)
    VectorXcd stateVector_;

    void validateQuditIndex(int qudit) const;
    SparseMatrixXcd buildSingleQuditOp(const Eigen::MatrixXcd& gate, int qudit) const;

public:
    QuditStatevector(int numQudits, int dimension);
    QuditStatevector(int numQudits, int dimension, const VectorXcd& amplitudes);

    void normalize();
    double getNorm() const;
    std::string toString() const;

    // Physical Magnon / Qudit Gates
    void applyAharonovCasherRZ(int qudit, double theta);
    void applyCSUM(int controlQudit, int targetQudit);
    void applyControlledPhase(int controlQudit, int targetQudit, double phase);

    // Custom Qudit Operations
    void applyCustomSingleQuditOp(int qudit, const Eigen::MatrixXcd& op);

    // Measurement & Probability
    double getQuditStateProbability(int qudit, int stateLevel) const;
    int measure(int qudit);

    // Accessors
    int getNumQudits() const { return numQudits_; }
    int getDimension() const { return dimension_; }
    int getTotalDimension() const { return static_cast<int>(stateVector_.size()); }
    const VectorXcd& getStateVector() const { return stateVector_; }
    VectorXcd& getStateVector() { return stateVector_; }
};

} // namespace omniq

#endif // OMNIQ_QUDITSTATEVECTOR_H
