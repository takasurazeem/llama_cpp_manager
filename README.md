# LLAMA.CPP Manager

A lightweight, cross-platform desktop utility for configuring GGUF models and estimating VRAM usage.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey.svg)

## Overview

LLAMA.CPP Manager is an extremely lightweight desktop application that helps you:
- Select and analyze local GGUF models
- Configure inference parameters (context length, GPU offload, batch size, etc.)
- Estimate VRAM and RAM usage before loading models
- Detect available GPU resources
- Save and restore configuration profiles

**Key Features:**
- ✨ Ultra-lightweight: <50 MB VRAM footprint
- 🚀 Fast startup and single-binary deployment
- 🖥️ Cross-platform: Windows, macOS, Linux
- 🎨 Clean Dear ImGui interface
- 🔧 Integrates with llama.cpp tools for accurate model analysis
- 🗂️ **NEW**: Functional file browser for easy model selection
- 📤 **NEW**: Export configurations to llama.cpp CLI commands
- 🎨 **NEW**: Optional native file dialog support
- 📚 **NEW**: Recent models list with per-model settings persistence
- 💾 **NEW**: Hybrid configuration storage for better organization

## Requirements

### Required
- **llama.cpp**: This utility requires llama.cpp to be installed on your system
  - Download from: https://github.com/ggerganov/llama.cpp
  - Ensure `llama-cli` (or `main`) is in your system PATH
  - Optionally, `llama-gguf-dump` for detailed model inspection

### Build Dependencies
- C++17 compatible compiler (GCC 8+, Clang 7+, MSVC 2019+)
- CMake 3.15 or higher
- SDL2
- OpenGL 3.0+

## Installation

### Prerequisites

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libsdl2-dev
```

#### macOS
```bash
brew install cmake sdl2
```

#### Windows
Install vcpkg and use it to install dependencies:
```cmd
vcpkg install sdl2:x64-windows
```

### Building from Source

1. **Clone the repository:**
```bash
cd ~/Developer
git clone https://github.com/yourusername/llama_cpp_manager.git
cd llama_cpp_manager
```

2. **Download Dear ImGui** (if not using as submodule):
```bash
mkdir -p extern
cd extern
git clone https://github.com/ocornut/imgui.git
cd ..
```

3. **Build the project:**
```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

On Windows with Visual Studio:
```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release
```

4. **Run the application:**
```bash
./llama_cpp_manager
```

## llama.cpp Installation

### Quick Setup

#### Linux
```bash
# Clone llama.cpp
git clone https://github.com/ggerganov/llama.cpp.git
cd llama.cpp

# Build
make

# Add to PATH (add to ~/.bashrc or ~/.zshrc)
export PATH="$PATH:$(pwd)"
```

#### macOS
```bash
# Using Homebrew
brew install llama.cpp

# Or build from source
git clone https://github.com/ggerganov/llama.cpp.git
cd llama.cpp
make
```

#### Windows
1. Download prebuilt binaries from llama.cpp releases
2. Or build with CMake:
```cmd
git clone https://github.com/ggerganov/llama.cpp.git
cd llama.cpp
mkdir build
cd build
cmake ..
cmake --build . --config Release
```
3. Add the build directory to your system PATH

### Verifying Installation

Run this command to verify llama.cpp is installed:
```bash
llama-cli --version
```

If the utility can't find llama.cpp, you'll see a setup dialog where you can:
1. Install llama.cpp and retry detection
2. Specify a custom installation path

## What's New in v1.1.0

🎉 **Major Feature Update!**

- **Functional File Browser**: Browse and select GGUF models with an intuitive UI
- **CLI Command Export**: Generate ready-to-use `llama-cli` commands with your settings
- **Native File Dialogs** (optional): Professional cross-platform file dialogs

See [FEATURES.md](FEATURES.md) for detailed information about new features.

---

## Usage

### Basic Workflow

1. **Launch the application**
   - On first run, the app will detect llama.cpp installation
   - If not found, you'll be prompted to install or specify the path

2. **Select a GGUF model**
   - Click "Browse GGUF Model..."
   - Navigate through directories to find your model
   - Select a `.gguf` file (e.g., `llama-2-7b-Q4_K_M.gguf`)
   - Model info loads automatically!
   - Or select from **Recently Selected Models** list

3. **Configure parameters**
   - **Context Length**: Maximum context window (tokens)
   - **GPU Offload**: Number of layers to offload to GPU
   - **CPU Threads**: Thread pool size for CPU inference
   - **Batch Size**: Evaluation batch size

4. **Monitor VRAM estimates**
   - Real-time VRAM/RAM usage estimates
   - Warning indicators if configuration exceeds available VRAM

5. **Export to CLI** (NEW!)
   - Click "Export CLI Command" to generate llama.cpp command
   - Copy to clipboard and run in terminal

6. **Save configuration**
   - Check "Remember settings" to save per-model configurations
   - Settings automatically restored when selecting model from recent list

### Advanced Settings

Enable "Show advanced settings" to access:
- **RoPE Frequency Base/Scale**: For extended context
- **Flash Attention**: Experimental faster attention (may not work with all models)
- **KV Cache Quantization**: Reduce memory usage
- **MoE Settings**: For Mixtral and other MoE models
- **Expert CPU Offloading**: Force expert weights to CPU

## Configuration Parameters

### Core Settings

| Parameter | Description | Default | Range |
|-----------|-------------|---------|-------|
| Context Length | Maximum sequence length | 2048 | 512 - 262144 |
| GPU Offload | Layers offloaded to GPU | 0 | 0 - model layers |
| CPU Threads | Thread pool size | 8 | 1 - 32 |
| Batch Size | Evaluation batch size | 512 | 32 - 2048 |

### Memory Optimization

| Parameter | Description | Default |
|-----------|-------------|---------|
| Offload KV Cache | Store KV cache on GPU | ✓ |
| Keep Model in Memory | Don't unload between uses | ✓ |
| Use mmap() | Memory-map model file | ✓ |

### Experimental Features

⚠️ **Warning**: Experimental features may cause instability

- **Flash Attention**: Faster attention mechanism
- **K/V Cache Quantization**: Reduce cache memory
- **Force Experts to CPU**: For MoE models with limited VRAM

## VRAM Estimation

The utility estimates memory usage using:

```
Total VRAM = Model Weights + KV Cache + Input/Output Buffers + Overhead
```

### Formulas

**Model Weights (GPU):**
```
GPU Weights = Total Model Size × (GPU Layers / Total Layers)
```

**KV Cache:**
```
KV Cache = 2 × Layers × Context Length × Embedding Dim × Bytes Per Element
```

**Overhead:**
- CUDA: ~512 MB
- Metal: ~256 MB
- CPU-only: ~512 MB

## Project Structure

```
llama_cpp_manager/
├── CMakeLists.txt           # Build configuration
├── README.md                # This file
├── LICENSE                  # MIT License
├── extern/                  # External dependencies
│   └── imgui/              # Dear ImGui (submodule)
├── include/                 # Header files
│   ├── gguf_reader.h       # GGUF file parser
│   ├── vram_estimator.h    # Memory calculation
│   ├── gpu_detector.h      # GPU detection
│   ├── llama_cpp_interface.h  # llama.cpp integration
│   ├── config_manager.h    # Configuration I/O
│   └── file_browser.h      # File selection
├── src/                     # Implementation files
│   ├── main.cpp            # Application entry & UI
│   ├── gguf_reader.cpp
│   ├── vram_estimator.cpp
│   ├── gpu_detector.cpp
│   ├── llama_cpp_interface.cpp
│   ├── config_manager.cpp
│   └── file_browser.cpp
└── build/                   # Build output (generated)
```

## GPU Detection

The application automatically detects GPUs using:

- **NVIDIA**: `nvidia-smi` command
- **AMD**: `rocm-smi` command (Linux)
- **Apple**: Metal framework (macOS)
- **Fallback**: Vulkan device enumeration

If GPU detection fails, the app will default to CPU-only mode.

## Configuration Files

Settings are stored in a **hybrid configuration system** for better organization:

### Global Configuration
- **Linux/macOS**: `~/.config/llama_manager/config.json`
- **Windows**: `%USERPROFILE%\.config\llama_manager\config.json`

Contains:
- Global default settings
- Recent models list (up to 10 models)
- Last selected model path
- Remember settings preference

### Per-Model Configuration
- **Linux/macOS**: `~/.config/llama_manager/models/<hash>.json`
- **Windows**: `%USERPROFILE%\.config\llama_manager\models\<hash>.json`

Each model gets its own configuration file (hash based on path):
- Model-specific inference settings
- Persists independently
- Automatically loaded when selecting from recent list
- Only saved when "Remember settings" is enabled

Example global config:
```json
{
  "context_length": 4096,
  "gpu_layers": 32,
  "batch_size": 512,
  "threads": 10,
  "offload_kv_cache": true,
  "use_mmap": true,
  "last_model_path": "/path/to/model.gguf",
  "remember_settings": true,
  "recent_models": [
    "/path/to/model1.gguf",
    "/path/to/model2.gguf"
  ]
}
```

See [MODEL_SETTINGS.md](MODEL_SETTINGS.md) for detailed information about the configuration system.

## Troubleshooting

### "llama.cpp not found"
- Ensure llama.cpp is installed and `llama-cli` is in your PATH
- Use the "Specify llama.cpp Location" option in the setup dialog
- Verify installation: `llama-cli --version`

### "No GPU detected"
- Install GPU drivers (NVIDIA CUDA, AMD ROCm, etc.)
- Check GPU detection: `nvidia-smi` or `rocm-smi`
- Application will fall back to CPU mode

### Build errors
- Ensure all dependencies are installed
- Update CMake: `cmake --version` (need 3.15+)
- Check compiler version supports C++17

### High VRAM estimates
- Reduce context length
- Decrease GPU layer offload
- Enable KV cache quantization
- Use a more quantized model variant (e.g., Q4_K_M instead of Q8_0)

## Performance Tips

1. **Model Selection**
   - Q4_K_M offers best quality/size balance
   - Q8_0 for maximum quality (2× size)
   - IQ2_XXS/IQ3_XXS for extreme compression

2. **GPU Offload**
   - Offload as many layers as VRAM allows
   - Leave 1-2 GB VRAM free for system/driver

3. **Context Length**
   - Use minimum needed for your task
   - KV cache grows linearly with context

4. **Batch Size**
   - Larger = faster (more parallel processing)
   - Smaller = lower memory usage

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Acknowledgments

- [llama.cpp](https://github.com/ggerganov/llama.cpp) - Inference engine
- [Dear ImGui](https://github.com/ocornut/imgui) - Immediate mode GUI
- [SDL2](https://www.libsdl.org/) - Cross-platform window/input

## Support

- Issues: https://github.com/yourusername/llama_cpp_manager/issues
- Discussions: https://github.com/yourusername/llama_cpp_manager/discussions
- llama.cpp: https://github.com/ggerganov/llama.cpp

---

**Note**: This utility does not load or run models. It only analyzes model files and estimates resource requirements. Use llama.cpp directly for inference.
