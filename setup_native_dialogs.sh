#!/bin/bash

# Setup script for ImGuiFileDialog integration

echo "================================"
echo "ImGuiFileDialog Setup"
echo "================================"

# Check if ImGuiFileDialog already exists
if [ -d "extern/ImGuiFileDialog" ]; then
    echo "✓ ImGuiFileDialog already exists"
    exit 0
fi

echo "Cloning ImGuiFileDialog..."
git clone --depth 1 https://github.com/aiekick/ImGuiFileDialog.git extern/ImGuiFileDialog

if [ $? -eq 0 ]; then
    echo "✓ ImGuiFileDialog cloned successfully"
    echo ""
    echo "ImGuiFileDialog is now available in extern/ImGuiFileDialog"
    echo "Rebuild the project to use native file dialogs."
else
    echo "✗ Failed to clone ImGuiFileDialog"
    exit 1
fi