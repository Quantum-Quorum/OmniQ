# 🚀 OmniQ Quantum Debugger - Live Demo Guide

## 🎯 **What You're About to See**
A **State-of-the-Art (SOTA)** quantum computing debugger with real quantum calculations, not just simulations!

## 📋 **Prerequisites**
- macOS with Homebrew installed
- Qt6 and CMake (will be installed automatically)

## 🚀 **Quick Start (2 minutes)**

### 1. **Clone and Build**
```bash
git clone https://github.com/Quantum-Quorum/OmniQ.git
cd OmniQ/omniq-debugger
./build.sh
```

### 2. **Run the Debugger**
```bash
./build/omniq-debugger
```

## 🎮 **Interactive Demo Steps**

### **Step 1: Circuit Visualization**
- Look at the **Circuit View** panel (left side)
- You'll see a real quantum circuit with:
  - **Hadamard (H)** gate on qubit 0
  - **CNOT** gate between qubits 0→1
  - **Hadamard (H)** gate on qubit 1
- This creates a **Bell state** - a maximally entangled quantum state!

### **Step 2: Real Quantum Execution**
- Click **"Step Forward"** button (▶️)
- Watch the **Execution Progress** bar advance
- Each click applies a **real quantum gate** using actual quantum mechanics
- The **Current Step** counter shows which gate is being executed

### **Step 3: Quantum State Analysis**
After each step, observe the **Quantum State Viewer**:
- **State Vector**: Real complex amplitudes [a, b, c, d]
- **Probabilities**: Calculated from |a|², |b|², |c|², |d|²
- **Density Matrix**: Actual quantum state representation

### **Step 4: Qubit Debugging (Bloch Sphere)**
In the **Qubit Viewer** panel:
- **Real Bloch sphere coordinates** (θ, φ) from quantum calculations
- **Real measurement probabilities** for |0⟩ and |1⟩ states
- **Real quantum amplitudes** with complex numbers

## 🔬 **What You Should See (Real Quantum Physics)**

### **Initial State (Step 0)**
```
Qubit 0: |0⟩ state (θ=0°, φ=0°)
Qubit 1: |0⟩ state (θ=0°, φ=0°)
State Vector: [1, 0, 0, 0] (normalized)
```

### **After Step 1 (H gate on qubit 0)**
```
Qubit 0: Superposition (θ=90°, φ=0°)
Probabilities: ~50% |0⟩, ~50% |1⟩
State Vector: [0.707, 0.707, 0, 0]
```

### **After Step 2 (CNOT gate)**
```
Bell State: (|00⟩ + |11⟩)/√2
Entanglement: MAXIMAL
Both qubits now perfectly correlated!
```

### **After Step 3 (H gate on qubit 1)**
```
Complex entangled state
All four basis states have non-zero amplitudes
```

## 🎯 **Test These Real Quantum Features**

### ✅ **Real Quantum Calculations**
- **Gate Application**: Each button applies real quantum gates
- **State Evolution**: Watch state vector change with real math
- **Measurement**: Real probabilities, not fake data
- **Entanglement**: Observe quantum correlations develop

### ✅ **Advanced Quantum Analysis**
- **Bloch Sphere**: Real spherical coordinates from quantum states
- **Density Matrix**: Actual quantum state representation
- **State Purity**: Measures quantum state "purity"
- **Von Neumann Entropy**: Quantifies quantum information

### ✅ **Interactive Debugging**
- **Step-by-step execution**: Control quantum evolution
- **Real-time updates**: See changes instantly
- **State inspection**: Examine quantum states at any point
- **Measurement simulation**: Test quantum measurements

## 🧪 **Try This Interactive Test**

1. **Reset** the circuit (🔄 button)
2. **Step Forward** 3 times (▶️ button)
3. **Watch** the state vector change in real-time
4. **Observe** Bloch sphere parameters update
5. **See** probabilities change with each gate

## 🔬 **Scientific Verification**

### **Quantum Mechanics Accuracy**
- All calculations use **real quantum mechanics**
- **Eigen3** provides high-precision linear algebra
- Quantum gates are **properly implemented**
- State evolution follows **Schrödinger equation**
- Measurements follow **Born rule**

### **Bell State Creation**
```
|00⟩ → H(0) → (|0⟩ + |1⟩)/√2 ⊗ |0⟩
     → CNOT → (|00⟩ + |11⟩)/√2
     → H(1) → Complex entangled state
```

### **Quantum Measurement**
- **Computational basis (Z)**: |0⟩ vs |1⟩
- **X-basis**: (|0⟩ + |1⟩)/√2 vs (|0⟩ - |1⟩)/√2
- **Y-basis**: (|0⟩ + i|1⟩)/√2 vs (|0⟩ - i|1⟩)/√2

## 🏆 **What Makes This SOTA**

### **Real Quantum Backend**
- **Eigen3** for high-performance matrix operations
- **Complex number arithmetic** for quantum amplitudes
- **Proper quantum gate matrices** (H, X, Y, Z, CNOT, etc.)
- **Quantum state normalization** and evolution

### **Professional GUI**
- **Qt6** for modern, responsive interface
- **Real-time visualization** of quantum states
- **Interactive controls** for quantum execution
- **Professional debugging tools**

### **Research-Grade Features**
- **Quantum state tomography**
- **Entanglement measures**
- **Quantum coherence analysis**
- **State purity calculations**

## 🚀 **Ready for Production**

This debugger is **production-ready** for:
- **Quantum algorithm development**
- **Quantum computing research**
- **Educational quantum computing**
- **Professional quantum software development**

## 📞 **Support**

If you encounter any issues:
1. Check that Qt6 and CMake are installed
2. Ensure you're on macOS with Homebrew
3. Run `./build.sh` to rebuild if needed

---

**🎉 Congratulations! You're now using a State-of-the-Art quantum computing debugger with real quantum calculations!** 