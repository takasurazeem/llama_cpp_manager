#!/bin/bash

# LLAMA.CPP Manager - Installation Check Script
# Validates the development environment

set -e

echo "====================================="
echo "LLAMA.CPP Manager - Environment Check"
echo "====================================="
echo ""

ERRORS=0
WARNINGS=0

# Function to check command
check_command() {
    local cmd=$1
    local required=$2
    local install_hint=$3
    
    if command -v $cmd &> /dev/null; then
        local version=$($cmd --version 2>&1 | head -n1 | grep -oP '\d+\.\d+(\.\d+)?' | head -n1 || echo "unknown")
        echo "✓ $cmd: $version"
        return 0
    else
        if [ "$required" = "true" ]; then
            echo "✗ $cmd: NOT FOUND (required)"
            echo "  Install: $install_hint"
            ERRORS=$((ERRORS + 1))
        else
            echo "⚠ $cmd: NOT FOUND (optional)"
            echo "  Install: $install_hint"
            WARNINGS=$((WARNINGS + 1))
        fi
        return 1
    fi
}

echo "Core Build Tools:"
echo "-----------------"
check_command "cmake" "true" "sudo apt install cmake (Linux) / brew install cmake (macOS)"
check_command "make" "true" "sudo apt install build-essential (Linux)"
check_command "git" "true" "sudo apt install git (Linux) / brew install git (macOS)"

# Check compiler
echo ""
if check_command "gcc" "false" "sudo apt install build-essential"; then
    GCC_VERSION=$(gcc -dumpversion | cut -d. -f1)
    if [ "$GCC_VERSION" -lt 8 ]; then
        echo "  ⚠ GCC version $GCC_VERSION is old (need 8+)"
        WARNINGS=$((WARNINGS + 1))
    fi
elif check_command "clang" "false" "sudo apt install clang"; then
    CLANG_VERSION=$(clang --version | grep -oP '\d+\.\d+' | head -n1 | cut -d. -f1)
    if [ "$CLANG_VERSION" -lt 7 ]; then
        echo "  ⚠ Clang version $CLANG_VERSION is old (need 7+)"
        WARNINGS=$((WARNINGS + 1))
    fi
else
    echo "✗ No C++ compiler found (gcc or clang)"
    ERRORS=$((ERRORS + 1))
fi

echo ""
echo "Dependencies:"
echo "-------------"

# Check SDL2
if pkg-config --exists sdl2 2>/dev/null; then
    SDL2_VERSION=$(pkg-config --modversion sdl2)
    echo "✓ SDL2: $SDL2_VERSION"
elif [ -f "/usr/include/SDL2/SDL.h" ] || [ -f "/usr/local/include/SDL2/SDL.h" ]; then
    echo "✓ SDL2: installed (version unknown)"
else
    echo "✗ SDL2: NOT FOUND (required)"
    echo "  Install: sudo apt install libsdl2-dev (Linux) / brew install sdl2 (macOS)"
    ERRORS=$((ERRORS + 1))
fi

# Check OpenGL
if pkg-config --exists gl 2>/dev/null; then
    echo "✓ OpenGL: available"
elif [ -f "/usr/include/GL/gl.h" ] || [ -f "/System/Library/Frameworks/OpenGL.framework" ]; then
    echo "✓ OpenGL: available"
else
    echo "⚠ OpenGL: NOT FOUND"
    echo "  Usually provided by GPU drivers"
    WARNINGS=$((WARNINGS + 1))
fi

echo ""
echo "Project Files:"
echo "--------------"

# Check Dear ImGui
if [ -d "extern/imgui" ] && [ -f "extern/imgui/imgui.cpp" ]; then
    echo "✓ Dear ImGui: installed"
else
    echo "✗ Dear ImGui: NOT FOUND (required)"
    echo "  Install: git clone https://github.com/ocornut/imgui.git extern/imgui"
    ERRORS=$((ERRORS + 1))
fi

# Check source files
if [ -d "src" ] && [ -d "include" ]; then
    SRC_COUNT=$(find src -name "*.cpp" | wc -l)
    HDR_COUNT=$(find include -name "*.h" | wc -l)
    echo "✓ Source files: $SRC_COUNT .cpp, $HDR_COUNT .h"
else
    echo "✗ Source files: MISSING"
    ERRORS=$((ERRORS + 1))
fi

# Check CMakeLists.txt
if [ -f "CMakeLists.txt" ]; then
    echo "✓ CMakeLists.txt: present"
else
    echo "✗ CMakeLists.txt: MISSING"
    ERRORS=$((ERRORS + 1))
fi

echo ""
echo "Runtime (Optional):"
echo "-------------------"

# Check llama.cpp
if check_command "llama-cli" "false" "Build from https://github.com/ggerganov/llama.cpp"; then
    LLAMA_PATH=$(which llama-cli)
    echo "  Path: $LLAMA_PATH"
elif check_command "main" "false" "Build from https://github.com/ggerganov/llama.cpp"; then
    LLAMA_PATH=$(which main)
    echo "  Path: $LLAMA_PATH"
else
    echo "  Note: llama.cpp can be installed later"
fi

# Check for llama-gguf-dump
if command -v llama-gguf-dump &> /dev/null; then
    echo "✓ llama-gguf-dump: available"
elif command -v gguf-dump &> /dev/null; then
    echo "✓ gguf-dump: available"
else
    echo "⚠ gguf-dump: not found (optional, enhances model info)"
fi

echo ""
echo "GPU Tools (Optional):"
echo "---------------------"

# Check GPU detection tools
if command -v nvidia-smi &> /dev/null; then
    GPU_NAME=$(nvidia-smi --query-gpu=name --format=csv,noheader 2>/dev/null | head -n1 || echo "unknown")
    echo "✓ nvidia-smi: $GPU_NAME"
elif command -v rocm-smi &> /dev/null; then
    echo "✓ rocm-smi: AMD GPU detected"
elif [ -d "/System/Library/Frameworks/Metal.framework" ]; then
    echo "✓ Metal: Apple GPU available"
else
    echo "⚠ No GPU detection tools found (CPU mode only)"
    WARNINGS=$((WARNINGS + 1))
fi

echo ""
echo "====================================="
echo "Summary:"
echo "====================================="
echo ""

if [ $ERRORS -eq 0 ] && [ $WARNINGS -eq 0 ]; then
    echo "✓ All checks passed! Ready to build."
    echo ""
    echo "Next step: ./build.sh"
    exit 0
elif [ $ERRORS -eq 0 ]; then
    echo "⚠ $WARNINGS warning(s) found, but you can proceed."
    echo ""
    echo "Next step: ./build.sh"
    exit 0
else
    echo "✗ $ERRORS error(s) and $WARNINGS warning(s) found."
    echo ""
    echo "Fix the errors above before building."
    exit 1
fi
