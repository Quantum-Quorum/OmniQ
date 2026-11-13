//
// Created by Goutham Arcot on 18/01/26.
//

#include "omniq/Noise/NoiseChannel.h"
#include "omniq/Operators.h"
#include <cmath>

namespace omniq {
namespace noise {

bool NoiseChannel::verifyCPTP() const {
  auto kraus = getKrausOperators();
  if (kraus.empty())
    return false;

  // Verify Σ_k E_k† E_k = I
  MatrixXcd sum = MatrixXcd::Zero(2, 2);
  for (const auto &E_k : kraus) {
    sum += E_k.adjoint() * E_k;
  }

  MatrixXcd identity = MatrixXcd::Identity(2, 2);
  double error = (sum - identity).norm();

  return error < 1e-10;
}

void NoiseChannel::applyKrausOperators(
    DensityMatrix &rho, int qubit, const std::vector<MatrixXcd> &kraus) const {
  int numQubits = rho.getNumQubits();
  int dim = 1 << numQubits;

  // Get current density matrix
  const MatrixXcd &rho_matrix = rho.getDensityMatrix();
  MatrixXcd new_rho = MatrixXcd::Zero(dim, dim);

  // Apply each Kraus operator: ρ' += E_k_full × ρ × E_k_full†
  for (const auto &E_k : kraus) {
    if (E_k.rows() == dim && E_k.cols() == dim) {
      new_rho += E_k * rho_matrix * E_k.adjoint();
    } else if (E_k.rows() == 2 && E_k.cols() == 2) {
      // Single-qubit Kraus operator embedded in multi-qubit system
      MatrixXcd Ek_full = MatrixXcd::Identity(1, 1);
      for (int i = 0; i < numQubits; ++i) {
        if (i == qubit) {
          Ek_full = Operators::tensorProduct(Ek_full, E_k);
        } else {
          Ek_full = Operators::tensorProduct(Ek_full, Operators::IDENTITY);
        }
      }
      new_rho += Ek_full * rho_matrix * Ek_full.adjoint();
    } else {
      throw std::invalid_argument(
          "Kraus operator dimensions incompatible with target density matrix");
    }
  }

  // Update density matrix directly
  rho.getDensityMatrix() = new_rho;
}

} // namespace noise
} // namespace omniq
