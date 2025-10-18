#!/bin/bash

# LLAMA.CPP Manager - Run Script
# Builds (if needed) and runs the application

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Check if binary exists
if [ ! -f "build/llama_cpp_manager" ]; then
    echo "Binary not found. Building..."
    ./build.sh
fi

# Check if binary needs rebuilding
NEEDS_REBUILD=0

# Check if any source files are newer than binary
if [ -d "src" ]; then
    for file in src/*.cpp; do
        if [ "$file" -nt "build/llama_cpp_manager" ]; then
            NEEDS_REBUILD=1
            break
        fi
    done
fi

# Check if any header files are newer
if [ $NEEDS_REBUILD -eq 0 ] && [ -d "include" ]; then
    for file in include/*.h; do
        if [ "$file" -nt "build/llama_cpp_manager" ]; then
            NEEDS_REBUILD=1
            break
        fi
    done
fi

if [ $NEEDS_REBUILD -eq 1 ]; then
    echo "Source files changed. Rebuilding..."
    ./build.sh
fi

# Run the application
echo "Starting LLAMA.CPP Manager..."
echo ""
./build/llama_cpp_manager "$@"
