#!/usr/bin/env python3
"""
OmniQ Demo - Showcasing all implemented features
"""

import sys
sys.path.insert(0, '/Users/gouthamarcot/Documents/Quantum/codebases/OmniQ/libomniq-core/build/lib')

import _omniq_core as omniq

print("=" * 60)
print("OmniQ Feature Demonstration")
print("=" * 60)

# 1. Basic Circuit Simulation
print("\n1. Creating a Bell State (|00⟩ + |11⟩)/√2")
print("-" * 60)

circuit = omniq.Circuit(2)
circuit.addGate(omniq.GateType.H, [], [0], [])  # Hadamard on qubit 0
circuit.addGate(omniq.GateType.CNOT, [0], [1], [])  # CNOT(0, 1)

state = circuit.execute()
print(f"State vector amplitudes:")
sv = state.getStateVector()
for i in range(4):
    amp = sv[i]
    print(f"  |{i:02b}⟩: {amp.real:.4f} + {amp.imag:.4f}i")

# 2. Clifford Simulator (1000+ qubits capable!)
print("\n\n2. Clifford Simulator - GHZ State with 10 qubits")
print("-" * 60)

from _omniq_core import CliffordSimulator

clifford_sim = omniq.CliffordSimulator(10)
print(f"Initialized {clifford_sim.getNumQubits()} qubit Clifford simulator")

# Create GHZ state: H on qubit 0, then CNOT chain
clifford_sim.applyH(0)
for i in range(9):
    clifford_sim.applyCNOT(i, i+1)

print("Applied: H(0) + CNOT(0,1) + CNOT(1,2) + ... + CNOT(8,9)")
print("Created 10-qubit GHZ state!")

# Measure first qubit
result = clifford_sim.measure(0)
print(f"Measured qubit 0: {result}")

# 3. Quantum Entanglement Metrics
print("\n\n3. Entanglement Analysis - Concurrence & Von Neumann Entropy")
print("-" * 60)

# Create maximally entangled state
bell_circuit = omniq.Circuit(2)
bell_circuit.addGate(omniq.GateType.H, [], [0], [])
bell_circuit.addGate(omniq.GateType.CNOT, [0], [1], [])
bell_state = bell_circuit.execute()

concurrence = omniq.calculateConcurrence(bell_state)
print(f"Concurrence: {concurrence:.4f} (1.0 = maximally entangled)")

# Density matrix for entropy
rho = omniq.DensityMatrix(bell_state)
entropy = omniq.calculateVonNeumannEntropy(rho)
print(f"Von Neumann Entropy: {entropy:.4f}")

# 4. Quantum Error Correction
print("\n\n4. QEC - Surface Code & Decoders")
print("-" * 60)

from _omniq_core import SurfaceCode, Syndrome, MWPMDecoder, UnionFindDecoder

# Create distance-3 surface code
surface_code = omniq.SurfaceCode(3)
print(f"Surface Code:")
print(f"  Distance: {surface_code.getDistance()}")
print(f"  Data qubits: {surface_code.getNumDataQubits()}")
print(f"  Ancilla qubits: {surface_code.getNumAncillaQubits()}")
print(f"  Total: {surface_code.getTotalQubits()}")

# Prepare logical zero state
logical_zero = surface_code.prepareLogicalZero()
print(f"Prepared logical |0⟩ state with {logical_zero.getNumQubits()} qubits")

# Create decoders
mwpm_decoder = omniq.MWPMDecoder()
uf_decoder = omniq.UnionFindDecoder()
print(f"Decoders available: {mwpm_decoder.getName()}, {uf_decoder.getName()}")

# 5. Noise Simulation
print("\n\n5. Noise Models - Realistic Decoherence")
print("-" * 60)

from _omniq_core import DepolarizingChannel, AmplitudeDampingChannel, PhaseDampingChannel

# Depolarizing channel
depol = omniq.DepolarizingChannel(0.01)  # 1% error probability
print(f"Depolarizing: {depol.getDescription()}")

# T1 relaxation (amplitude damping)
T1_channel = omniq.AmplitudeDampingChannel.fromT1(50e-6, 50e-9)  # T1=50μs, gate=50ns
print(f"Amplitude Damping: γ={T1_channel.getGamma():.6f}")

# T2 dephasing
T2_channel = omniq.PhaseDampingChannel.fromT2(70e-6, 50e-9)  # T2=70μs, gate=50ns  
print(f"Phase Damping: λ={T2_channel.getLambda():.6f}")

# NoiseModel
noise_model = omniq.NoiseModel.createTypicalModel()
print(f"\n{noise_model.toString()}")

# 6. Advanced Math - Spectral Analysis
print("\n\n6. Spectral Analysis - Eigenvalues & Negativity")
print("-" * 60)

from _omniq_core import SpectralAnalysis

# Create density matrix from Bell state
bell_rho = omniq.DensityMatrix(bell_state)
dm = bell_rho.getDensityMatrix()

# Eigendecomposition
eig_decomp = omniq.SpectralAnalysis.computeEigendecomposition(dm)
print(f"Eigenvalues of Bell state density matrix:")
for i in range(len(eig_decomp.eigenvalues)):
    print(f"  λ_{i} = {eig_decomp.eigenvalues[i]:.4f}")

# Negativity (entanglement measure)
negativity = omniq.SpectralAnalysis.calculateNegativity(dm, 2, 2)
print(f"Negativity: {negativity:.4f} (>0 indicates entanglement)")

print("\n" + "=" * 60)
print("Demo Complete! All features working ✅")
print("=" * 60)
