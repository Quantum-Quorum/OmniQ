//
// Created by Goutham Arcot on 17/07/25.
//

#ifndef OMNIQ_CIRCUIT_H
#define OMNIQ_CIRCUIT_H

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <functional>
#include "QuantumStates.h"

namespace omniq {

// Forward declarations
class Gate;
class Measurement;

/**
 * @brief Enumeration of gate types
 */
enum class GateType {
    HADAMARD,
    PAULI_X,
    PAULI_Y,
    PAULI_Z,
    CNOT,
    SWAP,
    PHASE_SHIFT,
    ROTATION_X,
    ROTATION_Y,
    ROTATION_Z,
    CUSTOM
};

/**
 * @brief Base class for quantum gates
 */
class Gate {
protected:
    GateType type_;
    std::vector<int> qubits_;
    std::vector<double> parameters_;
    std::string name_;

public:
    Gate(GateType type, const std::vector<int>& qubits, 
         const std::vector<double>& parameters = {}, const std::string& name = "");
    virtual ~Gate() = default;
    
    GateType get_type() const { return type_; }
    const std::vector<int>& get_qubits() const { return qubits_; }
    const std::vector<double>& get_parameters() const { return parameters_; }
    const std::string& get_name() const { return name_; }
    
    virtual void apply(Statevector& state) const = 0;
    virtual void apply(DensityMatrix& state) const = 0;
    virtual std::string to_string() const = 0;
};

/**
 * @brief Measurement operation
 */
class Measurement {
private:
    int qubit_;
    std::string basis_;
    std::string observable_;

public:
    Measurement(int qubit, const std::string& basis = "computational", 
                const std::string& observable = "Z");
    
    int get_qubit() const { return qubit_; }
    const std::string& get_basis() const { return basis_; }
    const std::string& get_observable() const { return observable_; }
    
    int execute(Statevector& state) const;
    double execute_expectation(DensityMatrix& state) const;
    std::string to_string() const;
};

/**
 * @brief Quantum circuit representation
 */
class Circuit {
private:
    int num_qubits_;
    int num_classical_bits_;
    std::vector<std::shared_ptr<Gate>> gates_;
    std::vector<std::shared_ptr<Measurement>> measurements_;
    std::map<std::string, std::shared_ptr<Gate>> custom_gates_;
    
public:
    explicit Circuit(int num_qubits, int num_classical_bits = 0);
    Circuit(const Circuit& other);
    Circuit& operator=(const Circuit& other);
    
    // Circuit construction
    void add_gate(std::shared_ptr<Gate> gate);
    void add_measurement(std::shared_ptr<Measurement> measurement);
    void add_hadamard(int qubit);
    void add_pauli_x(int qubit);
    void add_pauli_y(int qubit);
    void add_pauli_z(int qubit);
    void add_cnot(int control, int target);
    void add_swap(int qubit1, int qubit2);
    void add_phase_shift(int qubit, double angle);
    void add_rotation_x(int qubit, double angle);
    void add_rotation_y(int qubit, double angle);
    void add_rotation_z(int qubit, double angle);
    void add_measurement(int qubit, const std::string& basis = "computational");
    
    // Custom gates
    void add_custom_gate(const std::string& name, std::shared_ptr<Gate> gate);
    void apply_custom_gate(const std::string& name, const std::vector<int>& qubits);
    
    // Circuit execution
    Statevector execute_statevector(const Statevector& initial_state = Statevector(0)) const;
    DensityMatrix execute_density_matrix(const DensityMatrix& initial_state = DensityMatrix(0)) const;
    
    // Circuit analysis
    int get_depth() const;
    int get_gate_count() const { return gates_.size(); }
    int get_measurement_count() const { return measurements_.size(); }
    int get_num_qubits() const { return num_qubits_; }
    int get_num_classical_bits() const { return num_classical_bits_; }
    
    // Circuit manipulation
    void reset();
    void remove_gate(size_t index);
    void remove_measurement(size_t index);
    void clear();
    
    // Circuit composition
    Circuit compose(const Circuit& other) const;
    Circuit tensor_product(const Circuit& other) const;
    
    // Circuit visualization
    std::string to_string() const;
    std::string to_qasm() const;
    
    // Circuit optimization
    void optimize();
    void decompose_to_basic_gates();
    
    // Circuit validation
    bool is_valid() const;
    std::vector<std::string> get_validation_errors() const;
};

// Concrete gate implementations
class HadamardGate : public Gate {
public:
    explicit HadamardGate(int qubit);
    void apply(Statevector& state) const override;
    void apply(DensityMatrix& state) const override;
    std::string to_string() const override;
};

class PauliXGate : public Gate {
public:
    explicit PauliXGate(int qubit);
    void apply(Statevector& state) const override;
    void apply(DensityMatrix& state) const override;
    std::string to_string() const override;
};

class PauliYGate : public Gate {
public:
    explicit PauliYGate(int qubit);
    void apply(Statevector& state) const override;
    void apply(DensityMatrix& state) const override;
    std::string to_string() const override;
};

class PauliZGate : public Gate {
public:
    explicit PauliZGate(int qubit);
    void apply(Statevector& state) const override;
    void apply(DensityMatrix& state) const override;
    std::string to_string() const override;
};

class CNOTGate : public Gate {
public:
    CNOTGate(int control, int target);
    void apply(Statevector& state) const override;
    void apply(DensityMatrix& state) const override;
    std::string to_string() const override;
};

class PhaseShiftGate : public Gate {
public:
    PhaseShiftGate(int qubit, double angle);
    void apply(Statevector& state) const override;
    void apply(DensityMatrix& state) const override;
    std::string to_string() const override;
};

class RotationXGate : public Gate {
public:
    RotationXGate(int qubit, double angle);
    void apply(Statevector& state) const override;
    void apply(DensityMatrix& state) const override;
    std::string to_string() const override;
};

class RotationYGate : public Gate {
public:
    RotationYGate(int qubit, double angle);
    void apply(Statevector& state) const override;
    void apply(DensityMatrix& state) const override;
    std::string to_string() const override;
};

class RotationZGate : public Gate {
public:
    RotationZGate(int qubit, double angle);
    void apply(Statevector& state) const override;
    void apply(DensityMatrix& state) const override;
    std::string to_string() const override;
};

} // namespace omniq

#endif // OMNIQ_CIRCUIT_H 