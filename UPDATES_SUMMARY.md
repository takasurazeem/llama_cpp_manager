# Recent Updates Summary

## Overview
This document summarizes the recent bug fixes and improvements made to the llama_cpp_manager application.

## Bug Fixed: Recent Models List Not Persisting

### Problem
The application was not maintaining a list of recently selected models between sessions. While it remembered the last selected model path, it did not:
- Show the recent models list
- Remember model-specific settings
- Provide quick access to previously used models

### Root Cause
**Configuration Mismatch**: The `SaveConfig()` method was writing JSON arrays (`"recent_models": [...]`), but the `LoadConfig()` method was using a simple line-by-line parser that couldn't understand JSON array syntax. The parser was looking for individual `"recent_model"` key-value pairs that were never written.

### Solution Implemented

#### 1. Fixed JSON Array Parsing
Updated `ConfigManager::LoadConfig()` to properly parse JSON arrays:
- Detects when entering the `recent_models` array
- Extracts model paths from array elements
- Handles quotes and commas correctly
- Exits array parsing at closing bracket

#### 2. Hybrid Configuration System
Implemented a better architecture for storing configurations:

**Global Config** (`~/.config/llama_manager/config.json`):
- Global default settings
- Recent models list (up to 10)
- Last model path
- Remember settings preference

**Per-Model Configs** (`~/.config/llama_manager/models/<hash>.json`):
- Model-specific inference settings
- Individual file per model
- Isolated from other configs
- Only created when "Remember settings" is enabled

#### 3. Removed Auto-Loading
Changed startup behavior:
- **Before**: Last model was auto-loaded on startup
- **After**: No auto-loading; user selects from recent list
- **Benefit**: Better control, proper settings restoration

#### 4. Smart Settings Restoration
When selecting a model:
1. Check if per-model config exists
2. If yes: Restore saved settings
3. If no: Apply intelligent defaults based on system resources

## New Features

### 1. Recent Models List
- Displays up to 10 most recently used models
- Persists across application sessions
- Shows filename with full path in tooltip
- Highlights currently loaded model in green
- Grays out missing files with warning
- Click any model to load it instantly

### 2. Per-Model Settings Persistence
- Each model can have its own configuration
- Settings automatically saved when "Remember settings" is enabled
- Settings automatically restored when selecting from recent list
- Isolated storage prevents corruption from affecting other models

### 3. Build and Run Script
Created `build_and_run.sh` for development workflow:
```bash
#!/bin/bash
# Builds project, kills old instances, launches new one
./build_and_run.sh
```

## Technical Changes

### Files Modified

**`include/config_manager.h`:**
- Added per-model settings methods
- Added hash generation for model paths
- Added model config path resolution

**`src/config_manager.cpp`:**
- Fixed `LoadConfig()` to parse JSON arrays
- Implemented `SaveModelSettings()`
- Implemented `LoadModelSettings()`
- Implemented `HasModelSettings()`
- Implemented `ClearModelSettings()`
- Implemented `GenerateModelHash()`
- Implemented `GetModelConfigPath()`

**`src/main.cpp`:**
- Removed auto-loading on startup
- Updated model selection to restore saved settings
- Updated all save operations to save per-model settings
- Enhanced recent models UI display

**`config.json`:**
- Added `recent_models` array field

### Configuration File Structure

```
~/.config/llama_manager/
├── config.json                 # Global settings + recent list
└── models/
    ├── abc123def456.json       # Model 1 settings
    ├── 789ghi012jkl.json       # Model 2 settings
    └── ...
```

## User-Visible Changes

### Before
❌ No recent models list  
❌ Settings lost between sessions  
❌ Model auto-loaded on startup (unexpected)  
❌ No per-model configuration  
❌ Had to browse for model every time  

### After
✅ Recent models list with up to 10 models  
✅ Settings persist per model  
✅ Explicit model selection (better UX)  
✅ Per-model configurations isolated  
✅ Quick access to frequently used models  
✅ Smart settings restoration  

## Usage Example

### First Time Using a Model
1. Click "Browse GGUF Model..."
2. Select `llama-2-7b-Q4_K_M.gguf`
3. Intelligent defaults applied (context: 2048, GPU layers: 32)
4. Adjust settings to your preference (context: 4096, GPU layers: 35)
5. Enable "Remember settings"
6. Click "Save Configuration"
7. Model added to recent list

### Next Time
1. Launch application
2. See model in "Recently Selected Models"
3. Click on model name
4. Settings restored: Context: 4096, GPU layers: 35 ✓
5. Ready to use immediately

### Multiple Models
- Each model maintains its own settings
- Switch between models with one click
- No reconfiguration needed
- Settings never interfere with each other

## Testing Performed

✅ Recent models list persists across restarts  
✅ Per-model settings save and restore correctly  
✅ Global settings still work  
✅ Missing files shown with warning  
✅ Currently loaded model highlighted  
✅ Up to 10 models tracked (oldest removed)  
✅ Settings restore on recent list selection  
✅ Intelligent defaults for new models  
✅ Build and run script works correctly  

## Documentation Updated

- ✅ `README.md` - Added recent models and per-model settings
- ✅ `CHANGELOG.md` - Added v1.1.0 updates
- ✅ `FEATURES.md` - Added recent models section
- ✅ `BUILD.md` - Added build_and_run.sh documentation
- ✅ `MODEL_SETTINGS.md` - Created detailed technical doc
- ✅ `UPDATES_SUMMARY.md` - This file

## Benefits

### For Users
1. **Faster Workflow**: No need to browse for models repeatedly
2. **Consistent Experience**: Settings preserved per model
3. **Better Organization**: Clear separation of configs
4. **Peace of Mind**: Corruption-proof isolated configs
5. **Flexibility**: Try different settings without losing originals

### For Developers
1. **Clean Architecture**: Separation of concerns
2. **Scalability**: Handles many models efficiently
3. **Maintainability**: Isolated configs easier to debug
4. **Extensibility**: Easy to add more per-model features

## Future Enhancements

Potential improvements for future releases:
- Model favorites/bookmarks
- Search/filter in recent models list
- Export/import model configurations
- Configuration profiles (Low/Medium/High VRAM)
- Drag-to-reorder recent models
- Multi-select for batch operations
- Configuration diff/compare view

## Conclusion

The recent models and per-model settings features significantly improve the user experience by:
- Eliminating repetitive configuration tasks
- Providing quick access to frequently used models
- Maintaining model-specific optimizations
- Creating a more professional, polished application

The hybrid configuration architecture provides a solid foundation for future enhancements while keeping the codebase clean and maintainable.
