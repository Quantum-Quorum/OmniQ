//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef OMNIQ_QEC_SURFACECODE_H
#define OMNIQ_QEC_SURFACECODE_H

#include "omniq/QEC/Stabilizer.h"
#include "omniq/QEC/Syndrome.h"
#include "omniq/Statevector.h"
#include <map>
#include <vector>

namespace omniq {
namespace qec {

/**
 * @brief Qubit position on the 2D lattice
 */
struct QubitPosition {
  int row;
  int col;
  int globalIndex; // Index in the overall quantum circuit

  QubitPosition() : row(0), col(0), globalIndex(0) {}
  QubitPosition(int r, int c, int idx) : row(r), col(c), globalIndex(idx) {}
};

/**
 * @brief Surface code implementation with rotated lattice
 */
class SurfaceCode {
public:
  /**
   * @brief Construct surface code with given distance
   * @param distance Code distance (3, 5, 7, etc. - must be odd)
   */
  explicit SurfaceCode(int distance);

  // Accessors
  int getDistance() const { return distance_; }
  int getNumDataQubits() const { return static_cast<int>(dataQubits_.size()); }
  int getNumAncillaQubits() const {
    return xAncillaQubits_.size() + zAncillaQubits_.size();
  }
  int getTotalQubits() const {
    return getNumDataQubits() + getNumAncillaQubits();
  }

  const std::vector<QubitPosition> &getDataQubits() const {
    return dataQubits_;
  }
  const std::vector<QubitPosition> &getXAncillaQubits() const {
    return xAncillaQubits_;
  }
  const std::vector<QubitPosition> &getZAncillaQubits() const {
    return zAncillaQubits_;
  }

  const std::vector<StabilizerInfo> &getXStabilizers() const {
    return xStabilizers_;
  }
  const std::vector<StabilizerInfo> &getZStabilizers() const {
    return zStabilizers_;
  }

  /**
   * @brief Measure all stabilizers and extract syndrome
   * @param state Current quantum state (includes ancilla qubits)
   * @return Syndrome vector (X-stabilizers first, then Z-stabilizers)
   */
  Syndrome measureSyndromes(const Statevector &state);

  /**
   * @brief Apply correction based on error chain
   * @param errorChain List of data qubit indices to flip
   * @param state Quantum state to correct (modified in-place)
   */
  void applyCorrection(const std::vector<int> &errorChain, Statevector &state);

  /**
   * @brief Get neighboring data qubits for a given ancilla
   */
  std::vector<int> getStabilizerSupport(int stabilizerIndex,
                                        bool isXType) const;

  /**
   * @brief Prepare logical |0⟩ state
   */
  Statevector prepareLogicalZero() const;

  /**
   * @brief Prepare logical |+⟩ state
   */
  Statevector prepareLogicalPlus() const;

  /**
   * @brief Check if state is in logical |0⟩ subspace
   */
  bool isLogicalZero(const Statevector &state) const;

private:
  void initializeLattice();
  void constructStabilizers();

  int distance_;

  // Qubit layout
  std::vector<QubitPosition> dataQubits_;
  std::vector<QubitPosition> xAncillaQubits_; // Plaquette stabilizers (X-type)
  std::vector<QubitPosition> zAncillaQubits_; // Vertex stabilizers (Z-type)

  // Stabilizer definitions
  std::vector<StabilizerInfo> xStabilizers_;
  std::vector<StabilizerInfo> zStabilizers_;

  // Maps for efficient lookup
  std::map<std::pair<int, int>, int> positionToDataQubit_;
  std::map<std::pair<int, int>, int> positionToXAncilla_;
  std::map<std::pair<int, int>, int> positionToZAncilla_;
};

} // namespace qec
} // namespace omniq

#endif // OMNIQ_QEC_SURFACECODE_H
