//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef OMNIQ_QEC_STABILIZER_H
#define OMNIQ_QEC_STABILIZER_H

#include <string>
#include <vector>

namespace omniq {
namespace qec {

enum class PauliOperator {
  I, // Identity
  X,
  Y,
  Z
};

/**
 * @brief Represents a single stabilizer operator (tensor product of Paulis)
 */
class Stabilizer {
public:
  Stabilizer();
  explicit Stabilizer(const std::vector<PauliOperator> &paulis);

  // Accessors
  int getNumQubits() const { return static_cast<int>(paulis_.size()); }
  PauliOperator getPauli(int qubitIndex) const;
  const std::vector<PauliOperator> &getPaulis() const { return paulis_; }

  // Support qubits (non-identity positions)
  std::vector<int> getSupportQubits() const;
  int getWeight() const; // Number of non-identity operators

  // Type identification
  bool isXType() const; // True if all non-I are X or Y
  bool isZType() const; // True if all non-I are Z or Y

  std::string toString() const;

private:
  std::vector<PauliOperator> paulis_;
};

/**
 * @brief Represents position and type of a stabilizer in the code
 */
struct StabilizerInfo {
  int index; // Unique identifier
  int row;   // Position on lattice
  int col;
  bool isXType;  // X-stabilizer (plaquette) or Z-stabilizer (vertex)
  Stabilizer op; // The actual Pauli operator
};

} // namespace qec
} // namespace omniq

#endif // OMNIQ_QEC_STABILIZER_H
