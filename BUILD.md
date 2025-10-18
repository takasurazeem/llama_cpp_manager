# Build Instructions

Detailed build instructions for LLAMA.CPP Manager across different platforms.

## Table of Contents
- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Platform-Specific Instructions](#platform-specific-instructions)
- [Troubleshooting](#troubleshooting)

## Prerequisites

All platforms require:
- CMake 3.15+
- C++17 compatible compiler
- Git

## Quick Start

### Automated Setup (Linux/macOS)

```bash
cd llama_cpp_manager
./setup.sh
```

The setup script will:
1. Check for required dependencies
2. Clone Dear ImGui if needed
3. Detect or help install llama.cpp
4. Build the project

### Quick Build and Run (Development)

For rapid development iterations:

```bash
./build_and_run.sh
```

This script will:
1. Build the project
2. Kill any running instances
3. Launch the updated application
4. Perfect for testing changes quickly

### Manual Setup

```bash
# 1. Clone Dear ImGui
mkdir -p extern
git clone https://github.com/ocornut/imgui.git extern/imgui

# 2. Create build directory
mkdir -p build && cd build

# 3. Configure
cmake ..

# 4. Build
make -j$(nproc)  # Linux
make -j$(sysctl -n hw.ncpu)  # macOS

# 5. Run
./llama_cpp_manager
```

## Platform-Specific Instructions

### Linux (Ubuntu/Debian)

#### Install Dependencies

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libsdl2-dev \
    libgl1-mesa-dev
```

#### Build

```bash
cd llama_cpp_manager
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### Install llama.cpp

```bash
# Option 1: Build from source
git clone https://github.com/ggerganov/llama.cpp.git ~/llama.cpp
cd ~/llama.cpp
make -j$(nproc)

# Add to PATH
echo 'export PATH="$PATH:$HOME/llama.cpp"' >> ~/.bashrc
source ~/.bashrc

# Verify
llama-cli --version
```

### Linux (Fedora/RHEL)

```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    git \
    SDL2-devel \
    mesa-libGL-devel
```

Then follow the same build steps as Ubuntu.

### Linux (Arch)

```bash
sudo pacman -S base-devel cmake git sdl2
```

### macOS

#### Install Dependencies

Using Homebrew:
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake git sdl2
```

#### Build

```bash
cd llama_cpp_manager
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

#### Install llama.cpp

```bash
# Option 1: Using Homebrew
brew install llama.cpp

# Option 2: Build from source
git clone https://github.com/ggerganov/llama.cpp.git ~/llama.cpp
cd ~/llama.cpp
make -j$(sysctl -n hw.ncpu)

# Add to PATH
echo 'export PATH="$PATH:$HOME/llama.cpp"' >> ~/.zshrc
source ~/.zshrc

# Verify
llama-cli --version
```

### Windows

#### Prerequisites

1. **Visual Studio 2019 or 2022** with C++ development tools
2. **CMake** from https://cmake.org/download/
3. **Git** from https://git-scm.com/download/win

#### Option 1: Using vcpkg (Recommended)

```cmd
:: Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
bootstrap-vcpkg.bat
vcpkg integrate install

:: Install dependencies
vcpkg install sdl2:x64-windows

:: Clone project
cd %USERPROFILE%\Developer
git clone <repository-url> llama_cpp_manager
cd llama_cpp_manager

:: Clone Dear ImGui
mkdir extern
git clone https://github.com/ocornut/imgui.git extern\imgui

:: Build
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64 ^
    -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake
cmake --build . --config Release

:: Run
Release\llama_cpp_manager.exe
```

#### Option 2: Manual Dependencies

1. Download SDL2 development libraries from https://www.libsdl.org/download-2.0.php
   - Get SDL2-devel-2.x.x-VC.zip
   - Extract to `C:\SDL2`

2. Build the project:
```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64 ^
    -DSDL2_DIR=C:\SDL2\cmake
cmake --build . --config Release
```

#### Install llama.cpp on Windows

**Option 1: Prebuilt Binaries**
1. Download from https://github.com/ggerganov/llama.cpp/releases
2. Extract to `C:\llama.cpp`
3. Add to PATH:
   - Windows Settings → System → About → Advanced system settings
   - Environment Variables → System Variables → Path
   - Add `C:\llama.cpp`

**Option 2: Build from Source**
```cmd
git clone https://github.com/ggerganov/llama.cpp.git C:\llama.cpp
cd C:\llama.cpp
mkdir build
cd build
cmake ..
cmake --build . --config Release

:: Add to PATH
setx PATH "%PATH%;C:\llama.cpp\build\bin\Release"
```

Verify:
```cmd
llama-cli --version
```

## Build Options

### CMake Configuration Options

```bash
# Debug build
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Release build (default)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Specify compiler
cmake .. -DCMAKE_CXX_COMPILER=clang++

# Use vcpkg
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
```

### GPU-Specific Builds

The application automatically detects available GPUs. For optimal performance:

**NVIDIA:**
- Install CUDA Toolkit (optional, for better detection)
- Ensure nvidia-smi is in PATH

**AMD (Linux):**
- Install ROCm: https://rocm.docs.amd.com/
- Ensure rocm-smi is available

**Apple Silicon:**
- Metal is automatically available on macOS
- No additional setup needed

## Troubleshooting

### CMake can't find SDL2

**Linux:**
```bash
sudo apt-get install libsdl2-dev
# or
sudo dnf install SDL2-devel
```

**macOS:**
```bash
brew install sdl2
```

**Windows:**
```cmd
vcpkg install sdl2:x64-windows
```

### Dear ImGui not found

```bash
# Make sure it's cloned in the correct location
mkdir -p extern
git clone https://github.com/ocornut/imgui.git extern/imgui

# Verify structure
ls extern/imgui/*.cpp  # Should show imgui.cpp, imgui_draw.cpp, etc.
```

### Compiler errors

**"filesystem not found":**
- Ensure C++17 is enabled (should be automatic)
- Update compiler: GCC 8+, Clang 7+, MSVC 2019+

**SDL2 linking errors:**
```bash
# Linux: Install dev package
sudo apt-get install libsdl2-dev

# macOS: Ensure brew SDL2 is found
export CMAKE_PREFIX_PATH="/opt/homebrew"
```

### llama.cpp not detected

The application will show a setup dialog if llama.cpp is not found.

**Verify installation:**
```bash
which llama-cli  # Should show path
llama-cli --version  # Should show version
```

**Add to PATH:**
```bash
# Linux/macOS
export PATH="$PATH:/path/to/llama.cpp"

# Permanently add to ~/.bashrc or ~/.zshrc
echo 'export PATH="$PATH:/path/to/llama.cpp"' >> ~/.bashrc
```

**Windows:**
1. Right-click "This PC" → Properties
2. Advanced system settings → Environment Variables
3. Edit "Path" → Add llama.cpp directory

### Runtime errors

**"SDL2.dll not found" (Windows):**
- Copy SDL2.dll to the same directory as llama_cpp_manager.exe
- Or add SDL2 bin directory to PATH

**"Cannot open display" (Linux):**
- Ensure X11 is running
- Set DISPLAY environment variable: `export DISPLAY=:0`

**Permission denied:**
```bash
# Make executable
chmod +x llama_cpp_manager
```

## Development Build

For development with debugging symbols:

```bash
mkdir build-debug && cd build-debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Run with debugger
gdb ./llama_cpp_manager
```

## IDE Integration

### Visual Studio Code

Install extensions:
- C/C++ (Microsoft)
- CMake Tools

Open folder in VS Code:
```bash
code llama_cpp_manager
```

Configure kit and build target in CMake Tools panel.

### CLion

1. Open project directory
2. CLion will automatically detect CMakeLists.txt
3. Configure toolchain in Settings → Build, Execution, Deployment → Toolchains
4. Build from Run menu

### Visual Studio

1. Open folder or CMakeLists.txt
2. VS will configure CMake automatically
3. Select build configuration (Debug/Release)
4. Build from Build menu

## Clean Build

```bash
# Remove build directory
rm -rf build

# Clean and rebuild
mkdir build && cd build
cmake ..
make clean
make -j$(nproc)
```

## Cross-Compilation

### Build for different architecture

```bash
# ARM64 on x86_64
cmake .. -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
         -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
         -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++
```

## Static Linking (Portable Binary)

For a single-file distributable:

```bash
cmake .. -DBUILD_SHARED_LIBS=OFF \
         -DCMAKE_EXE_LINKER_FLAGS="-static-libgcc -static-libstdc++"
```

Note: SDL2 may still require dynamic linking on some platforms.

## Performance Optimizations

### Release with optimizations

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_CXX_FLAGS="-O3 -march=native"
make -j$(nproc)
```

### Link-Time Optimization (LTO)

```bash
cmake .. -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

## Getting Help

If you encounter issues not covered here:

1. Check GitHub Issues: <repository-url>/issues
2. Review llama.cpp documentation: https://github.com/ggerganov/llama.cpp
3. Create a new issue with:
   - Operating system and version
   - Compiler and version
   - Full error messages
   - Steps to reproduce
