//
// Created by Goutham Arcot on 17/07/25.
//

#include "libomniq-core/include/omniq/Circuit.h"
#include "libomniq-core/include/omniq/QuantumStates.h"
#include "libomniq-core/include/omniq/Statevector.h"
#include <iostream>

using namespace omniq;

int main() {
  std::cout << "OmniQ Quantum Computing Framework" << std::endl;
  std::cout << "=================================" << std::endl;

  try {
    // Create a simple Bell state circuit
    Circuit circuit(2);
    circuit.addGate(GateType::H, 0);
    circuit.addGate(GateType::CNOT, 0, 1);

    // Execute the circuit
    circuit.executeAll();
    Statevector result(circuit.getStateVector());

    std::cout << "Bell state created successfully!" << std::endl;
    std::cout << "Number of gates: " << circuit.getTotalSteps() << std::endl;

    // Print the final state
    std::cout << "\nFinal quantum state:" << std::endl;
    std::cout << result.toString() << std::endl;

    std::cout << "\nOmniQ is working correctly!" << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}