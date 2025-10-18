# LLAMA.CPP Manager - Project Summary

## Overview

A lightweight, cross-platform C++ desktop utility for GGUF model configuration and VRAM estimation, designed to be an extremely efficient alternative to heavier tools like LM Studio.

## Key Design Decisions

### 1. llama.cpp Integration
- **Does not reinvent the wheel**: Leverages llama.cpp's existing tools (`llama-cli`, `llama-gguf-dump`)
- **Detection system**: Automatically finds llama.cpp or prompts user to install/configure
- **Validation**: Ensures llama.cpp is available before allowing model operations

### 2. Architecture

```
┌─────────────────────────────────────────────────────────┐
│                     Main Application                     │
│                    (Dear ImGui + SDL2)                   │
└──────────────┬──────────────────────────┬────────────────┘
               │                          │
        ┌──────▼────────┐         ┌───────▼────────┐
        │ llama.cpp     │         │  GPU Detector  │
        │  Interface    │         │   (nvidia-smi, │
        │               │         │   rocm-smi,    │
        │ - Detection   │         │   Metal, etc.) │
        │ - Execute CLI │         └────────────────┘
        │ - Parse output│
        └──────┬────────┘
               │
        ┌──────▼────────┐
        │ GGUF Reader   │
        │ Model Info    │
        └───────────────┘
               │
        ┌──────▼────────┐
        │ VRAM          │
        │ Estimator     │
        └───────────────┘
```

### 3. Memory Footprint

**Target**: <50 MB VRAM
- Dear ImGui: ~5-10 MB
- SDL2 window: ~2-5 MB
- Application logic: ~1-3 MB
- OpenGL context: ~10-20 MB
- **Total**: ~20-40 MB typical usage

**Comparison**:
- Electron-based tools: 200-500 MB base
- This utility: 20-40 MB (5-10× more efficient)

### 4. Supported Platforms

| Platform | Status | Detection Method | Notes |
|----------|--------|------------------|-------|
| Linux | ✅ | nvidia-smi, rocm-smi, Vulkan | Full support |
| macOS | ✅ | Metal framework | Native integration |
| Windows | ✅ | nvidia-smi, WMI | CUDA recommended |

## Component Descriptions

### llama_cpp_interface.h/.cpp
**Purpose**: Bridge to llama.cpp tools

**Features**:
- Automatic detection in PATH and common locations
- Custom path configuration
- Executes llama.cpp commands and parses output
- Version detection
- Model metadata extraction via `llama-gguf-dump`
- VRAM estimation via `llama-cli --verbose`

**Detection Flow**:
1. Search PATH for `llama-cli` or `main`
2. Check common locations:
   - Linux: `/usr/local/bin`, `~/.local/bin`, `/opt/llama.cpp`
   - macOS: `/opt/homebrew/bin`, `/usr/local/bin`
   - Windows: `C:\Program Files\llama.cpp`, `%LOCALAPPDATA%\llama.cpp`
3. If not found: Show setup dialog
4. Allow user to specify custom path

### gguf_reader.h/.cpp
**Purpose**: Parse GGUF file headers and detect model properties

**Features**:
- Read GGUF magic number and validate files
- Extract basic model info without full parsing
- Detect quantization from filename (Q4_K_M, Q8_0, etc.)
- Fallback when llama.cpp tools unavailable

**Quantization Detection**:
Parses filename patterns: `model-name-Q4_K_M.gguf` → `Q4_K_M`

### vram_estimator.h/.cpp
**Purpose**: Calculate memory requirements

**Formulas**:
```cpp
// Model weights on GPU
GPU_Memory = Total_Size × (GPU_Layers / Total_Layers)

// KV Cache
KV_Cache = 2 × Layers × Context × Embedding_Dim × Bytes_Per_Element × 1.2

// Input embeddings
Input_Memory = Vocab_Size × Embedding_Dim × 2  // FP16

// Output buffer
Output_Memory = Batch_Size × Embedding_Dim × 4 × 4  // FP32 + intermediate

// Total VRAM
Total = GPU_Memory + KV_Cache + Input_Memory + Output_Memory + Overhead (512MB)
```

**Optimizations Calculated**:
- KV cache quantization (2 bytes → 1 byte per element)
- GPU layer offloading (partial model loading)
- Context length impact

### gpu_detector.h/.cpp
**Purpose**: Detect available GPU hardware

**Detection Methods**:
1. **NVIDIA**: Parse `nvidia-smi --query-gpu=name,memory.total`
2. **AMD**: Parse `rocm-smi --showmeminfo` (Linux)
3. **Apple**: Metal framework APIs (macOS)
4. **Fallback**: Vulkan device enumeration

**GPU Info Extracted**:
- Device name
- Total VRAM
- Available VRAM
- Vendor identification

### config_manager.h/.cpp
**Purpose**: Persist user settings

**Configuration Storage**:
- Linux/macOS: `~/.config/llama_manager/config.json`
- Windows: `%USERPROFILE%\.config\llama_manager\config.json`

**Saved Settings**:
- Inference parameters (context, GPU layers, batch size)
- Advanced options (Flash Attention, cache quantization)
- Last model path
- Remember settings preference

### file_browser.h/.cpp
**Purpose**: Directory navigation and GGUF file selection

**Features**:
- Recursive directory scanning for `.gguf` files
- Cross-platform path handling
- Integration with native file dialogs (future enhancement)

### main.cpp
**Purpose**: Application entry point and UI rendering

**UI Structure**:
1. **Setup Dialog** (if llama.cpp not found):
   - Error explanation
   - Installation instructions
   - Path specification
   - Retry detection

2. **Main Window**:
   - Memory estimate banner (GPU/Total usage)
   - Model selection browser
   - Model info display (architecture, quantization, layers)
   - Core settings (context, GPU offload, threads, batch)
   - Advanced settings (RoPE, Flash Attention, MoE)
   - Configuration persistence controls

3. **Warning System**:
   - Red progress bar when VRAM exceeds available
   - Experimental feature badges
   - Tooltip explanations

## UI Design Principles

### Matching LM Studio/Uigent Layout
- **Top**: Memory usage estimate with visual indicators
- **Middle**: Model selection and parameters
- **Bottom**: Advanced settings (collapsible)
- **Footer**: GPU info, llama.cpp version, actions

### Color Coding
- 🟢 Green: Normal operation (<75% VRAM)
- 🟠 Orange: Warning (75-90% VRAM)
- 🔴 Red: Critical (>90% VRAM or exceeding capacity)
- 🟡 Yellow: Experimental features

### Performance
- Immediate mode GUI (no retained state)
- Real-time VRAM recalculation on parameter change
- Minimal redraws (vsync-limited)

## Build System

### CMake Configuration
- Minimum: CMake 3.15
- Standard: C++17
- Dependencies: SDL2, OpenGL, Dear ImGui (included)

### Portable Binary Strategy
- Static link where possible
- SDL2 dynamic (platform requirement)
- Single executable output
- No runtime dependencies beyond system libraries

### Cross-Platform Considerations
```cmake
# Platform-specific linking
if(UNIX AND NOT APPLE)
    # Linux
    target_link_libraries(... dl pthread)
elseif(APPLE)
    # macOS
    target_link_libraries(... "-framework Cocoa" "-framework Metal")
elseif(WIN32)
    # Windows
    # SDL2 DLL copied automatically
endif()
```

## Future Enhancements

### Phase 1 (Core Functionality)
- [x] llama.cpp integration
- [x] GGUF detection and parsing
- [x] VRAM estimation
- [x] GPU detection
- [x] Basic UI

### Phase 2 (Polish)
- [ ] Native file dialog integration
- [ ] Full GGUF metadata parsing (without llama.cpp)
- [ ] Model performance benchmarking
- [ ] Export configuration to llama.cpp command line
- [ ] Dark/light theme toggle

### Phase 3 (Advanced)
- [ ] Multiple GPU support
- [ ] Remote model loading (HTTP/HuggingFace)
- [ ] Configuration presets (gaming, development, research)
- [ ] Memory profiling mode (actual vs estimated)
- [ ] Integration with ollama, lmstudio, etc.

## Performance Benchmarks (Target)

| Metric | Target | Notes |
|--------|--------|-------|
| Startup time | <1 second | Cold start |
| VRAM usage | <50 MB | Base application |
| Model scan | <5 seconds | 1000 GGUF files |
| Config load | <100 ms | JSON parsing |
| GPU detection | <500 ms | All platforms |

## Code Quality

### Standards
- C++17 features used appropriately
- RAII for resource management
- Clear separation of concerns
- Platform abstraction layers

### Error Handling
- Graceful fallbacks when llama.cpp unavailable
- User-friendly error messages
- No silent failures

### Documentation
- Inline comments for complex logic
- README with examples
- BUILD.md with platform specifics
- Code comments explain "why", not "what"

## Testing Strategy

### Manual Testing
- [ ] llama.cpp detection on all platforms
- [ ] GPU detection (NVIDIA, AMD, Apple)
- [ ] GGUF file loading (various quantizations)
- [ ] VRAM estimation accuracy
- [ ] Configuration persistence
- [ ] UI responsiveness

### Validation
- Compare VRAM estimates with actual llama.cpp usage
- Verify GPU detection against system tools
- Cross-reference GGUF parsing with llama.cpp output

## Deployment

### Distribution Methods
1. **Source**: Clone and build (developers)
2. **Binary**: Precompiled releases (users)
3. **Package**: AppImage (Linux), DMG (macOS), Installer (Windows)

### Installation Size
- Binary: ~2-5 MB (without SDL2)
- With dependencies: ~10-15 MB
- Total install: <20 MB

## Comparison with Alternatives

| Feature | This Utility | LM Studio | Ollama | llama.cpp CLI |
|---------|-------------|-----------|--------|---------------|
| VRAM Usage | <50 MB | 200-500 MB | 50-100 MB | 0 MB |
| Startup | <1s | 5-10s | 2-3s | Instant |
| GUI | Yes | Yes | Web | No |
| Model Load | No | Yes | Yes | Yes |
| VRAM Est | Yes | Yes | No | Manual |
| Config | Yes | Yes | Limited | CLI args |
| Cross-platform | Yes | Yes | Yes | Yes |

**Unique Selling Point**: Lightweight configuration tool that doesn't load models, just estimates and prepares settings for llama.cpp.

## License

MIT License - permissive, commercial-friendly

## Conclusion

This utility fills a gap between heavy GUI applications (LM Studio) and bare CLI tools (llama.cpp). It provides:

1. **Efficiency**: 5-10× less memory than Electron alternatives
2. **Integration**: Works with existing llama.cpp installations
3. **Clarity**: Clear VRAM estimates before loading
4. **Simplicity**: Single-purpose tool, does one thing well

Perfect for users who:
- Want to test configurations before loading
- Need to optimize VRAM usage for their hardware
- Prefer lightweight tools over feature-heavy IDEs
- Use llama.cpp CLI but want easier configuration

---

**Development Time**: ~4-6 hours for initial implementation
**Lines of Code**: ~3000-4000 (estimated)
**Dependencies**: Minimal (SDL2, OpenGL, ImGui)
**Maintenance**: Low - stable dependencies, focused scope
