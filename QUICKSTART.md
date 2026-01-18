# OmniQ Quick Start Guide

## Running the Qt Debugger (GUI)

```bash
cd /Users/gouthamarcot/Documents/Quantum/codebases/OmniQ/omniq-debugger/build
./omniq-debugger
```

This launches the full Qt GUI with:
- 3D Bloch Sphere visualization
- Entanglement Graph (3D network)
- Hilbert Space Viewer (3D cityscape)
- Accelerated Circuit View (OpenGL rendering)
- Quantum State Viewer with all metrics
- Live state inspection

---

## Using Python Bindings

### Option 1: Direct Python import

```bash
cd /Users/gouthamarcot/Documents/Quantum/codebases/OmniQ
export PYTHONPATH=/Users/gouthamarcot/Documents/Quantum/codebases/OmniQ/libomniq-core/build/lib:$PYTHONPATH
python3
```

```python
import _omniq_core as omniq

# Create Bell state
circuit = omniq.Circuit(2)
circuit.addGate(omniq.GateType.H, [], [0], [])
circuit.addGate(omniq.GateType.CNOT, [0], [1], [])
state = circuit.execute()

print("Bell state created!")
sv = state.getStateVector()
for i in range(4):
    print(f"|{i:02b}⟩: {sv[i]}")
```

### Option 2: IPython/Jupyter

```bash
export PYTHONPATH=/Users/gouthamarcot/Documents/Quantum/codebases/OmniQ/libomniq-core/build/lib:$PYTHONPATH
ipython
```

```python
import _omniq_core as omniq

# Clifford Simulator - 100 qubits instantly!
sim = omniq.CliffordSimulator(100)
sim.applyH(0)
for i in range(99):
    sim.applyCNOT(i, i+1)
print(f"Created 100-qubit GHZ state!")
result = sim.measure(0)
print(f"Measurement: {result}")
```

---

## Quick Examples

### 1. Bell State with Entanglement Metrics

```python
import _omniq_core as omniq

# Create circuit
circuit = omniq.Circuit(2)
circuit.addGate(omniq.GateType.H, [], [0], [])
circuit.addGate(omniq.GateType.CNOT, [0], [1], [])
state = circuit.execute()

# Calculate concurrence
concurrence = omniq.calculateConcurrence(state)
print(f"Concurrence: {concurrence:.4f}")  # Should be 1.0
```

### 2. Clifford Simulator (1000+ qubits)

```python
import _omniq_core as omniq

# Simulate 500 qubits
sim = omniq.CliffordSimulator(500)

# Create cluster state
for i in range(500):
    sim.applyH(i)
for i in range(499):
    sim.applyCZ(i, i+1)

print("500-qubit cluster state created!")
```

### 3. QEC Surface Code

```python
import _omniq_core as omniq

# Distance-5 surface code
surface_code = omniq.SurfaceCode(5)
print(f"Data qubits: {surface_code.getNumDataQubits()}")  # 25
print(f"Ancilla: {surface_code.getNumAncillaQubits()}")   # 24

# Prepare logical state
logical_zero = surface_code.prepareLogicalZero()

# Use decoders
mwpm = omniq.MWPMDecoder()
uf = omniq.UnionFindDecoder()
print(f"Decoders: {mwpm.getName()}, {uf.getName()}")
```

### 4. Noise Simulation

```python
import _omniq_core as omniq

# Create noisy single-qubit gate
rho = omniq.DensityMatrix(1)  # |0⟩

# Apply T1 relaxation
T1_noise = omniq.AmplitudeDampingChannel.fromT1(50e-6, 50e-9)
T1_noise.apply(rho, 0)

# Apply T2 dephasing  
T2_noise = omniq.PhaseDampingChannel.fromT2(70e-6, 50e-9)
T2_noise.apply(rho, 0)

# Check purity
purity = rho.getPurity()
print(f"Purity after noise: {purity:.4f}")
```

### 5. Spectral Analysis

```python
import _omniq_core as omniq

# Create maximally entangled state
circuit = omniq.Circuit(2)
circuit.addGate(omniq.GateType.H, [], [0], [])
circuit.addGate(omniq.GateType.CNOT, [0], [1], [])
state = circuit.execute()

# Get density matrix
rho = omniq.DensityMatrix(state)
dm = rho.getDensityMatrix()

# Eigenvalues
eig = omniq.SpectralAnalysis.computeEigendecomposition(dm)
print("Eigenvalues:", eig.eigenvalues)

# Negativity (entanglement)
neg = omniq.SpectralAnalysis.calculateNegativity(dm, 2, 2)
print(f"Negativity: {neg:.4f}")
```

---

## Available Features

✅ **Core Simulation**
- State vector simulation (up to ~20 qubits)
- Density matrix simulation
- All standard gates (H, X, Y, Z, CNOT, RX, RY, RZ, etc.)

✅ **Clifford Simulator**
- 1000+ qubit simulation
- All Clifford gates
- Measurement

✅ **QEC**
- Surface codes (distance 3, 5, 7, ...)
- MWPM decoder (greedy)
- Union-Find decoder (fast)
- Syndrome extraction

✅ **Noise Models**
- Depolarizing channel
- Amplitude damping (T1)
- Phase damping (T2)
- NoiseModel presets

✅ **Analysis**
- Concurrence
- Von Neumann entropy
- Fidelity
- Eigenvalue decomposition
- Partial transpose
- Negativity

✅ **Debugger (GUI)**
- Step-by-step execution
- Breakpoints (conditional)
- State history
- 3D visualizations

---

## Build from Source (if needed)

```bash
# Core library
cd /Users/gouthamarcot/Documents/Quantum/codebases/OmniQ/libomniq-core
mkdir -p build && cd build
cmake .. && make -j4

# Debugger (Qt GUI)
cd /Users/gouthamarcot/Documents/Quantum/codebases/OmniQ/omniq-debugger
mkdir -p build && cd build
cmake .. && make -j4
```

---

## Troubleshooting

**Python module not found:**
```bash
export PYTHONPATH=/Users/gouthamarcot/Documents/Quantum/codebases/OmniQ/libomniq-core/build/lib:$PYTHONPATH
```

**Qt not opening:**
```bash
# Make sure Qt6 is installed
brew install qt@6
```

**Segmentation fault:**
- Most likely: Using more than ~20 qubits with regular state vector simulator
- Solution: Use Clifford simulator for large qubit counts
