# Changelog

All notable changes to LLAMA.CPP Manager will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2025-10-18

### Added
- **Functional file browser** - Implemented working ImGui-based file browser with directory navigation
  - Browse directories and select GGUF files
  - Navigate up to parent directories
  - Clear visual distinction between files and folders
  - Automatic model loading when file is selected
- **Export to llama.cpp CLI command** - Generate ready-to-use command line with all configured parameters
  - Includes all inference settings (context, GPU layers, threads, batch size)
  - Advanced options (RoPE, Flash Attention, cache quantization)
  - MoE model settings
  - Copy to clipboard functionality
- **Native file dialog support** (optional) - Added setup script for ImGuiFileDialog integration
  - Professional cross-platform file dialogs
  - Optional enhancement for better UX
  - Simple setup with `./setup_native_dialogs.sh`
- **Recent models list** - Maintains history of recently selected models
  - Up to 10 most recently used models tracked
  - Persists across application sessions
  - Quick access to frequently used models
- **Per-model settings persistence** - Individual configuration saved for each model
  - Hybrid configuration storage (global + per-model)
  - Settings automatically restored when selecting a model from recent list
  - Stored in separate files: `~/.config/llama_manager/models/<hash>.json`
  - "Remember settings" checkbox controls per-model saving
- **Build and run helper script** - Added `build_and_run.sh` for rapid development
  - Automatically builds project
  - Kills existing instances
  - Launches updated application
  - Convenient for development workflow

### Fixed
- File browser now properly displays and allows model selection
- Model info automatically loaded when file is selected through browser
- C++17 compatibility maintained (no C++20 dependencies)
- Recent models list properly loads and saves (fixed JSON array parsing)
- Configuration persistence now works correctly across sessions

### Improved
- Better model loading workflow
- Enhanced user experience with functional file selection
- Workflow integration with CLI export feature

## [1.0.0] - 2025-10-17

### Added
- Initial release of LLAMA.CPP Manager
- Complete Dear ImGui-based GUI for GGUF model configuration
- Real-time VRAM and RAM estimation
- GPU detection (NVIDIA, AMD, Apple Metal)
- llama.cpp integration for model metadata parsing
- Configuration save/load system (JSON format)
- File browser for GGUF model selection
- Helper scripts for setup, build, and environment checking
  - `setup.sh` - Automated first-time setup
  - `build.sh` - Quick incremental builds
  - `clean_build.sh` - Fresh rebuild from scratch
  - `run.sh` - Smart build-and-run
  - `check_env.sh` - Environment validation
- 8 example configuration presets for different hardware profiles
- Comprehensive documentation (10+ markdown files)
- Makefile with convenient targets
- Cross-platform support (Linux, macOS, Windows)

### Features
- Model parameter configuration
  - Context length (512-262144 tokens)
  - GPU layer offload
  - CPU thread pool size
  - Evaluation batch size
  - RoPE frequency base/scale
  - Seed control
- Advanced settings
  - Flash Attention (experimental)
  - KV cache quantization
  - MoE model support (num_experts)
  - Expert CPU offloading
- Memory estimation
  - Model weights calculation
  - KV cache estimation
  - GPU VRAM usage with color-coded warnings
  - Total RAM usage display
- GPU detection
  - Automatic GPU detection (NVIDIA/AMD/Apple)
  - VRAM capacity reporting
  - Fallback to CPU-only mode

### Technical Details
- C++17 codebase
- Dependencies: SDL2, OpenGL 3.0+, Dear ImGui
- Build system: CMake 3.15+
- External integration: llama.cpp tools (llama-cli, llama-gguf-dump)
- Architecture: 7 modular components
  - `llama_cpp_interface` - llama.cpp detection and CLI wrapper
  - `gguf_reader` - GGUF file format parser
  - `vram_estimator` - Memory usage calculator
  - `gpu_detector` - GPU detection and VRAM query
  - `config_manager` - JSON configuration I/O
  - `file_browser` - GGUF file selection
  - `main` - Application entry and UI

### Fixed
- C++17 compatibility in `file_browser.cpp` (replaced C++20 `ends_with()`)
- CMakeLists.txt configuration and structure
- Missing `main.cpp` implementation

### Documentation
- `README.md` - Project overview and quick start
- `QUICKSTART.md` - Fast setup guide
- `BUILD.md` - Detailed build instructions
- `DESIGN.md` - Architecture and design decisions
- `STRUCTURE.md` - Code organization
- `SUMMARY.md` - Technical summary
- `TROUBLESHOOTING.md` - Common issues and solutions
- `INDEX.md` - Documentation index
- `PACKAGE.md` - Complete package overview
- `WHATS_NEW.md` - Feature additions log
- `CHANGELOG.md` - This file
- `examples/README.md` - Configuration examples guide

### Known Limitations
- No actual model loading/inference (use llama.cpp for that)
- Single GPU support only
- Basic file browser (no native dialogs yet)
- Manual model path selection (no auto-discovery)

## [Unreleased]

### Planned Features
- Native file dialogs (Windows Explorer, macOS Finder, Linux dialogs)
- Multi-GPU detection and configuration
- Automatic model discovery in common directories
- Model download progress tracking
- Recent models list
- Configuration profiles management
- Dark/light theme toggle
- Command-line interface for scripting
- Model comparison view (side-by-side configs)
- Export to llama.cpp CLI command
- VRAM usage graphs over time
- Model format conversion suggestions

### Potential Improvements
- Better error messages with recovery suggestions
- Tooltips for all parameters
- Quick presets (Low/Medium/High/Ultra)
- Model performance predictions (tokens/sec)
- Integration with Ollama
- Integration with LocalAI
- Web UI variant (optional)
- Batch configuration for multiple models

---

## Version History

- **1.0.0** (2025-10-17) - Initial release
  - Full featured GGUF model configuration tool
  - Production-ready with comprehensive documentation
  - Successfully tested on Linux (Arch) with RTX 3090

---

## Links

- **Repository**: https://github.com/yourusername/llama_cpp_manager
- **Issues**: https://github.com/yourusername/llama_cpp_manager/issues
- **llama.cpp**: https://github.com/ggerganov/llama.cpp
- **Dear ImGui**: https://github.com/ocornut/imgui

## How to Report Issues

When reporting bugs, please include:
1. Operating system and version
2. GPU model and driver version
3. Compiler and version used to build
4. Output from `./check_env.sh`
5. Steps to reproduce the issue
6. Expected vs actual behavior
7. Any error messages or logs

## Contributing

See the main README.md for contribution guidelines.
