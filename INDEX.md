# 📚 Project Index

Quick reference to all files in the LLAMA.CPP Manager project.

## 🚀 Start Here

| File | Purpose | When to Use |
|------|---------|-------------|
| **QUICKSTART.md** | 3-command setup | First time, want speed |
| **README.md** | Full user guide | Learning how to use |
| **setup.sh** | Automated setup | First time setup |
| **Makefile** | Build commands | Daily development |

## 📖 Documentation Map

### For Users

| Document | Purpose | Size |
|----------|---------|------|
| **QUICKSTART.md** | Fast 3-step setup | 3.3K |
| **README.md** | Complete user manual | 9.2K |
| **TROUBLESHOOTING.md** | Problem solving | 9.4K |
| **examples/README.md** | Config guide | - |
| **PACKAGE.md** | Project overview | 9.9K |
| **WHATS_NEW.md** | Recent additions | 13K |
| **CHANGELOG.md** | Version history | 5.5K |

### For Developers

| Document | Purpose | Size |
|----------|---------|------|
| **BUILD.md** | Build instructions | 8.6K |
| **DESIGN.md** | Architecture & formulas | 11K |
| **STRUCTURE.md** | File organization | 7.0K |
| **SUMMARY.md** | Implementation summary | 7.9K |
| **CHANGELOG.md** | Version history | 5.5K |

## 🔧 Scripts & Tools

### Build Scripts

| Script | Purpose | Usage |
|--------|---------|-------|
| **setup.sh** | First-time setup | `./setup.sh` |
| **build.sh** | Quick build | `./build.sh` |
| **clean_build.sh** | Fresh rebuild | `./clean_build.sh` |
| **run.sh** | Build and run | `./run.sh` |
| **check_env.sh** | Check dependencies | `./check_env.sh` |

All scripts are executable (`chmod +x` already applied).

### Makefile Targets

```bash
make              # Build
make run          # Build and run
make clean        # Clean build directory
make rebuild      # Clean rebuild
make check        # Check environment
make install      # Install to system
make help         # Show all targets
```

## ⚙️ Configuration Files

### Build Configuration

| File | Purpose |
|------|---------|
| **CMakeLists.txt** | CMake build config |
| **vcpkg.json** | vcpkg dependencies |
| **Makefile** | Make targets |
| **.gitignore** | Git exclusions |

### Example Configs (in `examples/`)

| File | VRAM | Use Case |
|------|------|----------|
| **config_default.json** | 8-10 GB | Balanced |
| **config_low_vram.json** | 4-6 GB | Budget GPU |
| **config_medium_vram.json** | 8-12 GB | Gaming GPU |
| **config_high_vram.json** | 16+ GB | High-end GPU |
| **config_extended_context.json** | 12+ GB | Long documents |
| **config_cpu_only.json** | 0 GB | No GPU |
| **config_mixtral.json** | 12+ GB | MoE models |
| **config_max_quality.json** | 16+ GB | Best quality |

## 💻 Source Code

### Headers (in `include/`)

| File | Purpose | Lines |
|------|---------|-------|
| **llama_cpp_interface.h** | llama.cpp integration | ~70 |
| **gguf_reader.h** | GGUF file parsing | ~70 |
| **vram_estimator.h** | Memory calculations | ~80 |
| **gpu_detector.h** | Hardware detection | ~50 |
| **config_manager.h** | Settings I/O | ~40 |
| **file_browser.h** | File navigation | ~40 |

### Implementations (in `src/`)

| File | Purpose | Lines |
|------|---------|-------|
| **main.cpp** | UI & application | ~700 |
| **llama_cpp_interface.cpp** | Integration logic | ~500 |
| **gguf_reader.cpp** | File parsing | ~300 |
| **vram_estimator.cpp** | Calculations | ~300 |
| **gpu_detector.cpp** | Detection | ~400 |
| **config_manager.cpp** | Config I/O | ~200 |
| **file_browser.cpp** | Directory scan | ~150 |

## 📁 Directory Structure

```
llama_cpp_manager/
├── 📄 Documentation (14 files, ~75K)
│   ├── README.md
│   ├── QUICKSTART.md
│   ├── BUILD.md
│   ├── DESIGN.md
│   ├── STRUCTURE.md
│   ├── SUMMARY.md
│   ├── TROUBLESHOOTING.md
│   ├── PACKAGE.md
│   └── LICENSE
│
├── 🔧 Scripts (5 files, ~13K)
│   ├── setup.sh
│   ├── build.sh
│   ├── clean_build.sh
│   ├── run.sh
│   └── check_env.sh
│
├── ⚙️ Build Config (4 files)
│   ├── CMakeLists.txt
│   ├── Makefile
│   ├── vcpkg.json
│   └── .gitignore
│
├── 💻 Source Code (13 files, ~2600 lines)
│   ├── include/ (6 headers)
│   └── src/ (7 implementations)
│
├── 📂 Examples (9 files)
│   ├── README.md
│   └── 8 config JSON files
│
└── 📁 External (user downloads)
    └── extern/imgui/
```

## 🎯 Quick Navigation

### "I want to..."

**Build the project**
→ `./setup.sh` or `make setup`

**Run the application**
→ `./run.sh` or `make run`

**Understand the code**
→ Read `STRUCTURE.md` then `DESIGN.md`

**Fix a build error**
→ Check `TROUBLESHOOTING.md`

**Configure for my GPU**
→ See `examples/README.md`

**Learn the parameters**
→ Read `README.md` Configuration section

**Extend the code**
→ Study `DESIGN.md` and `include/` headers

**Install system-wide**
→ `make install` (Linux/macOS)

## 📊 File Statistics

| Category | Count | Total Size |
|----------|-------|------------|
| Documentation | 14 | ~75 KB |
| Scripts | 5 | ~13 KB |
| Config Files | 4 | ~5 KB |
| Example Configs | 8 | ~5 KB |
| Headers | 6 | ~10 KB |
| Implementations | 7 | ~60 KB |
| **Total** | **44** | **~170 KB** |

## 🔍 Finding Things

### By Topic

**llama.cpp integration:**
- `include/llama_cpp_interface.h`
- `src/llama_cpp_interface.cpp`
- `DESIGN.md` (integration section)

**VRAM estimation:**
- `include/vram_estimator.h`
- `src/vram_estimator.cpp`
- `DESIGN.md` (formulas section)
- `examples/README.md` (VRAM guide)

**GPU detection:**
- `include/gpu_detector.h`
- `src/gpu_detector.cpp`
- `TROUBLESHOOTING.md` (GPU section)

**UI and interface:**
- `src/main.cpp`
- `DESIGN.md` (UI section)

**Configuration:**
- `include/config_manager.h`
- `src/config_manager.cpp`
- `examples/*.json`

### By Task

**Setup:**
- Start: `QUICKSTART.md`
- Detailed: `BUILD.md`
- Automated: `setup.sh`

**Building:**
- Quick: `make` or `./build.sh`
- Clean: `make rebuild` or `./clean_build.sh`
- Check: `./check_env.sh`

**Running:**
- Direct: `./build/llama_cpp_manager`
- Smart: `./run.sh` (rebuilds if needed)
- Make: `make run`

**Troubleshooting:**
- Common issues: `TROUBLESHOOTING.md`
- Environment: `./check_env.sh`
- Build errors: `BUILD.md`

## 🎓 Learning Path

### Beginner Path (30 minutes)
1. Read `QUICKSTART.md` (5 min)
2. Run `./setup.sh` (5 min)
3. Try the application (10 min)
4. Read `examples/README.md` (10 min)

### Intermediate Path (2 hours)
1. Read `README.md` (30 min)
2. Try different configs from `examples/` (30 min)
3. Read `STRUCTURE.md` (20 min)
4. Browse source headers in `include/` (20 min)
5. Experiment with builds (20 min)

### Advanced Path (4+ hours)
1. Complete Intermediate Path
2. Read `DESIGN.md` thoroughly (1 hour)
3. Study `src/main.cpp` (1 hour)
4. Explore all implementations (1 hour)
5. Try modifying and rebuilding (1+ hours)

## 🔗 External Dependencies

| Dependency | Where | How to Get |
|------------|-------|------------|
| **Dear ImGui** | `extern/imgui/` | `git clone https://github.com/ocornut/imgui.git extern/imgui` |
| **SDL2** | System | `sudo apt install libsdl2-dev` or `brew install sdl2` |
| **llama.cpp** | System PATH | `git clone https://github.com/ggerganov/llama.cpp.git` |

All handled automatically by `setup.sh`.

## 📞 Help & Support

| Need | File/Command |
|------|--------------|
| Quick help | `make help` |
| Check setup | `./check_env.sh` |
| Build issues | `TROUBLESHOOTING.md` |
| Usage help | `README.md` |
| Architecture | `DESIGN.md` |
| Everything | `PACKAGE.md` |

## 🎯 Most Important Files

**For first-time users:**
1. `QUICKSTART.md` - Get started fast
2. `setup.sh` - Automated setup
3. `README.md` - Learn features

**For developers:**
1. `DESIGN.md` - Understand architecture
2. `STRUCTURE.md` - Navigate code
3. `include/*.h` - Study interfaces

**For troubleshooting:**
1. `TROUBLESHOOTING.md` - Solutions
2. `check_env.sh` - Diagnostics
3. `BUILD.md` - Build help

---

**🎉 Everything you need is here! Pick your starting point and dive in!**
