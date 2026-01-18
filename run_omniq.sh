#!/bin/bash
# Quick launcher for OmniQ Debugger

echo "🚀 Launching OmniQ Quantum Debugger..."
echo ""
echo "Features available:"
echo "  ✅ 3D Bloch Sphere"
echo "  ✅ 3D Entanglement Graph"  
echo "  ✅ 3D Hilbert Space Viewer"
echo "  ✅ Accelerated Circuit View"
echo "  ✅ Quantum State Analysis"
echo ""

cd "$(dirname "$0")/omniq-debugger/build" || exit 1

if [ ! -f "./omniq-debugger" ]; then
    echo "❌ Error: omniq-debugger not found!"
    echo "Please build first:"
    echo "  cd omniq-debugger && mkdir -p build && cd build"
    echo "  cmake .. && make -j4"
    exit 1
fi

./omniq-debugger
