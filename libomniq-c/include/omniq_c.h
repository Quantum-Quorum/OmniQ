#ifndef OMNIQ_C_H
#define OMNIQ_C_H

#ifdef __cplusplus
extern "C" {
#endif

// Basic types
typedef struct omniq_circuit omniq_circuit_t;
typedef struct omniq_state omniq_state_t;

// Circuit creation and manipulation
omniq_circuit_t* omniq_circuit_create(int num_qubits);
void omniq_circuit_destroy(omniq_circuit_t* circuit);

// Gate operations
int omniq_circuit_add_hadamard(omniq_circuit_t* circuit, int qubit);
int omniq_circuit_add_pauli_x(omniq_circuit_t* circuit, int qubit);
int omniq_circuit_add_pauli_y(omniq_circuit_t* circuit, int qubit);
int omniq_circuit_add_pauli_z(omniq_circuit_t* circuit, int qubit);
int omniq_circuit_add_cnot(omniq_circuit_t* circuit, int control, int target);
int omniq_circuit_add_swap(omniq_circuit_t* circuit, int qubit1, int qubit2);
int omniq_circuit_add_rotation_x(omniq_circuit_t* circuit, int qubit, double angle);
int omniq_circuit_add_rotation_y(omniq_circuit_t* circuit, int qubit, double angle);
int omniq_circuit_add_rotation_z(omniq_circuit_t* circuit, int qubit, double angle);
int omniq_circuit_add_phase(omniq_circuit_t* circuit, int qubit, double angle);

// Circuit execution
omniq_state_t* omniq_circuit_execute(omniq_circuit_t* circuit);
int omniq_circuit_get_gate_count(omniq_circuit_t* circuit);

// State operations
void omniq_state_destroy(omniq_state_t* state);
int omniq_state_measure(omniq_state_t* state, int qubit);
double omniq_state_get_amplitude_real(omniq_state_t* state, int index);
double omniq_state_get_amplitude_imag(omniq_state_t* state, int index);
int omniq_state_get_num_qubits(omniq_state_t* state);

// Debugger functions (like Python circuit.debug())
int omniq_debugger_show(omniq_circuit_t* circuit);
int omniq_debugger_show_state(omniq_state_t* state);
int omniq_debugger_show_circuit_and_state(omniq_circuit_t* circuit, omniq_state_t* state);

// Convenience functions
int omniq_debugger_open();  // Open debugger without circuit
int omniq_debugger_close(); // Close debugger

#ifdef __cplusplus
}
#endif

#endif // OMNIQ_C_H
