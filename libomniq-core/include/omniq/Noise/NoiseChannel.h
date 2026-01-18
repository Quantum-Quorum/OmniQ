//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef OMNIQ_NOISE_NOISECHANNEL_H
#define OMNIQ_NOISE_NOISECHANNEL_H

#include "omniq/DensityMatrix.h"
#include <Eigen/Dense>
#include <memory>
#include <string>
#include <vector>

namespace omniq {
namespace noise {

using Eigen::MatrixXcd;

/**
 * @brief Abstract base class for quantum noise channels
 *
 * Noise channels are implemented using the Kraus operator representation:
 * ρ' = Σ_k E_k ρ E_k†
 *
 * where {E_k} are the Kraus operators satisfying Σ_k E_k† E_k = I
 */
class NoiseChannel {
public:
  virtual ~NoiseChannel() = default;

  /**
   * @brief Apply noise channel to a density matrix
   * @param rho Density matrix to apply noise to (modified in-place)
   * @param qubit Target qubit index for single-qubit channels
   */
  virtual void apply(DensityMatrix &rho, int qubit) const = 0;

  /**
   * @brief Get Kraus operators for this channel
   * @return Vector of Kraus operators {E_0, E_1, ...}
   */
  virtual std::vector<MatrixXcd> getKrausOperators() const = 0;

  /**
   * @brief Get channel name for identification
   */
  virtual std::string getName() const = 0;

  /**
   * @brief Get channel description
   */
  virtual std::string getDescription() const { return getName(); }

  /**
   * @brief Verify channel is completely positive and trace preserving
   * @return True if Σ_k E_k† E_k = I (within numerical precision)
   */
  bool verifyCPTP() const;

protected:
  /**
   * @brief Apply Kraus operators to full density matrix
   * @param rho Full system density matrix
   * @param qubit Target qubit
   * @param kraus Kraus operators (2x2 matrices)
   */
  void applyKrausOperators(DensityMatrix &rho, int qubit,
                           const std::vector<MatrixXcd> &kraus) const;
};

} // namespace noise
} // namespace omniq

#endif // OMNIQ_NOISE_NOISECHANNEL_H
