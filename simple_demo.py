#!/usr/bin/env python3
"""Simple OmniQ Demo - All Features"""

import sys
sys.path.insert(0, 'libomniq-core/build/lib')

try:
    import _omniq_core as omniq
    print("✅ OmniQ loaded successfully!\n")
except ImportError as e:
    print(f"❌ Error: {e}")
    print("\nRun this from the OmniQ root directory:")
    print("  cd /Users/gouthamarcot/Documents/Quantum/codebases/OmniQ")
    print("  python3 simple_demo.py")
    sys.exit(1)

print("="*60)
print(" OmniQ Feature Demo")
print("="*60)

# 1. Bell State
print("\n📊 1. Creating Bell State |00⟩ + |11⟩")
circuit = omniq.Circuit(2)
circuit.addGate(omniq.GateType.H, [], [0], [])
circuit.addGate(omniq.GateType.CNOT, [0], [1], [])
state = circuit.executeCircuit()

sv = state.getStateVector()
print(f"   |00⟩: {abs(sv[0]):.3f}")
print(f"   |11⟩: {abs(sv[3]):.3f}")

concurrence = omniq.calculateConcurrence(state)
print(f"   Concurrence: {concurrence:.3f} ✅")

# 2. Clifford Simulator
print("\n⚡ 2. Clifford Simulator - 50 Qubit GHZ State")
sim = omniq.CliffordSimulator(50)
sim.applyH(0)
for i in range(49):
    sim.applyCNOT(i, i+1)
print(f"   Created {sim.getNumQubits()}-qubit GHZ state ✅")

# 3. Surface Code
print("\n🔧 3. Quantum Error Correction - Surface Code")
surface = omniq.SurfaceCode(3)
print(f"   Distance: {surface.getDistance()}")
print(f"   Data qubits: {surface.getNumDataQubits()}")
print(f"   Total qubits: {surface.getTotalQubits()} ✅")

# 4. Decoders
print("\n🎯 4. QEC Decoders")
mwpm = omniq.MWPMDecoder()
uf = omniq.UnionFindDecoder()
print(f"   {mwpm.getName()}: {mwpm.getDescription()}")
print(f"   {uf.getName()}: {uf.getDescription()} ✅")

# 5. Noise
print("\n🌫️  5. Noise Channels")
depol = omniq.DepolarizingChannel(0.01)
print(f"   {depol.getDescription()} ✅")

T1 = omniq.AmplitudeDampingChannel.fromT1(50e-6, 50e-9)
print(f"   AmplitudeDamping(γ={T1.getGamma():.6f}) ✅")

noise = omniq.NoiseModel.createTypicalModel()
print(f"   NoiseModel configured ✅")

print("\n" + "="*60)
print(" ✨ All Features Working! ✨")
print("="*60)
print("\nNext steps:")
print("  • Run GUI: cd omniq-debugger/build && ./omniq-debugger")
print("  • See QUICKSTART.md for more examples")
