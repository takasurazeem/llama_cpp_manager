# Quick Start Guide

## 1. Prerequisites Check

```bash
# Check if you have the basics
cmake --version      # Need 3.15+
g++ --version        # Need GCC 8+ or Clang 7+
llama-cli --version  # Need llama.cpp installed
```

## 2. One-Command Setup (Linux/macOS)

```bash
cd llama_cpp_manager
./setup.sh
```

## 3. Manual Build (3 Steps)

```bash
# Step 1: Get Dear ImGui
git clone https://github.com/ocornut/imgui.git extern/imgui

# Step 2: Build
mkdir build && cd build && cmake .. && make -j$(nproc)

# Step 3: Run
./llama_cpp_manager
```

## 4. If llama.cpp is Missing

```bash
# Clone and build llama.cpp
git clone https://github.com/ggerganov/llama.cpp.git ~/llama.cpp
cd ~/llama.cpp && make -j$(nproc)

# Add to PATH (Linux/macOS)
echo 'export PATH="$PATH:$HOME/llama.cpp"' >> ~/.bashrc
source ~/.bashrc

# Verify
llama-cli --version
```

## 5. First Run

1. Launch: `./build/llama_cpp_manager`
2. Click "Browse GGUF Model..."
3. Select a `.gguf` file
4. Adjust sliders (Context Length, GPU Offload)
5. Watch VRAM estimate update in real-time

## Common Parameters

| Setting | What It Does | Recommended |
|---------|--------------|-------------|
| **Context Length** | Max conversation length | 2048-4096 for chat, 8192+ for documents |
| **GPU Offload** | Model layers on GPU | Max your VRAM allows (watch the bar!) |
| **CPU Threads** | Parallel CPU processing | Number of CPU cores |
| **Batch Size** | Parallel token processing | 512 (default is fine) |

## Troubleshooting Quick Fixes

```bash
# App won't start
ldd build/llama_cpp_manager  # Check missing libraries
sudo apt-get install libsdl2-2.0-0  # Install SDL2 runtime

# llama.cpp not found
which llama-cli  # Should show a path
export PATH="$PATH:/path/to/llama.cpp"

# Build errors
rm -rf build  # Clean build
./setup.sh    # Re-run setup
```

## Example Configurations

### Low VRAM (4-6 GB)
- Context: 2048
- GPU Layers: 20-25
- Quantization: Q4_K_M or Q4_0
- Model: 7B parameters max

### Medium VRAM (8-12 GB)
- Context: 4096
- GPU Layers: 30-35
- Quantization: Q5_K_M or Q6_K
- Model: 13B parameters

### High VRAM (16+ GB)
- Context: 8192+
- GPU Layers: All layers
- Quantization: Q8_0 or F16
- Model: 30B+ parameters

## Quick Commands Reference

```bash
# Build
cmake --build build

# Clean rebuild
rm -rf build && mkdir build && cd build && cmake .. && make

# Run with verbose output
./build/llama_cpp_manager --verbose

# Install Dear ImGui
git clone https://github.com/ocornut/imgui.git extern/imgui

# Update Dear ImGui
cd extern/imgui && git pull && cd ../..

# Check GPU
nvidia-smi          # NVIDIA
rocm-smi            # AMD
system_profiler     # macOS
```

## Tips

💡 **Start conservative**: Begin with lower context and fewer GPU layers, then increase until you hit VRAM limits

💡 **Watch the color**: Green = safe, Orange = careful, Red = will OOM

💡 **Save configs**: Check "Remember settings" for frequently used models

💡 **Quantization matters**: Q4_K_M is sweet spot for size/quality

💡 **Context is expensive**: Doubling context roughly doubles KV cache memory

## Getting Help

- 📖 Full docs: See `README.md`
- 🔨 Build issues: See `BUILD.md`
- 🏗️ Architecture: See `DESIGN.md`
- 💬 Questions: Open an issue on GitHub

---

**Ready to use llama.cpp more efficiently? Run `./setup.sh` now!**
