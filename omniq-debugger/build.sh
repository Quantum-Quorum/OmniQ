#!/bin/bash

# OmniQ Debugger Build Script

echo "Building OmniQ Debugger..."

# Check if Qt6 is installed
if ! pkg-config --exists Qt6Core Qt6Widgets; then
    echo "Error: Qt6 not found. Please install Qt6 development packages."
    echo "On Ubuntu/Debian: sudo apt-get install qt6-base-dev"
    echo "On macOS: brew install qt6"
    echo "On Windows: Download Qt6 from qt.io"
    exit 1
fi

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

echo "Build complete!"
echo "Run the debugger with: ./omniq-debugger" 