//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef OMNIQ_DENSITYMATRIX_H
#define OMNIQ_DENSITYMATRIX_H

#include <Eigen/Dense>
#include <complex>
#include <string>
#include <vector>

// Include Statevector header
#include "omniq/Statevector.h"

namespace omniq {

class DensityMatrix {
private:
  int numQubits_;
  MatrixXcd densityMatrix_;

  // Helper methods
  void validateQubitIndex(int qubit) const;
  MatrixXcd createSingleQubitGate(const Matrix2cd &gate, int qubit) const;
  MatrixXcd createTwoQubitGate(const Matrix4cd &gate, int qubit1,
                               int qubit2) const;

public:
  // Constructors
  explicit DensityMatrix(int numQubits);
  explicit DensityMatrix(const MatrixXcd &matrix);
  explicit DensityMatrix(const Statevector &statevector);
  DensityMatrix(const DensityMatrix &other);
  DensityMatrix &operator=(const DensityMatrix &other);

  // State operations
  void normalize();
  double getTrace() const;
  double getPurity() const;
  double getVonNeumannEntropy() const;
  std::string toString() const;

  // Quantum operations
  void applyHadamard(int qubit);
  void applyCNOT(int control, int target);
  void applyPauliX(int qubit);
  void applyPauliY(int qubit);
  void applyPauliZ(int qubit);
  void applyPhaseShift(int qubit, double angle);
  void applyRotationX(int qubit, double angle);
  void applyRotationY(int qubit, double angle);
  void applyRotationZ(int qubit, double angle);

  // Measurements
  double measureExpectation(int qubit, const std::string &observable);

  // State analysis
  DensityMatrix tensorProduct(const DensityMatrix &other) const;
  MatrixXcd partialTrace(const std::vector<int> &qubitsToTrace) const;
  double getQubitProbability(int qubit, int value) const;

  // State properties
  bool isPure() const;
  bool isMixed() const;
  bool isValid() const;

  // Accessors
  int getNumQubits() const { return numQubits_; }
  const MatrixXcd &getDensityMatrix() const { return densityMatrix_; }
  MatrixXcd &getDensityMatrix() { return densityMatrix_; }
};

} // namespace omniq

#endif // OMNIQ_DENSITYMATRIX_H