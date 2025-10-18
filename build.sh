#!/bin/bash

# LLAMA.CPP Manager - Quick Build Script
# Rebuilds the project quickly without full setup

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

echo "================================"
echo "LLAMA.CPP Manager - Quick Build"
echo "================================"
echo ""

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Build directory not found. Running full setup..."
    ./setup.sh
    exit 0
fi

# Check if Dear ImGui exists
if [ ! -d "extern/imgui" ]; then
    echo "Dear ImGui not found. Cloning..."
    mkdir -p extern
    git clone https://github.com/ocornut/imgui.git extern/imgui
fi

# Detect number of cores
if command -v nproc &> /dev/null; then
    CORES=$(nproc)
elif command -v sysctl &> /dev/null; then
    CORES=$(sysctl -n hw.ncpu)
else
    CORES=4
fi

echo "Building with $CORES cores..."
echo ""

cd build

# Reconfigure if CMakeLists.txt changed
if [ ../CMakeLists.txt -nt CMakeCache.txt ]; then
    echo "CMakeLists.txt changed, reconfiguring..."
    cmake ..
fi

# Build
make -j$CORES

BUILD_STATUS=$?

if [ $BUILD_STATUS -eq 0 ]; then
    echo ""
    echo "✓ Build successful!"
    echo ""
    echo "Run the application with:"
    echo "  ./build/llama_cpp_manager"
    echo ""
else
    echo ""
    echo "✗ Build failed!"
    exit 1
fi
