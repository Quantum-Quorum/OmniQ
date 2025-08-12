#include "omniq_c.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <complex>
#include <cmath>

// Forward declarations for the quantum backend
namespace omniq {
    class QuantumCircuit;
    class QuantumState;
}

// C wrapper structures
struct omniq_circuit {
    std::unique_ptr<omniq::QuantumCircuit> circuit;
    int num_qubits;
    
    omniq_circuit(int nq) : num_qubits(nq) {
        // Initialize with a simple circuit
        circuit = std::make_unique<omniq::QuantumCircuit>(nq);
    }
};

struct omniq_state {
    std::unique_ptr<omniq::QuantumState> state;
    int num_qubits;
    
    omniq_state(int nq) : num_qubits(nq) {
        state = std::make_unique<omniq::QuantumState>(nq);
    }
};

// Circuit creation and manipulation
omniq_circuit_t* omniq_circuit_create(int num_qubits) {
    if (num_qubits <= 0) {
        std::cerr << "Error: Number of qubits must be positive" << std::endl;
        return nullptr;
    }
    
    try {
        return new omniq_circuit(num_qubits);
    } catch (const std::exception& e) {
        std::cerr << "Error creating circuit: " << e.what() << std::endl;
        return nullptr;
    }
}

void omniq_circuit_destroy(omniq_circuit_t* circuit) {
    if (circuit) {
        delete circuit;
    }
}

// Gate operations
int omniq_circuit_add_hadamard(omniq_circuit_t* circuit, int qubit) {
    if (!circuit || qubit < 0 || qubit >= circuit->num_qubits) {
        return -1;
    }
    
    try {
        circuit->circuit->addGate(omniq::GateType::H, qubit);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error adding Hadamard gate: " << e.what() << std::endl;
        return -1;
    }
}

int omniq_circuit_add_pauli_x(omniq_circuit_t* circuit, int qubit) {
    if (!circuit || qubit < 0 || qubit >= circuit->num_qubits) {
        return -1;
    }
    
    try {
        circuit->circuit->addGate(omniq::GateType::X, qubit);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error adding Pauli-X gate: " << e.what() << std::endl;
        return -1;
    }
}

int omniq_circuit_add_pauli_y(omniq_circuit_t* circuit, int qubit) {
    if (!circuit || qubit < 0 || qubit >= circuit->num_qubits) {
        return -1;
    }
    
    try {
        circuit->circuit->addGate(omniq::GateType::Y, qubit);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error adding Pauli-Y gate: " << e.what() << std::endl;
        return -1;
    }
}

int omniq_circuit_add_pauli_z(omniq_circuit_t* circuit, int qubit) {
    if (!circuit || qubit < 0 || qubit >= circuit->num_qubits) {
        return -1;
    }
    
    try {
        circuit->circuit->addGate(omniq::GateType::Z, qubit);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error adding Pauli-Z gate: " << e.what() << std::endl;
        return -1;
    }
}

int omniq_circuit_add_cnot(omniq_circuit_t* circuit, int control, int target) {
    if (!circuit || control < 0 || control >= circuit->num_qubits ||
        target < 0 || target >= circuit->num_qubits || control == target) {
        return -1;
    }
    
    try {
        circuit->circuit->addGate(omniq::GateType::CNOT, control, target);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error adding CNOT gate: " << e.what() << std::endl;
        return -1;
    }
}

int omniq_circuit_add_swap(omniq_circuit_t* circuit, int qubit1, int qubit2) {
    if (!circuit || qubit1 < 0 || qubit1 >= circuit->num_qubits ||
        qubit2 < 0 || qubit2 >= circuit->num_qubits || qubit1 == qubit2) {
        return -1;
    }
    
    try {
        circuit->circuit->addGate(omniq::GateType::SWAP, qubit1, qubit2);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error adding SWAP gate: " << e.what() << std::endl;
        return -1;
    }
}

int omniq_circuit_add_rotation_x(omniq_circuit_t* circuit, int qubit, double angle) {
    if (!circuit || qubit < 0 || qubit >= circuit->num_qubits) {
        return -1;
    }
    
    try {
        circuit->circuit->addGate(omniq::GateType::RX, qubit, angle);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error adding RX gate: " << e.what() << std::endl;
        return -1;
    }
}

int omniq_circuit_add_rotation_y(omniq_circuit_t* circuit, int qubit, double angle) {
    if (!circuit || qubit < 0 || qubit >= circuit->num_qubits) {
        return -1;
    }
    
    try {
        circuit->circuit->addGate(omniq::GateType::RY, qubit, angle);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error adding RY gate: " << e.what() << std::endl;
        return -1;
    }
}

int omniq_circuit_add_rotation_z(omniq_circuit_t* circuit, int qubit, double angle) {
    if (!circuit || qubit < 0 || qubit >= circuit->num_qubits) {
        return -1;
    }
    
    try {
        circuit->circuit->addGate(omniq::GateType::RZ, qubit, angle);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error adding RZ gate: " << e.what() << std::endl;
        return -1;
    }
}

int omniq_circuit_add_phase(omniq_circuit_t* circuit, int qubit, double angle) {
    if (!circuit || qubit < 0 || qubit >= circuit->num_qubits) {
        return -1;
    }
    
    try {
        circuit->circuit->addGate(omniq::GateType::PHASE, qubit, angle);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error adding Phase gate: " << e.what() << std::endl;
        return -1;
    }
}

// Circuit execution
omniq_state_t* omniq_circuit_execute(omniq_circuit_t* circuit) {
    if (!circuit) {
        return nullptr;
    }
    
    try {
        auto state = new omniq_state(circuit->num_qubits);
        
        // Execute the circuit step by step
        while (circuit->circuit->executeStep(*state->state)) {
            // Continue execution
        }
        
        return state;
    } catch (const std::exception& e) {
        std::cerr << "Error executing circuit: " << e.what() << std::endl;
        return nullptr;
    }
}

int omniq_circuit_get_gate_count(omniq_circuit_t* circuit) {
    if (!circuit) {
        return -1;
    }
    
    try {
        return circuit->circuit->getTotalSteps();
    } catch (const std::exception& e) {
        std::cerr << "Error getting gate count: " << e.what() << std::endl;
        return -1;
    }
}

// State operations
void omniq_state_destroy(omniq_state_t* state) {
    if (state) {
        delete state;
    }
}

int omniq_state_measure(omniq_state_t* state, int qubit) {
    if (!state || qubit < 0 || qubit >= state->num_qubits) {
        return -1;
    }
    
    try {
        // Simple measurement in computational basis
        double prob_0 = state->state->getQubitProbability(qubit, 0);
        double prob_1 = state->state->getQubitProbability(qubit, 1);
        
        // Normalize probabilities
        double total = prob_0 + prob_1;
        if (total > 0) {
            prob_0 /= total;
            prob_1 /= total;
        }
        
        // Return measurement result based on probabilities
        double rand_val = static_cast<double>(rand()) / RAND_MAX;
        return (rand_val < prob_0) ? 0 : 1;
    } catch (const std::exception& e) {
        std::cerr << "Error measuring qubit: " << e.what() << std::endl;
        return -1;
    }
}

double omniq_state_get_amplitude_real(omniq_state_t* state, int index) {
    if (!state || index < 0 || index >= (1 << state->num_qubits)) {
        return 0.0;
    }
    
    try {
        // This would need to be implemented based on the actual state representation
        // For now, return a placeholder
        return 0.0;
    } catch (const std::exception& e) {
        std::cerr << "Error getting amplitude real part: " << e.what() << std::endl;
        return 0.0;
    }
}

double omniq_state_get_amplitude_imag(omniq_state_t* state, int index) {
    if (!state || index < 0 || index >= (1 << state->num_qubits)) {
        return 0.0;
    }
    
    try {
        // This would need to be implemented based on the actual state representation
        // For now, return a placeholder
        return 0.0;
    } catch (const std::exception& e) {
        std::cerr << "Error getting amplitude imaginary part: " << e.what() << std::endl;
        return 0.0;
    }
}

int omniq_state_get_num_qubits(omniq_state_t* state) {
    if (!state) {
        return -1;
    }
    return state->num_qubits;
}

// Debugger functions (like Python circuit.debug())
int omniq_debugger_show(omniq_circuit_t* circuit) {
    if (!circuit) {
        std::cerr << "Error: Cannot show debugger for null circuit" << std::endl;
        return -1;
    }
    
    try {
        // Find the debugger executable
        std::string debugger_path = "../omniq-debugger/build/omniq-debugger";
        
        // Check if debugger exists
        FILE* file = fopen(debugger_path.c_str(), "r");
        if (!file) {
            // Try alternative paths
            debugger_path = "./omniq-debugger/build/omniq-debugger";
            file = fopen(debugger_path.c_str(), "r");
            if (!file) {
                debugger_path = "omniq-debugger/build/omniq-debugger";
                file = fopen(debugger_path.c_str(), "r");
                if (!file) {
                    std::cerr << "Error: OmniQ debugger not found. Please build it first." << std::endl;
                    return -1;
                }
            }
        }
        fclose(file);
        
        // Launch the debugger
        std::string command = debugger_path + " &";
        int result = system(command.c_str());
        
        if (result == 0) {
            std::cout << "🚀 OmniQ Quantum Debugger opened!" << std::endl;
            std::cout << "   • Use the GUI to inspect quantum states" << std::endl;
            std::cout << "   • Drag and drop gates to build circuits" << std::endl;
            std::cout << "   • Step through quantum operations" << std::endl;
            return 0;
        } else {
            std::cerr << "Error: Failed to launch debugger" << std::endl;
            return -1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error opening debugger: " << e.what() << std::endl;
        return -1;
    }
}

int omniq_debugger_show_state(omniq_state_t* state) {
    if (!state) {
        std::cerr << "Error: Cannot show debugger for null state" << std::endl;
        return -1;
    }
    
    // For now, just open the debugger (state visualization would need more integration)
    return omniq_debugger_open();
}

int omniq_debugger_show_circuit_and_state(omniq_circuit_t* circuit, omniq_state_t* state) {
    if (!circuit) {
        std::cerr << "Error: Cannot show debugger for null circuit" << std::endl;
        return -1;
    }
    
    // For now, just open the debugger (circuit and state visualization would need more integration)
    return omniq_debugger_show(circuit);
}

// Convenience functions
int omniq_debugger_open() {
    try {
        // Find the debugger executable
        std::string debugger_path = "../omniq-debugger/build/omniq-debugger";
        
        // Check if debugger exists
        FILE* file = fopen(debugger_path.c_str(), "r");
        if (!file) {
            debugger_path = "./omniq-debugger/build/omniq-debugger";
            file = fopen(debugger_path.c_str(), "r");
            if (!file) {
                debugger_path = "omniq-debugger/build/omniq-debugger";
                file = fopen(debugger_path.c_str(), "r");
                if (!file) {
                    std::cerr << "Error: OmniQ debugger not found. Please build it first." << std::endl;
                    std::cerr << "💡 Try: cd omniq-debugger && ./build.sh" << std::endl;
                    return -1;
                }
            }
        }
        fclose(file);
        
        // Launch the debugger
        std::string command = debugger_path + " &";
        int result = system(command.c_str());
        
        if (result == 0) {
            std::cout << "🚀 OmniQ Quantum Debugger opened!" << std::endl;
            std::cout << "   • Use the GUI to inspect quantum states" << std::endl;
            std::cout << "   • Drag and drop gates to build circuits" << std::endl;
            std::cout << "   • Step through quantum operations" << std::endl;
            return 0;
        } else {
            std::cerr << "Error: Failed to launch debugger" << std::endl;
            return -1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error opening debugger: " << e.what() << std::endl;
        return -1;
    }
}

int omniq_debugger_close() {
    try {
        // Kill any running debugger processes
        std::string command = "pkill -f omniq-debugger";
        int result = system(command.c_str());
        
        if (result == 0) {
            std::cout << "✅ OmniQ Quantum Debugger closed" << std::endl;
            return 0;
        } else {
            std::cout << "ℹ️  No debugger processes found to close" << std::endl;
            return 0;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error closing debugger: " << e.what() << std::endl;
        return -1;
    }
}
