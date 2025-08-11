//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef QPE_H
#define QPE_H

#include <vector>
#include <complex>
#include <functional>
#include <memory>
#include "../../../include/omniq/Circuit.h"

namespace omniq {

/**
 * @brief Unitary operator function type for QPE
 */
using UnitaryOperator = std::function<void(Statevector&, int)>;

/**
 * @brief Quantum Phase Estimation algorithm implementation
 */
class QPE {
private:
    int num_precision_qubits_;
    int num_eigenstate_qubits_;
    UnitaryOperator unitary_;
    std::vector<double> eigenvalues_;
    std::vector<std::complex<double>> eigenstates_;
    
public:
    /**
     * @brief Constructor for QPE
     * @param num_precision_qubits Number of precision qubits
     * @param num_eigenstate_qubits Number of eigenstate qubits
     * @param unitary Unitary operator to estimate phase of
     */
    QPE(int num_precision_qubits, int num_eigenstate_qubits, UnitaryOperator unitary);
    
    /**
     * @brief Set eigenvalues and eigenstates
     * @param eigenvalues Vector of eigenvalues
     * @param eigenstates Vector of eigenstates (as complex amplitudes)
     */
    void set_eigenvalues_and_states(const std::vector<double>& eigenvalues,
                                   const std::vector<std::complex<double>>& eigenstates);
    
    /**
     * @brief Build the complete QPE circuit
     * @return Quantum circuit implementing QPE
     */
    Circuit build_circuit() const;
    
    /**
     * @brief Build the Quantum Fourier Transform part
     * @return QFT circuit
     */
    Circuit build_qft() const;
    
    /**
     * @brief Build the inverse Quantum Fourier Transform part
     * @return Inverse QFT circuit
     */
    Circuit build_inverse_qft() const;
    
    /**
     * @brief Execute QPE algorithm
     * @param initial_state Initial eigenstate (optional)
     * @return Final quantum state
     */
    Statevector execute(const Statevector& initial_state = Statevector(0)) const;
    
    /**
     * @brief Execute with measurements and return phase estimates
     * @param num_shots Number of measurement shots
     * @return Vector of phase estimates (normalized to [0, 1))
     */
    std::vector<double> execute_with_measurements(int num_shots = 1000) const;
    
    /**
     * @brief Get the number of precision qubits
     * @return Number of precision qubits
     */
    int get_num_precision_qubits() const { return num_precision_qubits_; }
    
    /**
     * @brief Get the number of eigenstate qubits
     * @return Number of eigenstate qubits
     */
    int get_num_eigenstate_qubits() const { return num_eigenstate_qubits_; }
    
    /**
     * @brief Get the total number of qubits
     * @return Total number of qubits
     */
    int get_total_qubits() const { return num_precision_qubits_ + num_eigenstate_qubits_; }
};

/**
 * @brief Quantum Fourier Transform gate
 */
class QFTGate : public Gate {
private:
    int num_qubits_;
    bool inverse_;
    
public:
    QFTGate(int num_qubits, bool inverse = false);
    
    void apply(Statevector& state) const override;
    void apply(DensityMatrix& state) const override;
    std::string to_string() const override;
    
private:
    void apply_qft_to_statevector(Statevector& state) const;
    void apply_inverse_qft_to_statevector(Statevector& state) const;
};

/**
 * @brief Controlled unitary gate for QPE
 */
class ControlledUnitaryGate : public Gate {
private:
    int control_qubit_;
    int target_start_;
    int num_target_qubits_;
    UnitaryOperator unitary_;
    int power_;
    
public:
    ControlledUnitaryGate(int control_qubit, int target_start, int num_target_qubits,
                         UnitaryOperator unitary, int power = 1);
    
    void apply(Statevector& state) const override;
    void apply(DensityMatrix& state) const override;
    std::string to_string() const override;
    
private:
    void apply_controlled_unitary_to_statevector(Statevector& state) const;
};

/**
 * @brief Utility functions for QPE
 */
namespace qpe_utils {
    
    /**
     * @brief Create a simple phase rotation unitary
     * @param phase Phase to rotate by
     * @return Unitary operator
     */
    UnitaryOperator create_phase_rotation_unitary(double phase);
    
    /**
     * @brief Create a controlled phase rotation unitary
     * @param phase Phase to rotate by
     * @return Unitary operator
     */
    UnitaryOperator create_controlled_phase_rotation_unitary(double phase);
    
    /**
     * @brief Convert phase measurement to eigenvalue
     * @param phase_measurement Phase measurement (normalized to [0, 1))
     * @param num_precision_qubits Number of precision qubits
     * @return Eigenvalue
     */
    double phase_to_eigenvalue(double phase_measurement, int num_precision_qubits);
    
    /**
     * @brief Convert eigenvalue to phase
     * @param eigenvalue Eigenvalue
     * @param num_precision_qubits Number of precision qubits
     * @return Phase (normalized to [0, 1))
     */
    double eigenvalue_to_phase(double eigenvalue, int num_precision_qubits);
    
    /**
     * @brief Estimate precision of QPE
     * @param num_precision_qubits Number of precision qubits
     * @return Precision in bits
     */
    double estimate_precision(int num_precision_qubits);
    
    /**
     * @brief Calculate success probability for given precision
     * @param num_precision_qubits Number of precision qubits
     * @param true_phase True phase value
     * @return Success probability
     */
    double calculate_success_probability(int num_precision_qubits, double true_phase);
}

} // namespace omniq

#endif // QPE_H
