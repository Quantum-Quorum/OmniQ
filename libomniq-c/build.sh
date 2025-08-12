#!/bin/bash

echo "Building OmniQ C Library..."

# Check if CMake is installed
if ! command -v cmake &> /dev/null; then
    echo "Error: CMake not found. Please install CMake first."
    echo "On macOS: brew install cmake"
    echo "On Ubuntu: sudo apt-get install cmake"
    exit 1
fi

# Check if Eigen3 is installed
if ! pkg-config --exists eigen3; then
    echo "Error: Eigen3 not found. Please install Eigen3 first."
    echo "On macOS: brew install eigen"
    echo "On Ubuntu: sudo apt-get install libeigen3-dev"
    exit 1
fi

# Create build directory
mkdir -p build
cd build

# Configure and build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

if [ $? -eq 0 ]; then
    echo "✅ OmniQ C Library built successfully!"
    echo "📁 Library location: build/libomniq-c.dylib (macOS) or build/libomniq-c.so (Linux)"
    echo ""
    echo "🚀 To test the debugger functionality:"
    echo "   cd examples"
    echo "   gcc -o debugger_demo debugger_demo.c -I../include -L../build -lomniq-c"
    echo "   ./debugger_demo"
    echo ""
    echo "   Or for C++:"
    echo "   g++ -o debugger_demo_cpp debugger_demo.cpp -I../include -L../build -lomniq-c"
    echo "   ./debugger_demo_cpp"
else
    echo "❌ Build failed!"
    exit 1
fi 