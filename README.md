<img width="640" height="229" alt="image" src="https://github.com/user-attachments/assets/bff330e7-a8fa-4703-bb5a-ce065ebf2464" />

**OmniQ: Seamless Quantum Programming Across Python and C.**

OmniQ is a modern, open-source software library designed for quantum computing research and development. Its primary goal is to provide a unified and intuitive interface for constructing, simulating, and executing quantum algorithms, accessible from Python and high-performance C applications via a robust C++ core engine.

**Why OmniQ?**

* **Cross-Language Interoperability:** Write quantum code in Python using a consistent API, powered by a high-performance C++ backend. Integrate quantum components into larger applications written in Python or C/C++.
* **Ease of Use:** Focus on an intuitive programming model for defining quantum circuits, gates, and measurements.
* **Flexibility:** Support for various quantum computational paradigms, including circuit-based computation, variational algorithms (QML), and potentially others.
* **Extensibility:** Designed to be modular, allowing easy addition of new gates, devices, and application modules.
* **Performance:** Leverage a C++-based core for efficient simulation and potentially optimized execution on quantum hardware.

## Installation

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.16 or higher
- Python 3.8 or higher
- NumPy 1.21.0 or higher

### Building from Source

```bash
# Clone the repository
git clone https://github.com/Quantum-Quorum/OmniQ.git
cd OmniQ

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j$(nproc)

# Run tests
ctest --output-on-failure

# Install
sudo make install
```

### Python Package Installation

```bash
# Install from source
cd omniq-python
pip install -e .

# Or install dependencies for development
pip install -e ".[dev,test,docs]"
```

## Quick Start

### Basic Quantum Circuit

```python
import omniq as oq

# Create a 2-qubit circuit
circuit = oq.Circuit(2)

# Add quantum gates
circuit.add_gate('H', [0])      # Hadamard on qubit 0
circuit.add_gate('CNOT', [0, 1]) # CNOT with control=0, target=1

# Open debugger GUI (like df.head() in pandas)
circuit.debug()                 # Opens quantum debugger
# OR
oq.show_debugger(circuit)       # Standalone function

# C/C++ equivalent
omniq_debugger_show(circuit);   // Opens quantum debugger
omniq_debugger_open();          // Standalone function

# Execute the circuit
result = circuit.execute()
print(f"Final state: {result}")

# Measure the qubits
measurement1 = result.measure(0)
measurement2 = result.measure(1)
print(f"Measurements: {measurement1}, {measurement2}")
```

### Grover's Algorithm Example

```python
import omniq as oq

# Create a database search oracle
target_value = 5
oracle = oq.create_database_oracle(target_value)

# Create and run Grover's algorithm
grover = oq.GroversAlgorithm(4, oracle, 1)
results = grover.execute(num_shots=1000)

# Analyze results
from collections import Counter
result_counts = Counter(results)
print(f"Target value {target_value} found {result_counts[target_value]} times")
```

## Key Features

* **Unified API:** Consistent interface across Python, C, and C++ with seamless interoperability
* **High Performance:** C++ core engine optimized for quantum simulations
* **Interactive Debugger:** GUI debugger with drag-and-drop circuit building (like pandas DataFrame methods)
* **Multi-Language Debugger:** Python, C, and C++ debugger interfaces
* **Comprehensive Gate Set:** Support for all standard quantum gates (Hadamard, Pauli, CNOT, rotations, etc.)
* **Quantum Algorithms:** Built-in implementations of Grover's, QPE, and other quantum algorithms
* **Circuit Optimization:** Automatic circuit optimization and QASM export
* **Multiple Backends:** Support for various simulators and hardware platforms
* **Extensible Architecture:** Modular design for easy addition of new features

### Supported Quantum Gates

- **Single-qubit gates:** H, X, Y, Z, RX, RY, RZ, Phase
- **Two-qubit gates:** CNOT, SWAP, CZ, CRX, CRY, CRZ
- **Multi-qubit gates:** Toffoli, Fredkin, custom controlled gates
- **Measurement:** Computational basis, custom observables

## Core Concepts

OmniQ follows a device-agnostic approach similar to PennyLane and Qiskit:

* **Device:** Represents the backend (simulator or QPU) where computations run
* **Circuit:** Quantum circuit representation with gates and measurements
* **Statevector:** Pure quantum state representation
* **DensityMatrix:** Mixed quantum state representation
* **Operations:** Quantum gates and measurement operations
* **Wires:** System for addressing qubits (integer indices)
* **Parameters:** Gate parameters (rotation angles, etc.) for variational circuits

## Functionality Modules

### Core Quantum Computing (omniq.core)
* **Circuit Construction:** Build quantum circuits with gates and measurements
* **State Simulation:** Statevector and density matrix simulations
* **Gate Operations:** Complete set of quantum gates and operations
* **Circuit Optimization:** Automatic optimization and decomposition

### Quantum Algorithms (omniq.algorithms)
* **Grover's Algorithm:** Database search and SAT solving
* **Quantum Phase Estimation:** Eigenvalue estimation with QFT
* **Quantum Fourier Transform:** QFT and inverse QFT implementations
* **Amplitude Estimation:** Quantum amplitude estimation algorithms

### Cryptography (omniq.crypto)
* **Shor's Algorithm:** Integer factoring with quantum period finding
* **Grover's for Crypto:** Quantum attacks on symmetric cryptography
* **Quantum Key Distribution:** BB84 and other QKD protocols
* **Post-Quantum Crypto:** Lattice-based and other post-quantum schemes

### Quantum Machine Learning (omniq.qml)
* **Variational Circuits:** Parameterized quantum circuits for ML
* **Quantum Kernels:** Quantum kernel methods for classification
* **Quantum Neural Networks:** Quantum analogues of classical NNs

### Interactive Debugger (omniq.debugger)
* **GUI Debugger:** Qt6-based graphical debugger with real-time visualization
* **Drag-and-Drop Builder:** Visual circuit construction with gate palette
* **State Visualization:** Bloch sphere and density matrix visualization
* **Step-by-Step Execution:** Interactive circuit execution and debugging
* **Circuit Import/Export:** Save and load circuits in JSON format
* **Multi-Language Support:** Python, C, and C++ debugger interfaces
* **Hybrid Classical-Quantum:** Integration with classical ML frameworks

### Optimization (omniq.optimization)
* **Variational Quantum Eigensolver (VQE):** Ground state energy estimation
* **Quantum Approximate Optimization Algorithm (QAOA):** Combinatorial optimization
* **Quantum Natural Gradient:** Advanced optimization techniques
* **Parameter Shift Rules:** Automatic differentiation for quantum circuits

## Supported Backends

### Simulators
* **default.qubit:** High-performance statevector simulator (C++)
* **lightning.qubit:** PennyLane Lightning integration
* **density_matrix:** Mixed state simulator
* **noisy_simulator:** Simulator with noise models

### Hardware Backends
* **IBM Quantum:** IBM Quantum Experience integration
* **IonQ:** IonQ cloud quantum computers
* **Rigetti:** Rigetti quantum computers
* **Xanadu Cloud:** Photonic quantum computers

## Performance Benchmarks

| Backend | Qubits | Time (s) | Memory (GB) |
|---------|--------|----------|-------------|
| default.qubit | 20 | 0.5 | 0.5 |
| lightning.qubit | 20 | 0.3 | 0.5 |
| density_matrix | 10 | 2.1 | 4.0 |

## Development & Contribution

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for details on:

* Setting up a development environment
* Running tests and benchmarks
* Coding standards and style guidelines
* Submitting pull requests
* Reporting issues on the [GitHub Issue Tracker](https://github.com/Quantum-Quorum/OmniQ/issues)

### Development Setup

```bash
# Clone with submodules
git clone --recursive https://github.com/Quantum-Quorum/OmniQ.git
cd OmniQ

# Install development dependencies
pip install -e ".[dev,test,docs]"

# Run tests
pytest tests/
ctest --output-on-failure

# Run linting
black omniq-python/
flake8 omniq-python/
mypy omniq-python/
```

## Roadmap

### Version 0.2.0 (Q2 2024)
* [ ] Hardware backend integrations (IBM, IonQ, Rigetti)
* [ ] Advanced noise models and error mitigation
* [ ] Quantum chemistry module (VQE, UCCSD)
* [ ] Performance optimizations and GPU acceleration

### Version 0.3.0 (Q3 2024)
* [ ] Quantum machine learning framework
* [ ] Hybrid classical-quantum optimization
* [ ] Quantum error correction codes
* [ ] Advanced quantum algorithms library

### Version 1.0.0 (Q4 2024)
* [ ] Production-ready API stability
* [ ] Comprehensive documentation and tutorials
* [ ] Performance benchmarking suite
* [ ] Community-driven feature development

# OmniQ Quantum Circuit Debugger

A graphical debugger for quantum circuits built with Qt6 and the OmniQ quantum computing framework.

## Features

- **Circuit Visualization**: Visual representation of quantum circuits with gates and qubits
- **Step-by-Step Execution**: Debug quantum circuits by stepping through each gate
- **State Inspection**: View quantum state vectors and density matrices in real-time
- **Qubit Analysis**: Individual qubit probability and expectation value calculations
- **Circuit Optimization**: Suggestions for circuit optimization and gate reduction
- **File I/O**: Load and save circuits in QASM and JSON formats

## Requirements

- Qt6 (Core, Widgets)
- CMake 3.16+
- C++17 compiler
- OmniQ Core Library

## Installation

### Prerequisites

1. **Install Qt6**:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install qt6-base-dev
   
   # macOS
   brew install qt6
   
   # Windows
   # Download Qt6 from qt.io
   ```

2. **Build OmniQ Core Library**:
   ```bash
   cd ../libomniq-core
   mkdir build && cd build
   cmake ..
   make
   sudo make install
   ```

### Building the Debugger

```bash
# Clone the repository
git clone <repository-url>
cd omniq-debugger

# Build using the provided script
./build.sh

# Or build manually
mkdir build && cd build
cmake ..
make
```

## Usage

### Running the Debugger

```bash
./omniq-debugger
```

### Basic Operations

1. **Create New Circuit**: File → New or Ctrl+N
2. **Load Circuit**: File → Open or Ctrl+O
3. **Save Circuit**: File → Save or Ctrl+S
4. **Step Forward**: Debug → Step Forward or F10
5. **Step Backward**: Debug → Step Backward or Shift+F10
6. **Run Circuit**: Debug → Run or F5
7. **Pause Execution**: Debug → Pause or F6
8. **Reset Circuit**: Debug → Reset or Ctrl+R

### Interface Layout

- **Left Panel**: Circuit visualization with gates and qubit lines
- **Right Panel**: Quantum state viewer showing current state vector
- **Bottom Panel**: Qubit details table with probabilities
- **Toolbar**: Debug controls (step, run, pause, reset)
- **Status Bar**: Current execution status and progress

## Architecture

### Core Components

1. **MainWindow**: Main application window with menus, toolbars, and dock widgets
2. **CoreInterface**: Bridge between GUI and OmniQ core library
3. **CircuitView**: Widget for visualizing quantum circuits
4. **QuantumStateViewer**: Widget for displaying quantum states
5. **QubitViewer**: Widget for individual qubit analysis

### Key Classes

- `MainWindow`: Main application interface
- `CoreInterface`: OmniQ library integration
- `CircuitView`: Circuit visualization
- `QuantumStateViewer`: State display
- `QubitViewer`: Qubit analysis

## Development

### Project Structure

```
omniq-debugger/
├── src/
│   ├── main.cpp              # Application entry point
│   ├── MainWindow.h/cpp      # Main window implementation
│   ├── CoreInterface.h/cpp   # OmniQ integration
│   └── widgets/
│       ├── CircuitView.h/cpp      # Circuit visualization
│       ├── QuantumStateViewer.h/cpp # State display
│       └── QubitViewer.h/cpp      # Qubit analysis
├── CMakeLists.txt            # Build configuration
├── build.sh                  # Build script
└── README.md                 # This file
```

### Adding New Features

1. **New Widgets**: Add to `src/widgets/` directory
2. **Core Integration**: Extend `CoreInterface` class
3. **UI Updates**: Modify `MainWindow` class
4. **Build System**: Update `CMakeLists.txt`

### Debugging

- Use Qt Creator for debugging Qt applications
- Enable debug output with `QT_LOGGING_RULES="*.debug=true"`
- Check console output for error messages

## Screenshots
![alt text](https://github.com/Quantum-Quorum/OmniQ/blob/main/screenshots/screenshot1.png)


![alt text](https://github.com/Quantum-Quorum/OmniQ/blob/main/screenshots/screenshot2.png)
## Troubleshooting

### Common Issues

1. **Qt6 not found**: Install Qt6 development packages
2. **OmniQ library not found**: Build and install OmniQ core library
3. **Compilation errors**: Ensure C++17 support and proper include paths
4. **Runtime errors**: Check console output for error messages

### Build Issues

```bash
# Clean build
rm -rf build
mkdir build && cd build
cmake ..
make clean
make
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## License

This project is licensed under the Apache License 2.0 - see the LICENSE file for details.

## Acknowledgments

- Qt6 for the GUI framework
- OmniQ team for the quantum computing library
- Quantum computing community for inspiration and feedback 

## Screenshots



## Citing OmniQ

If you use OmniQ in your research, please cite:

```bibtex
@software{omniq2024,
  title={OmniQ: Seamless Quantum Programming Across Python and C},
  author={Arcot, Goutham},
  year={2024},
  url={https://github.com/Quantum-Quorum/OmniQ}
}
```

## License

OmniQ is released under the Apache License 2.0. See the [LICENSE](LICENSE) file for details.

## Acknowledgments

* Inspired by PennyLane, Qiskit, and other quantum computing frameworks
* Built with modern C++ and Python best practices
* Community contributions and feedback are invaluable

## Support

* **Documentation:** yet to make
* **Issues:** [GitHub Issues](https://github.com/Quantum-Quorum/OmniQ/issues)
* **Discussions:** [GitHub Discussions](https://github.com/Quantum-Quorum/OmniQ/discussions)
* **Email:** gouthamarcod@gmail.com

