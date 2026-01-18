//
// Created by Goutham Arcot on 18/01/26.
//

#include "omniq/Spectral.h"
#include <Eigen/Eigenvalues>

namespace omniq {

SpectralAnalysis::EigenDecomposition
SpectralAnalysis::computeEigendecomposition(const Eigen::MatrixXcd &matrix) {
  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> solver(matrix);

  EigenDecomposition result;
  result.eigenvalues = solver.eigenvalues();
  result.eigenvectors = solver.eigenvectors();

  return result;
}

Eigen::MatrixXcd SpectralAnalysis::partialTranspose(const Eigen::MatrixXcd &rho,
                                                    int dimA, int dimB) {
  int dim = dimA * dimB;
  Eigen::MatrixXcd rho_T(dim, dim);

  // Partial transpose on subsystem B
  for (int i1 = 0; i1 < dimA; ++i1) {
    for (int j1 = 0; j1 < dimA; ++j1) {
      for (int i2 = 0; i2 < dimB; ++i2) {
        for (int j2 = 0; j2 < dimB; ++j2) {
          int i = i1 * dimB + i2;
          int j = j1 * dimB + j2;
          int i_T = i1 * dimB + j2; // Transpose indices of B
          int j_T = j1 * dimB + i2;

          rho_T(i_T, j_T) = rho(i, j);
        }
      }
    }
  }

  return rho_T;
}

double SpectralAnalysis::calculateNegativity(const Eigen::MatrixXcd &rho,
                                             int dimA, int dimB) {
  // Compute partial transpose
  Eigen::MatrixXcd rho_T = partialTranspose(rho, dimA, dimB);

  // Compute eigenvalues
  auto decomp = computeEigendecomposition(rho_T);

  // Negativity = sum of negative eigenvalues (absolute value)
  double negativity = 0.0;
  for (int i = 0; i < decomp.eigenvalues.size(); ++i) {
    if (decomp.eigenvalues(i) < 0) {
      negativity += std::abs(decomp.eigenvalues(i));
    }
  }

  return negativity;
}

} // namespace omniq
