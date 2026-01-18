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

  // Apply each Kraus operator: ρ' += E_k ⊗ I ... ⊗ I × ρ × (E_k ⊗ I ... ⊗ I)†
  for (const auto &E_k : kraus) {
    // Build full operator E_k ⊗ I_{n-1}

    // This is a simplified approach - proper implementation would use
    // tensor product construction for the specific qubit index
    // For now, assume single qubit system or apply to qubit 0
    if (numQubits == 1) {
      new_rho += E_k * rho_matrix * E_k.adjoint();
    } else {
      // TODO: Implement proper multi-qubit Kraus application
      // For now, only support single-qubit density matrices
      throw std::runtime_error("Multi-qubit noise not yet fully implemented");
    }
  }

  // Update density matrix directly
  rho.getDensityMatrix() = new_rho;
}

} // namespace noise
} // namespace omniq
