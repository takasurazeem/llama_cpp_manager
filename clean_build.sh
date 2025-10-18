#!/bin/bash

# LLAMA.CPP Manager - Clean Build Script
# Performs a clean rebuild from scratch

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

echo "================================"
echo "LLAMA.CPP Manager - Clean Build"
echo "================================"
echo ""

read -p "This will remove the build directory. Continue? (y/n) " -n 1 -r
echo ""

if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Cancelled."
    exit 0
fi

# Remove build directory
if [ -d "build" ]; then
    echo "Removing build directory..."
    rm -rf build
fi

# Check for Dear ImGui
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

echo ""
echo "Creating fresh build directory..."
mkdir -p build
cd build

echo "Running CMake..."
cmake ..

echo ""
echo "Building with $CORES cores..."
make -j$CORES

BUILD_STATUS=$?

if [ $BUILD_STATUS -eq 0 ]; then
    echo ""
    echo "✓ Clean build successful!"
    echo ""
    echo "Run the application with:"
    echo "  ./build/llama_cpp_manager"
    echo ""
else
    echo ""
    echo "✗ Build failed!"
    exit 1
fi
