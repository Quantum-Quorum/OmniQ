//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef OMNIQ_SIMULATORS_CLIFFORDSIMULATOR_H
#define OMNIQ_SIMULATORS_CLIFFORDSIMULATOR_H

#include <Eigen/Dense>
#include <string>
#include <vector>

namespace omniq {
namespace simulators {

/**
 * @brief Fast Clifford circuit simulator using stabilizer tableau
 *
 * Simulates Clifford circuits (H, S, CNOT, Pauli gates) in O(n²) time per gate.
 * Can handle thousands of qubits efficiently.
 *
 * Tableau representation tracks n stabilizer generators and destabilizers.
 */
class CliffordSimulator {
public:
  explicit CliffordSimulator(int numQubits);

  // Clifford gates (all O(n) operations)
  void applyH(int qubit);
  void applyS(int qubit);
  void applySdag(int qubit);
  void applyCNOT(int control, int target);
  void applyCZ(int q1, int q2);
  void applyX(int qubit);
  void applyY(int qubit);
  void applyZ(int qubit);

  // Measurement (O(n²) worst case)
  int measure(int qubit);

  // State queries
  int getNumQubits() const { return numQubits_; }
  bool isPureState() const;

  // Reset to |0...0⟩
  void reset();

  // Tableau access (for debugging/analysis)
  const Eigen::MatrixXi &getXTableau() const { return x_; }
  const Eigen::MatrixXi &getZTableau() const { return z_; }
  const Eigen::VectorXi &getRVector() const { return r_; }

  std::string toString() const;

private:
  int numQubits_;

  // Stabilizer tableau: (x|z|r)
  // x_[i] = X part of i-th stabilizer
  // z_[i] = Z part of i-th stabilizer
  // r_[i] = phase (0 or 1) where phase = i^r
  Eigen::MatrixXi x_; // (2n × n) binary matrix
  Eigen::MatrixXi z_; // (2n × n) binary matrix
  Eigen::VectorXi r_; // (2n) phase vector

  // Helper: Row operations
  void rowsum(int h, int i);
  int g(int x1, int z1, int x2, int z2) const;

  // Random measurement outcomes
  std::vector<int> measurementHistory_;
};

} // namespace simulators
} // namespace omniq

#endif // OMNIQ_SIMULATORS_CLIFFORDSIMULATOR_H
