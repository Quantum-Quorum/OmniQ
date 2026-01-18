## ✅ **How to Run OmniQ**

### **EASIEST: Qt Debugger GUI**

All features are implemented and working in the GUI!

```bash
cd /Users/gouthamarcot/Documents/Quantum/codebases/OmniQ/omniq-debugger/build
./omniq-debugger
```

**What you get:**
- ✅ 3D Bloch Sphere visualization
- ✅ 3D Entanglement Graph (network visualization)
- ✅ 3D Hilbert Space Viewer (cityscape)
- ✅ Accelerated Circuit View (OpenGL, smooth zoom/pan)
- ✅ Quantum State Viewer
- ✅ All quantum metrics (concurrence, entropy, fidelity)

---

### **Python Bindings (Optional)**

Python bindings exist for basic features. New features (Clifford, QEC, etc.) need binding code added.

**Current Python support:**
- Circuit creation
- State vector simulation
- Basic gates
- Measurement
- Entanglement metrics (concurrence, entropy)

**To use existing bindings:**
```bash
cd /Users/gouthamarcot/Documents/Quantum/codebases/OmniQ/libomniq-core/build/lib
python3
```

```python
import _omniq_core
# Use basic quantum features
```

**To add new bindings** (for Clifford, QEC, etc):
- Need to update `libomniq-core/src/bindings.cpp`
- Add ~100 lines of pybind11 code
- Rebuild

---

### **Recommendation**

**🎯 Start with the Qt GUI** - it's fully functional and has all ~3,000 lines of new code working!

The GUI is production-ready and includes all implemented features from this session.
