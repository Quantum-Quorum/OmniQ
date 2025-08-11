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