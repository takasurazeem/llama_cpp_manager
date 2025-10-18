#!/bin/bash

# LLAMA.CPP Manager - Setup Script
# This script helps set up the development environment

set -e

echo "================================"
echo "LLAMA.CPP Manager Setup"
echo "================================"
echo ""

# Detect OS
OS="unknown"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
else
    echo "Unsupported OS: $OSTYPE"
    exit 1
fi

echo "Detected OS: $OS"
echo ""

# Check for required tools
echo "Checking dependencies..."

check_command() {
    if command -v $1 &> /dev/null; then
        echo "✓ $1 found"
        return 0
    else
        echo "✗ $1 not found"
        return 1
    fi
}

MISSING_DEPS=0

check_command cmake || MISSING_DEPS=$((MISSING_DEPS + 1))
check_command make || MISSING_DEPS=$((MISSING_DEPS + 1))
check_command git || MISSING_DEPS=$((MISSING_DEPS + 1))

if [ $OS == "linux" ]; then
    check_command gcc || check_command clang || MISSING_DEPS=$((MISSING_DEPS + 1))
elif [ $OS == "macos" ]; then
    check_command clang || MISSING_DEPS=$((MISSING_DEPS + 1))
fi

echo ""

if [ $MISSING_DEPS -gt 0 ]; then
    echo "Missing dependencies detected."
    echo ""
    
    if [ $OS == "linux" ]; then
        echo "Install dependencies with:"
        echo "  sudo apt-get install build-essential cmake git libsdl2-dev"
        echo ""
        read -p "Would you like to install them now? (y/n) " -n 1 -r
        echo ""
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            sudo apt-get update
            sudo apt-get install -y build-essential cmake git libsdl2-dev
        fi
    elif [ $OS == "macos" ]; then
        echo "Install dependencies with:"
        echo "  brew install cmake git sdl2"
        echo ""
        read -p "Would you like to install them now? (requires Homebrew) (y/n) " -n 1 -r
        echo ""
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            brew install cmake git sdl2
        fi
    fi
fi

# Check for Dear ImGui
echo ""
echo "Checking for Dear ImGui..."

if [ ! -d "extern/imgui" ]; then
    echo "Dear ImGui not found. Cloning..."
    mkdir -p extern
    git clone https://github.com/ocornut/imgui.git extern/imgui
    echo "✓ Dear ImGui installed"
else
    echo "✓ Dear ImGui found"
    
    # Update if needed
    read -p "Update Dear ImGui? (y/n) " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        cd extern/imgui
        git pull
        cd ../..
        echo "✓ Dear ImGui updated"
    fi
fi

# Check for llama.cpp
echo ""
echo "Checking for llama.cpp..."

if command -v llama-cli &> /dev/null; then
    LLAMA_PATH=$(which llama-cli)
    echo "✓ llama.cpp found at: $LLAMA_PATH"
elif command -v main &> /dev/null; then
    LLAMA_PATH=$(which main)
    echo "✓ llama.cpp found at: $LLAMA_PATH"
else
    echo "✗ llama.cpp not found"
    echo ""
    echo "llama.cpp is required to run this application."
    echo "Visit: https://github.com/ggerganov/llama.cpp"
    echo ""
    read -p "Would you like to clone and build llama.cpp now? (y/n) " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        read -p "Install to (default: ~/llama.cpp): " INSTALL_DIR
        INSTALL_DIR=${INSTALL_DIR:-"$HOME/llama.cpp"}
        
        git clone https://github.com/ggerganov/llama.cpp.git "$INSTALL_DIR"
        cd "$INSTALL_DIR"
        make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)
        
        echo ""
        echo "✓ llama.cpp built successfully"
        echo ""
        echo "Add to your PATH by adding this line to ~/.bashrc or ~/.zshrc:"
        echo "  export PATH=\"\$PATH:$INSTALL_DIR\""
        echo ""
        
        cd - > /dev/null
    fi
fi

# Build the project
echo ""
read -p "Build the project now? (y/n) " -n 1 -r
echo ""

if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo "Building project..."
    mkdir -p build
    cd build
    cmake ..
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu)
    cd ..
    
    echo ""
    echo "✓ Build complete!"
    echo ""
    echo "Run the application with:"
    echo "  ./build/llama_cpp_manager"
fi

echo ""
echo "Setup complete!"
