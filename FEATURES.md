# New Features in v1.1.0

This document describes the new features added in version 1.1.0 of LLAMA.CPP Manager.

## 🗂️ Functional File Browser

The file browser is now fully functional, providing an intuitive way to browse and select GGUF model files.

### Features:
- **Directory Navigation**: Browse through your filesystem to find model files
- **Parent Directory Access**: Navigate up to parent directories with ".." option
- **Visual Distinction**: Directories are clearly marked with `[DIR]` prefix
- **GGUF Filtering**: Only shows `.gguf` files and directories for easier navigation
- **Automatic Loading**: Selected models are automatically loaded with full metadata
- **Recent Models List**: Quick access to previously selected models

### Usage:
1. Click **"Browse GGUF Model..."** button
2. Navigate through directories by clicking on `[DIR]` entries
3. Use **".. (Parent Directory)"** to go up one level
4. Click on a `.gguf` file to select it
5. Or select from **"Recently Selected Models"** section
6. Model info loads automatically, including:
   - Architecture
   - Quantization type
   - Parameter count
   - Layer count
   - File size

### Example Workflow:
```
1. Click "Browse GGUF Model..."
2. Navigate: /home → user → models
3. Click: llama-2-7b-Q4_K_M.gguf
4. Model loads with all metadata
5. VRAM estimates update automatically
6. Model added to recent list for quick access
```

---

## 📤 Export to llama.cpp CLI Command

Generate ready-to-use `llama-cli` commands with all your configured parameters.

### Features:
- **Complete Command Generation**: Includes all configured settings
- **Copy to Clipboard**: One-click copy for easy pasting
- **All Parameters Supported**:
  - Model path
  - Context length (`-c`)
  - GPU layers (`-ngl`)
  - CPU threads (`-t`)
  - Batch size (`-b`)
  - RoPE settings (`--rope-freq-base`, `--rope-freq-scale`)
  - Memory options (`--no-kv-offload`, `--no-mmap`)
  - Advanced features (Flash Attention, cache quantization)
  - MoE settings for Mixtral models
  - Seed value

### Usage:
1. Configure your model settings in the UI
2. Click **"Export CLI Command"** button
3. Review the generated command in the popup
4. Click **"Copy to Clipboard"**
5. Paste into your terminal

### Example Output:
```bash
llama-cli -m "/path/to/model.gguf" -c 4096 -ngl 32 -t 8 -b 512 \
  --rope-freq-base 10000.0 --flash-attn \
  --cache-type-k q8_0 -p "Your prompt here"
```

### Benefits:
- **Workflow Integration**: Seamlessly transition from configuration to execution
- **No Manual Typing**: Eliminates errors from manually typing CLI arguments
- **Documentation**: Serves as a reference for your configuration
- **Reproducibility**: Save commands to recreate exact settings later

---

## 📚 Recent Models & Per-Model Settings

Keep track of your frequently used models and maintain separate configurations for each.

### Features:
- **Recent Models List**: Displays up to 10 most recently selected models
- **Persistent History**: List maintained across application sessions
- **Per-Model Configuration**: Each model remembers its own settings
- **Quick Access**: Click any recent model to load it instantly
- **Smart Highlighting**: Currently loaded model is highlighted
- **File Validation**: Shows which models still exist on disk
- **Hybrid Storage**: Efficient configuration organization

### How It Works:

**Selecting a Model:**
1. Browse for a model or select from recent list
2. Model loads with metadata
3. If "Remember settings" was enabled, restores saved configuration
4. If first time loading, applies intelligent defaults
5. Model added to top of recent list

**Saving Per-Model Settings:**
1. Configure parameters for your model
2. Enable "Remember settings" checkbox
3. Click "Save Configuration"
4. Settings saved to `~/.config/llama_manager/models/<hash>.json`
5. Next time you select this model, settings are restored

**Recent Models Display:**
- Located in "Recently Selected Models" section
- Shows up to 10 models (most recent first)
- Filename displayed for quick recognition
- Full path shown in tooltip on hover
- Currently loaded model highlighted in green
- Missing files shown in gray with warning

### Configuration Architecture:

**Global Config** (`config.json`):
```json
{
  "context_length": 2048,
  "gpu_layers": 0,
  ...
  "recent_models": [
    "/path/to/model1.gguf",
    "/path/to/model2.gguf"
  ]
}
```

**Per-Model Config** (`models/<hash>.json`):
```json
{
  "model_path": "/path/to/model.gguf",
  "context_length": 4096,
  "gpu_layers": 35,
  "batch_size": 512,
  ...
}
```

### Benefits:

✅ **Workflow Efficiency**: No need to reconfigure each time  
✅ **Model-Specific Tuning**: Different settings for different models  
✅ **Experimentation**: Try different configs without losing originals  
✅ **Quick Switching**: Jump between models with one click  
✅ **Isolation**: One corrupted config doesn't affect others  
✅ **Scalability**: Handles many models without performance impact  

### Example Use Cases:

**Multiple Model Sizes:**
- 7B model: Context 2048, GPU layers 32
- 13B model: Context 4096, GPU layers 40
- 30B model: Context 8192, GPU layers 48
- Each remembers its optimal settings

**Different Quantizations:**
- Q4_K_M variant: More GPU layers
- Q8_0 variant: Fewer GPU layers (uses more VRAM per layer)
- Settings saved independently

**Task-Specific Configs:**
- Chat model: Context 2048, standard settings
- Document analysis: Context 16384, max context
- Code generation: Context 4096, moderate settings

See [MODEL_SETTINGS.md](MODEL_SETTINGS.md) for detailed technical information.

---

## 🎨 Native File Dialog Support (Optional)

Professional cross-platform file dialogs using ImGuiFileDialog.

### Features:
- **Native Look & Feel**: Platform-appropriate file dialogs
- **Advanced Features**:
  - File search/filtering
  - Thumbnails (for supported formats)
  - Favorites/bookmarks
  - Multiple file selection (future)
  - Quick navigation

### Setup:
```bash
# Run the setup script
./setup_native_dialogs.sh

# Rebuild the project
./build.sh
```

### When to Use:
- **Basic Browser** (current): Works out of the box, no dependencies
- **Native Dialogs** (optional): Better UX, more features, requires ImGuiFileDialog

Both options work great! Choose based on your preferences:
- **Keep it simple**: Use the built-in browser
- **Professional UX**: Add native dialogs with one command

---

## 🔄 Workflow Example

Here's how the new features work together:

### 1. Select Model
```
Click "Browse GGUF Model..." 
  → Navigate to model location
  → Select llama-2-7b-Q4_K_M.gguf
  → Model info loads automatically
```

### 2. Configure Settings
```
Adjust sliders:
  - Context Length: 4096
  - GPU Offload: 32 layers
  - CPU Threads: 8
  - Batch Size: 512

Enable options:
  ✓ Offload KV Cache
  ✓ Flash Attention
```

### 3. Export Command
```
Click "Export CLI Command"
  → Review generated command
  → Click "Copy to Clipboard"
  → Paste into terminal:

llama-cli -m "model.gguf" -c 4096 -ngl 32 -t 8 -b 512 \
  --flash-attn -p "Your prompt here"
```

### 4. Execute
```bash
# Paste and modify the prompt
llama-cli -m "llama-2-7b-Q4_K_M.gguf" -c 4096 -ngl 32 -t 8 \
  -b 512 --flash-attn -p "Explain quantum computing"
```

---

## 📊 Comparison: Before vs After

| Feature | v1.0.0 | v1.1.0 |
|---------|--------|--------|
| **File Selection** | Stub/non-functional | ✅ Full browser with navigation |
| **Model Loading** | Manual path entry | ✅ Automatic from browser |
| **CLI Export** | ❌ Not available | ✅ Full command generation |
| **Clipboard** | ❌ Manual copying | ✅ One-click copy |
| **Native Dialogs** | ❌ Not available | ✅ Optional support |
| **Workflow** | Disjointed | ✅ Seamless integration |

---

## 🚀 Getting Started

### Update to v1.1.0
```bash
# Pull latest changes
git pull origin main

# Rebuild
./build.sh

# Run
./run.sh
```

### Try the New Features
1. **File Browser**: Click "Browse GGUF Model..." and explore
2. **CLI Export**: Configure a model, then click "Export CLI Command"
3. **Native Dialogs** (optional): Run `./setup_native_dialogs.sh`

---

## 💡 Tips & Tricks

### File Browser
- **Quick navigation**: Remember common paths for faster browsing
- **Parent access**: Use ".." to quickly navigate up
- **Error handling**: Gracefully handles permission errors

### CLI Export
- **Save commands**: Copy and save to a text file for later
- **Batch processing**: Generate multiple commands for different configs
- **Documentation**: Use as reference for parameter meanings

### Workflow
1. Start with example configs from `examples/`
2. Load model via file browser
3. Adjust settings as needed
4. Export CLI command
5. Run inference with llama.cpp

---

## 🐛 Known Issues & Limitations

### File Browser
- No favorites/bookmarks yet (use native dialogs for this)
- Single file selection only (multi-select planned)
- Basic UI (native dialogs provide richer experience)

### CLI Export
- Prompt is placeholder (you must replace "Your prompt here")
- No direct execution (copy and paste to terminal)
- Windows paths may need adjustment

### General
- Native dialogs require manual setup
- First-time navigation may be slow on network drives

---

## 🔮 Future Enhancements

Planned for future releases:
- **Quick Favorites**: Bookmark frequently used directories
- **Recent Models**: List of recently opened models
- **Direct Execution**: Run llama-cli directly from the UI
- **Command History**: Save and reuse previous commands
- **Batch Operations**: Configure multiple models at once
- **Profile Management**: Save named configuration profiles

---

## 📞 Feedback & Support

Found a bug? Have a feature request?
- **GitHub Issues**: https://github.com/yourusername/llama_cpp_manager/issues
- **Documentation**: See README.md, TROUBLESHOOTING.md
- **Examples**: Check `examples/README.md`

---

**Happy configuring!** 🎉