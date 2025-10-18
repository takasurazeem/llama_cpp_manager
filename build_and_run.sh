#!/bin/bash

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Change to the script directory (project root)
cd "$SCRIPT_DIR"

echo "Building llama_cpp_manager..."
./build.sh

# Check if build was successful
if [ $? -eq 0 ]; then
    echo ""
    echo "Build successful! Restarting application..."
    echo ""
    
    # Kill any running instances
    pkill -f llama_cpp_manager
    
    # Wait a moment for process to fully terminate
    sleep 0.5
    
    # Run the application
    ./run.sh
else
    echo ""
    echo "Build failed! Please check the errors above."
    exit 1
fi