//
// Created by Goutham Arcot on 18/01/26.
//

#ifndef OMNIQ_SPECTRAL_H
#define OMNIQ_SPECTRAL_H

#include <Eigen/Dense>
#include <complex>
#include <vector>

namespace omniq {

/**
 * @brief Spectral analysis utilities for quantum systems
 */
class SpectralAnalysis {
public:
  struct EigenDecomposition {
    Eigen::VectorXd eigenvalues;
    Eigen::MatrixXcd eigenvectors;
  };

  /**
   * @brief Compute eigendecomposition of Hermitian matrix
   */
  static EigenDecomposition
  computeEigendecomposition(const Eigen::MatrixXcd &matrix);

  /**
   * @brief Compute partial transpose (for negativity calculation)
   */
  static Eigen::MatrixXcd partialTranspose(const Eigen::MatrixXcd &rho,
                                           int systemADim, int systemBDim);

  /**
   * @brief Calculate negativity (entanglement measure)
   */
  static double calculateNegativity(const Eigen::MatrixXcd &rho, int systemADim,
                                    int systemBDim);
};

} // namespace omniq

#endif // OMNIQ_SPECTRAL_H
