# Troubleshooting Guide

Common issues and solutions for LLAMA.CPP Manager.

## Build Issues

### "CMake not found"
```bash
# Linux (Debian/Ubuntu)
sudo apt-get install cmake

# Linux (Fedora/RHEL)
sudo dnf install cmake

# macOS
brew install cmake
```

### "SDL2 not found"
```bash
# Linux (Debian/Ubuntu)
sudo apt-get install libsdl2-dev

# Linux (Fedora/RHEL)
sudo dnf install SDL2-devel

# macOS
brew install sdl2

# Windows (vcpkg)
vcpkg install sdl2:x64-windows
```

### "Dear ImGui not found"
```bash
# Clone Dear ImGui
git clone https://github.com/ocornut/imgui.git extern/imgui

# Verify
ls extern/imgui/imgui.cpp  # Should exist
```

### "No such file or directory: bits/c++config.h"
This usually means C++ standard library headers are missing.

```bash
# Linux (Debian/Ubuntu)
sudo apt-get install build-essential g++

# Linux (Fedora/RHEL)
sudo dnf install gcc-c++
```

### Build fails with "undefined reference to SDL_*"
SDL2 linking issue. Try:

```bash
# Rebuild from scratch
make clean
rm -rf build
cmake -B build
make -C build
```

If still failing, check SDL2 installation:
```bash
pkg-config --libs --cflags sdl2
```

### "filesystem not found" or "std::filesystem"
Your compiler may not support C++17 fully.

```bash
# Check compiler version
g++ --version  # Need GCC 8+ or Clang 7+

# Update on Ubuntu
sudo apt-get install g++-10
export CXX=g++-10
```

### "'std::string' has no member named 'ends_with'"
This is a C++20 feature that was mistakenly used. Should be fixed in current version.

**Solution**: Pull latest code. The codebase now uses C++17 compatible string comparison.

```bash
git pull origin main
./build.sh
```

If you're modifying code and encounter this:
```cpp
// Wrong (C++20):
if (str.ends_with(".gguf"))

// Right (C++17):
const std::string suffix = ".gguf";
if (str.length() >= suffix.length() && 
    str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0)
```

## Runtime Issues

### llama.cpp Not Detected

**Symptom**: Setup dialog appears saying llama.cpp not found

**Solutions**:

1. **Install llama.cpp**:
```bash
git clone https://github.com/ggerganov/llama.cpp.git ~/llama.cpp
cd ~/llama.cpp
make -j$(nproc)
```

2. **Add to PATH**:
```bash
# Linux/macOS (temporary)
export PATH="$PATH:$HOME/llama.cpp"

# Linux (permanent - bash)
echo 'export PATH="$PATH:$HOME/llama.cpp"' >> ~/.bashrc
source ~/.bashrc

# macOS (permanent - zsh)
echo 'export PATH="$PATH:$HOME/llama.cpp"' >> ~/.zshrc
source ~/.zshrc

# Windows
setx PATH "%PATH%;C:\llama.cpp"
```

3. **Specify custom path**: Use the "Specify llama.cpp Location" option in the setup dialog

4. **Verify installation**:
```bash
which llama-cli  # Should show path
llama-cli --version  # Should show version
```

### "Cannot open display" (Linux)

**Symptom**: Error about X11 display

**Solutions**:
```bash
# Check if X11 is running
echo $DISPLAY  # Should show something like :0

# Set display if needed
export DISPLAY=:0

# If using SSH, enable X11 forwarding
ssh -X user@host

# Or use Xvfb for headless
Xvfb :99 -screen 0 1024x768x24 &
export DISPLAY=:99
```

### "SDL2.dll not found" (Windows)

**Symptom**: Windows complains about missing SDL2.dll

**Solutions**:
1. Copy SDL2.dll to the same directory as llama_cpp_manager.exe
2. Or add SDL2 bin directory to PATH
3. Rebuild with static linking (advanced)

### Segmentation Fault on Startup

**Possible causes**:
1. Dear ImGui not properly linked
2. OpenGL driver issue
3. Corrupted binary

**Solutions**:
```bash
# Rebuild from scratch
make clean rebuild

# Check for missing shared libraries
ldd build/llama_cpp_manager  # Linux
otool -L build/llama_cpp_manager  # macOS

# Run with debugger
gdb ./build/llama_cpp_manager
run
backtrace  # If it crashes
```

### Window Doesn't Appear

**Possible causes**:
- OpenGL driver not installed
- GPU driver issue
- Compositor conflict (Linux)

**Solutions**:
```bash
# Check OpenGL
glxinfo | grep "OpenGL version"  # Linux

# Update GPU drivers
# NVIDIA: Download from nvidia.com
# AMD: Install mesa or AMDGPU-PRO
# Intel: Usually in mesa-utils

# Try software rendering (slow but works)
export LIBGL_ALWAYS_SOFTWARE=1
./build/llama_cpp_manager
```

## Model Loading Issues

### "Not a valid GGUF file"

**Symptom**: Error when trying to load a model

**Solutions**:
1. Ensure file has `.gguf` extension
2. Verify file isn't corrupted:
```bash
# Check file size (should be >100 MB typically)
ls -lh model.gguf

# Try with llama.cpp directly
llama-cli -m model.gguf --version
```

3. Re-download the model if corrupted

### Model Info Not Displayed

**Symptom**: Model loads but info panel is empty

**Solutions**:
1. Install `llama-gguf-dump` or `gguf-dump`:
```bash
cd ~/llama.cpp
make llama-gguf-dump
```

2. Check llama.cpp version (need recent version)
3. File permissions - ensure model file is readable

### VRAM Estimate Shows 0 or Wrong Value

**Possible causes**:
- Model metadata not parsed correctly
- llama.cpp tools not available
- Model file incomplete

**Solutions**:
1. Verify llama.cpp is working:
```bash
llama-cli -m model.gguf -n 0 --verbose
```

2. Check model file integrity
3. Try different model to isolate issue

## GPU Detection Issues

### "No GPU detected"

**Symptom**: Application shows "CPU mode only"

**Solutions**:

**NVIDIA**:
```bash
# Check if nvidia-smi works
nvidia-smi

# If not, install/update NVIDIA drivers
# Ubuntu: sudo ubuntu-drivers autoinstall
# Or download from nvidia.com
```

**AMD (Linux)**:
```bash
# Check ROCm installation
rocm-smi

# Install ROCm if needed
# See: https://rocm.docs.amd.com/
```

**Apple**:
Metal should work automatically on macOS. If not, check:
```bash
# Verify Metal support
system_profiler SPDisplaysDataType | grep Metal
```

### GPU Detected but VRAM Shows Wrong Amount

**Solutions**:
1. Update GPU drivers
2. Check other applications using VRAM:
```bash
# NVIDIA
nvidia-smi

# AMD
rocm-smi
```

3. Restart computer (driver state issue)

## Configuration Issues

### Settings Not Saved

**Possible causes**:
- No write permission to config directory
- Disk full
- Config file corrupted

**Solutions**:
```bash
# Check config directory
ls -la ~/.config/llama_manager/

# Fix permissions
chmod 755 ~/.config/llama_manager/
chmod 644 ~/.config/llama_manager/config.json

# Delete corrupted config
rm ~/.config/llama_manager/config.json

# Check disk space
df -h ~/.config
```

### Configuration Loads Wrong Values

**Solutions**:
1. Delete config and start fresh:
```bash
rm ~/.config/llama_manager/config.json
```

2. Use example config:
```bash
cp examples/config_default.json ~/.config/llama_manager/config.json
```

## Performance Issues

### Application Slow to Start

**Causes**:
- Scanning large directories for models
- GPU detection slow
- llama.cpp detection searching many paths

**Solutions**:
1. Reduce model search scope
2. Specify llama.cpp path in config
3. Use SSD instead of HDD

### UI Laggy/Stuttering

**Solutions**:
1. Update GPU drivers
2. Disable vsync (might cause tearing):
   - Modify `SDL_GL_SetSwapInterval(0)` in main.cpp
3. Reduce window size
4. Close other GPU-intensive applications

### High CPU Usage When Idle

**This should not happen**. If it does:
1. Check for infinite loops in event handling
2. Rebuild with optimization:
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
```

3. Profile the application:
```bash
perf record -g ./build/llama_cpp_manager
perf report
```

## Memory Issues

### "Out of Memory" Errors

**During build**:
```bash
# Reduce parallel jobs
make -j2  # Instead of -j$(nproc)
```

**During runtime**:
- This shouldn't happen (app uses <50 MB)
- Check for memory leaks with valgrind:
```bash
valgrind --leak-check=full ./build/llama_cpp_manager
```

### Application Crashes After Running for a While

**Possible causes**:
- Memory leak
- GPU driver issue
- File descriptor leak

**Debug**:
```bash
# Run with memory debugging
valgrind ./build/llama_cpp_manager

# Check resource usage
top -p $(pgrep llama_cpp_manager)

# Check file descriptors
lsof -p $(pgrep llama_cpp_manager)
```

## Platform-Specific Issues

### macOS: "Cannot open because developer cannot be verified"

**Solution**:
```bash
# Remove quarantine attribute
xattr -d com.apple.quarantine build/llama_cpp_manager

# Or allow in System Preferences:
# Security & Privacy → General → Open Anyway
```

### macOS: "Metal device not found"

**Solution**:
Ensure you're on macOS 10.13+ with Metal-capable GPU

### Windows: "VCRUNTIME140.dll not found"

**Solution**:
Install Visual C++ Redistributable:
https://aka.ms/vs/17/release/vc_redist.x64.exe

### Linux: "version GLIBCXX_X.X.X not found"

**Solution**:
```bash
# Update libstdc++
sudo apt-get update
sudo apt-get install libstdc++6

# Or use newer compiler
sudo apt-get install g++-11
export CXX=g++-11
```

## Getting Help

If none of these solutions work:

1. **Check environment**:
```bash
./check_env.sh
```

2. **Gather information**:
```bash
# System info
uname -a
lsb_release -a  # Linux

# Compiler version
g++ --version
cmake --version

# GPU info
nvidia-smi  # NVIDIA
rocm-smi  # AMD
system_profiler SPDisplaysDataType  # macOS

# llama.cpp version
llama-cli --version
```

3. **Create an issue** with:
   - Operating system and version
   - Compiler and version
   - GPU model and driver version
   - Full error messages
   - Output from `./check_env.sh`
   - Steps to reproduce

4. **Search existing issues**:
   - GitHub Issues: <repository-url>/issues
   - llama.cpp Issues: https://github.com/ggerganov/llama.cpp/issues

## Known Limitations

1. **No web UI**: This is a desktop application only
2. **No model loading**: Only estimates, doesn't run inference
3. **Single GPU only**: Multi-GPU support not yet implemented
4. **Simple file browser**: Native dialogs not yet integrated

## Workarounds

### Need to test actual VRAM usage?

Use llama.cpp directly:
```bash
llama-cli -m model.gguf -c 4096 -ngl 32 --verbose
```

### Need to run inference?

Use llama.cpp, ollama, or LM Studio for actual model execution.

### Need remote model access?

Download models locally first. HTTP loading not yet supported.

---

**Still stuck?** Open an issue with detailed information!
