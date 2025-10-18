# Project Structure

```
llama_cpp_manager/
│
├── 📄 README.md                    # Main documentation
├── 📄 QUICKSTART.md               # Quick start guide
├── 📄 BUILD.md                    # Detailed build instructions
├── 📄 DESIGN.md                   # Architecture and design decisions
├── 📄 LICENSE                     # MIT License
├── 📄 CMakeLists.txt             # CMake build configuration
├── 📄 vcpkg.json                 # vcpkg dependencies
├── 📄 .gitignore                 # Git ignore rules
├── 🔧 setup.sh                   # Automated setup script (Linux/macOS)
│
├── 📁 include/                    # Header files (public interfaces)
│   ├── gguf_reader.h            # GGUF file parser interface
│   ├── vram_estimator.h         # Memory calculation engine
│   ├── gpu_detector.h           # GPU hardware detection
│   ├── llama_cpp_interface.h    # llama.cpp integration layer
│   ├── config_manager.h         # Configuration persistence
│   └── file_browser.h           # File/directory navigation
│
├── 📁 src/                        # Implementation files
│   ├── main.cpp                 # Application entry + Dear ImGui UI
│   ├── gguf_reader.cpp          # GGUF parsing implementation
│   ├── vram_estimator.cpp       # Memory calculation logic
│   ├── gpu_detector.cpp         # Platform-specific GPU detection
│   ├── llama_cpp_interface.cpp  # llama.cpp command execution
│   ├── config_manager.cpp       # JSON config I/O
│   └── file_browser.cpp         # Directory scanning
│
├── 📁 extern/                     # External dependencies
│   └── imgui/                   # Dear ImGui (clone separately)
│       ├── imgui.cpp
│       ├── imgui_draw.cpp
│       ├── imgui_widgets.cpp
│       ├── imgui_tables.cpp
│       └── backends/
│           ├── imgui_impl_sdl2.cpp
│           └── imgui_impl_opengl3.cpp
│
└── 📁 build/                      # Build output (generated)
    └── llama_cpp_manager        # Executable binary

```

## File Purposes

### Documentation (📄)
- **README.md**: User-facing documentation, installation, usage
- **QUICKSTART.md**: Fast setup for impatient users (3 commands)
- **BUILD.md**: Platform-specific build instructions (Windows, Linux, macOS)
- **DESIGN.md**: Technical architecture, formulas, design decisions
- **LICENSE**: MIT License

### Build System (🔧)
- **CMakeLists.txt**: CMake configuration, links SDL2/OpenGL/ImGui
- **vcpkg.json**: Dependency manifest for vcpkg users
- **setup.sh**: Automated setup (checks deps, clones ImGui, builds)

### Core Headers (include/) 
Each header defines clean interfaces with minimal dependencies:

1. **llama_cpp_interface.h**
   - Detects llama.cpp installation
   - Executes CLI commands (`llama-cli`, `llama-gguf-dump`)
   - Parses tool output for model metadata

2. **gguf_reader.h**
   - Reads GGUF magic number and headers
   - Detects quantization from filename
   - Fallback when llama.cpp unavailable

3. **vram_estimator.h**
   - Calculates memory breakdown (weights, KV cache, buffers)
   - Considers quantization, context length, GPU layers
   - Returns estimate in bytes/GB

4. **gpu_detector.h**
   - Detects NVIDIA (nvidia-smi), AMD (rocm-smi), Apple (Metal)
   - Queries available VRAM
   - Fallback to Vulkan enumeration

5. **config_manager.h**
   - Saves/loads JSON configuration
   - Per-model settings persistence
   - Cross-platform config paths

6. **file_browser.h**
   - Directory traversal
   - GGUF file filtering
   - Future: Native file dialogs

### Core Implementation (src/)

1. **main.cpp** (~500-700 lines)
   - SDL2 + OpenGL + ImGui initialization
   - Main render loop
   - UI layout (model selection, sliders, toggles)
   - llama.cpp setup dialog
   - Real-time VRAM estimate display

2. **llama_cpp_interface.cpp** (~400-500 lines)
   - PATH searching (Linux: `/usr/local/bin`, macOS: `/opt/homebrew`, Windows: `C:\Program Files`)
   - Command execution (cross-platform: popen vs CreateProcess)
   - Output parsing (regex for version, metadata)
   - Custom path validation

3. **gguf_reader.cpp** (~200-300 lines)
   - Binary file reading
   - Magic number validation ("GGUF")
   - Quantization string → enum mapping
   - Bits per weight lookup table

4. **vram_estimator.cpp** (~200-300 lines)
   - Formula implementations
   - GPU/CPU memory split calculation
   - KV cache sizing (context × embedding × layers)
   - Overhead constants (CUDA: 512MB)

5. **gpu_detector.cpp** (~300-400 lines)
   - Platform-specific GPU APIs
   - nvidia-smi parsing (CSV output)
   - Metal framework calls (Objective-C++)
   - Fallback detection chain

6. **config_manager.cpp** (~150-200 lines)
   - Simple JSON serialization (key:value parsing)
   - Config directory creation
   - Default values

7. **file_browser.cpp** (~100-150 lines)
   - std::filesystem directory iteration
   - .gguf extension filtering
   - Recursive scanning

### External Dependencies (extern/)

- **imgui/**: Cloned from https://github.com/ocornut/imgui
  - Not included in repo (user clones via setup.sh)
  - ~6 core .cpp files + 2 backend files
  - Header-mostly library (~30k lines total)

### Build Output (build/)

Generated by CMake:
- Linux: `llama_cpp_manager` (ELF binary, ~2-5 MB)
- macOS: `llama_cpp_manager` (Mach-O binary)
- Windows: `llama_cpp_manager.exe` + SDL2.dll

## Dependency Graph

```
main.cpp
  ├─→ llama_cpp_interface.h
  │     └─→ gguf_reader.h
  ├─→ vram_estimator.h
  │     └─→ gguf_reader.h
  ├─→ gpu_detector.h
  ├─→ config_manager.h
  │     └─→ vram_estimator.h
  └─→ file_browser.h

External:
  ├─→ imgui/*
  ├─→ SDL2
  └─→ OpenGL
```

## Code Statistics (Estimated)

| Component | Lines | Complexity |
|-----------|-------|------------|
| Headers | ~800 | Low |
| Implementations | ~2500 | Medium |
| Main UI | ~700 | Medium |
| **Total** | **~4000** | **Low-Medium** |

## Build Artifacts

```
build/
├── CMakeCache.txt
├── CMakeFiles/
├── Makefile
├── compile_commands.json      # For IDE integration
└── llama_cpp_manager          # Final executable
```

## Installation Footprint

- **Source**: ~100 KB (without extern/)
- **With ImGui**: ~5 MB
- **Binary**: 2-5 MB
- **Runtime memory**: <50 MB VRAM
- **Config files**: <10 KB

## Next Steps for Users

1. Clone Dear ImGui: `git clone https://github.com/ocornut/imgui extern/imgui`
2. Run setup: `./setup.sh`
3. Build: `cd build && make`
4. Run: `./llama_cpp_manager`

## Next Steps for Developers

1. Add native file dialogs (ImGuiFileDialog)
2. Implement full GGUF parser (no llama.cpp dependency)
3. Add export to llama.cpp command line
4. Create installers (AppImage, DMG, NSIS)
5. Add unit tests (Google Test)
6. CI/CD (GitHub Actions)

---

**Simple, focused, efficient. That's the goal.** 🚀
