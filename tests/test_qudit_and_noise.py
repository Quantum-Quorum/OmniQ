import sys
import os
import math
import pytest

# Ensure build and omniq-python are in path
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../omniq-python')))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../build/lib')))
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '../libomniq-core/build/lib')))

def test_qudit_statevector():
    try:
        import _omniq_core
    except ImportError:
        pytest.skip("_omniq_core C++ extension not compiled yet")

    qudit_sv = _omniq_core.QuditStatevector(2, 3) # 2 qutrits (d=3)
    assert qudit_sv.get_num_qudits() == 2
    assert qudit_sv.get_dimension() == 3
    assert qudit_sv.get_total_dimension() == 9
    assert qudit_sv.get_qudit_state_probability(0, 0) == 1.0

    # Apply Aharonov-Casher RZ rotation
    qudit_sv.apply_aharonov_casher_rz(0, math.pi / 2)
    
    # Apply CSUM gate
    qudit_sv.apply_csum(0, 1)

    # Apply Controlled Phase gate
    qudit_sv.apply_controlled_phase(0, 1, math.pi / 4)

    assert qudit_sv.get_norm() > 0.999

def test_quadit_dimension_4():
    try:
        from omniq import QuditStatevector
    except ImportError:
        pytest.skip("omniq C++ extension not compiled yet")

    if QuditStatevector is None:
        pytest.skip("QuditStatevector is None")

    quadit_sv = QuditStatevector(num_qudits=2, dimension=4) # 2 Quadits (d=4)
    assert quadit_sv.get_num_qudits() == 2
    assert quadit_sv.get_dimension() == 4
    assert quadit_sv.get_total_dimension() == 16
    assert math.isclose(quadit_sv.get_qudit_state_probability(0, 0), 1.0)

    # Apply Aharonov-Casher RZ rotation on quadit 0
    quadit_sv.apply_aharonov_casher_rz(0, math.pi / 4)

    # Apply CSUM Kerr interaction gate
    quadit_sv.apply_csum(0, 1)

    # Apply high-dimensional Controlled-Phase
    quadit_sv.apply_controlled_phase(0, 1, math.pi / 8)

    assert math.isclose(quadit_sv.get_norm(), 1.0, abs_tol=1e-6)

if __name__ == '__main__':
    pytest.main([__file__])
