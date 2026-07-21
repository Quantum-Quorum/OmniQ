import sys
import os
import math
import pytest

# Ensure omniq-python is in path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../omniq-python')))

from omniq import MagnonicGate, DualRailCompiler, create_dual_rail_grover

def test_gate_translation():
    translated_bs = DualRailCompiler.translate_gate(MagnonicGate.BEAM_SPLITTER_5050, 0)
    assert translated_bs == ('H', 0)
    
    translated_tbs = DualRailCompiler.translate_gate(MagnonicGate.TEMPORAL_BEAM_SPLITTER_100, 1)
    assert translated_tbs == ('X', 1)
    
    translated_ck = DualRailCompiler.translate_gate(MagnonicGate.CROSS_KERR, 0, 1)
    assert translated_ck[0] == 'CP'
    assert translated_ck[1] == 0
    assert translated_ck[2] == 1
    assert math.isclose(translated_ck[3], math.pi)

def test_dual_rail_grover():
    circuit = create_dual_rail_grover(2, target_value=3)
    assert circuit.num_qubits == 2
    assert len(circuit.gates) > 0

if __name__ == '__main__':
    pytest.main([__file__])
