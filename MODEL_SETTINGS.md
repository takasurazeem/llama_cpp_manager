# Model Settings Management

## Overview

The application now properly maintains a list of recently selected models along with per-model configuration settings.

## Architecture Decision: Hybrid Configuration Storage

We use a **hybrid approach** with separate configuration files:

### Main Config File: `~/.config/llama_manager/config.json`
Stores:
- Global application settings
- Default inference configuration
- List of recently selected models (up to 10)
- Last selected model path
- Remember settings preference

### Per-Model Config Files: `~/.config/llama_manager/models/<hash>.json`
Stores:
- Model-specific inference settings
- Individual configuration for each model path
- Automatically created when "Remember settings" is enabled

## Why This Approach?

✅ **Advantages:**
1. **Isolation**: Each model's settings are in separate files - corruption doesn't affect others
2. **Scalability**: Can handle many models without bloating main config
3. **Clean separation**: Global settings vs. per-model settings
4. **Easy backup**: Can backup/share individual model configs
5. **Performance**: Don't need to parse all model settings to load recent list

❌ **Alternative Single File Approach Would Have:**
- Large, complex JSON structure
- Risk of corruption affecting everything
- Slower parsing with many models
- Harder to manage individual model settings

## How It Works

### 1. Recent Models List
- Maintained in `config.json` as an array of model paths
- Most recently selected appears first
- Maximum of 10 models tracked
- Automatically updated when browsing for models

### 2. Per-Model Settings
When you select a model and click "Save Configuration" with "Remember settings" enabled:
- Settings are saved to `~/.config/llama_manager/models/<hash>.json`
- Hash is generated from the full model path for uniqueness
- Next time you select this model, settings are automatically restored

### 3. Behavior Changes

**Before:**
- ❌ Model was auto-loaded on startup
- ❌ Settings were always reset to intelligent defaults
- ❌ No way to preserve model-specific configurations

**After:**
- ✅ No auto-loading - user explicitly selects from recent list
- ✅ Saved settings are restored when model is selected
- ✅ Intelligent defaults only applied for new models
- ✅ Each model remembers its own configuration

## Usage

1. **Browse for a model**: Click "Browse GGUF Model..."
2. **Configure settings**: Adjust parameters as needed
3. **Save configuration**: Enable "Remember settings" and click "Save Configuration"
4. **Next time**: Model appears in recent list with saved settings icon

## File Structure

```
~/.config/llama_manager/
├── config.json                 # Global settings + recent models list
└── models/
    ├── abc123def456.json       # Settings for model 1
    ├── 789ghi012jkl.json       # Settings for model 2
    └── ...                     # More model configs
```

## Example Workflow

1. Load `llama-2-7b.gguf` → Intelligent defaults applied
2. Adjust context to 4096, GPU layers to 35
3. Click "Save Configuration" with "Remember settings" checked
4. Close application
5. Reopen application
6. Select `llama-2-7b.gguf` from recent list → Context: 4096, GPU: 35 restored ✓
7. Load different model `mixtral-8x7b.gguf` → New intelligent defaults
8. Original model settings preserved independently

## Implementation Details

- **Hash function**: Simple std::hash on full model path
- **Config format**: Simple JSON key-value pairs
- **Thread safety**: Single-threaded GUI application, no locking needed
- **Error handling**: Gracefully falls back to defaults if config file corrupted
