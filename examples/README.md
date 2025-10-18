# Example Configurations

This directory contains pre-configured settings for common use cases. You can load these configurations directly or use them as templates.

## Available Configurations

### 1. **config_default.json**
- **Use case**: Balanced settings for most users
- **VRAM required**: 8-10 GB
- **Context**: 2048 tokens
- **GPU layers**: 32
- **Recommended models**: 7B Q4_K_M

### 2. **config_low_vram.json**
- **Use case**: Limited VRAM (4-6 GB GPUs)
- **VRAM required**: 4-6 GB
- **Context**: 2048 tokens
- **GPU layers**: 20
- **Recommended models**: 7B Q4_K_M or Q4_0
- **Features**: KV cache quantization enabled

### 3. **config_medium_vram.json**
- **Use case**: Standard gaming GPUs (8-12 GB)
- **VRAM required**: 8-12 GB
- **Context**: 4096 tokens
- **GPU layers**: 35
- **Recommended models**: 13B Q4_K_M or 7B Q8_0

### 4. **config_high_vram.json**
- **Use case**: High-end GPUs (16+ GB)
- **VRAM required**: 16+ GB
- **Context**: 8192 tokens
- **GPU layers**: 48 (full offload)
- **Recommended models**: 30B Q4_K_M or 13B Q8_0
- **Features**: Flash Attention enabled

### 5. **config_extended_context.json**
- **Use case**: Long document processing
- **VRAM required**: 12+ GB
- **Context**: 16384 tokens
- **GPU layers**: 40
- **Recommended models**: 7B-13B Q4 with RoPE support
- **Features**: RoPE scaling configured

### 6. **config_cpu_only.json**
- **Use case**: No GPU available
- **VRAM required**: 0 GB (CPU only)
- **Context**: 2048 tokens
- **GPU layers**: 0
- **Threads**: 16 (high CPU usage)
- **Recommended models**: Any quantized model

### 7. **config_mixtral.json**
- **Use case**: Mixture-of-Experts models
- **VRAM required**: 12+ GB
- **Context**: 4096 tokens
- **GPU layers**: 33
- **Recommended models**: Mixtral 8x7B Q4_K_M
- **Features**: MoE settings configured (8 experts)

### 8. **config_max_quality.json**
- **Use case**: Maximum output quality
- **VRAM required**: 16+ GB
- **Context**: 4096 tokens
- **GPU layers**: 48 (full offload)
- **Recommended models**: F16 or Q8_0
- **Features**: Flash Attention, no cache quantization

## How to Use

### Method 1: Manual Loading
1. Open LLAMA.CPP Manager
2. Load a model
3. Manually copy values from the example config

### Method 2: Copy to Config Directory
```bash
# Linux/macOS
cp examples/config_low_vram.json ~/.config/llama_manager/config.json

# Windows
copy examples\config_low_vram.json %USERPROFILE%\.config\llama_manager\config.json
```

### Method 3: Command Line (Future Feature)
```bash
./llama_cpp_manager --config examples/config_high_vram.json
```

## Configuration Parameters Explained

### Core Settings

| Parameter | Description | Typical Range |
|-----------|-------------|---------------|
| `context_length` | Maximum tokens in context window | 512 - 32768 |
| `gpu_layers` | Number of model layers on GPU | 0 - 80 |
| `batch_size` | Tokens processed in parallel | 128 - 2048 |
| `threads` | CPU threads for processing | 4 - 32 |

### RoPE Settings (Advanced)

| Parameter | Description | Values |
|-----------|-------------|--------|
| `rope_freq_base` | Base frequency for position encoding | 0 (auto) or 10000+ |
| `rope_freq_scale` | Scaling factor for extended context | 0 (auto) or 0.5 - 2.0 |

### Memory Optimization

| Parameter | Description | Impact |
|-----------|-------------|--------|
| `offload_kv_cache` | Store attention cache on GPU | High VRAM savings |
| `use_mmap` | Memory-map model file | Faster loading |
| `keep_model_in_memory` | Don't unload between uses | Faster subsequent loads |

### Experimental Features

| Parameter | Description | Compatibility |
|-----------|-------------|---------------|
| `flash_attention` | Faster attention mechanism | Modern GPUs, some models |
| `quantize_k_cache` | Quantize key cache | All models (slight quality loss) |
| `quantize_v_cache` | Quantize value cache | All models (slight quality loss) |
| `force_experts_cpu` | CPU-only expert weights | MoE models only |

## Creating Custom Configurations

1. Start with the closest example config
2. Copy and rename: `cp config_medium_vram.json config_custom.json`
3. Edit values based on your hardware
4. Test VRAM estimate in the application
5. Adjust until the usage bar is in the green zone

## VRAM Estimation Guide

Use this table to choose an appropriate configuration:

| Your GPU VRAM | Recommended Config | Notes |
|---------------|-------------------|-------|
| 4 GB | `config_low_vram.json` | Small models only (7B Q4) |
| 6 GB | `config_low_vram.json` | Small-medium models |
| 8 GB | `config_medium_vram.json` | Most 7B-13B models |
| 12 GB | `config_medium_vram.json` or `config_high_vram.json` | 13B-30B models |
| 16 GB | `config_high_vram.json` | 30B+ models, extended context |
| 24 GB+ | `config_max_quality.json` | Any model, highest quality |

## Tips

💡 **Start conservative**: Begin with a config for less VRAM than you have, then increase

💡 **Watch the color bar**: 
- 🟢 Green (<75%) = Safe
- 🟠 Orange (75-90%) = Careful
- 🔴 Red (>90%) = Will likely OOM

💡 **Context is expensive**: Doubling context length roughly doubles KV cache memory

💡 **Quantization matters**:
- Q4_K_M: Best balance (recommended)
- Q5_K_M: Higher quality, more VRAM
- Q8_0: Near-original quality, 2× size
- IQ2_XXS: Extreme compression, quality loss

💡 **GPU layers**: 
- More layers = faster, more VRAM
- Offload in increments of 5-10 and test
- Last few layers have highest impact

## Troubleshooting

### "Out of Memory" Errors
1. Reduce `context_length` (biggest impact)
2. Reduce `gpu_layers` by 10-20%
3. Enable `quantize_k_cache` and `quantize_v_cache`
4. Reduce `batch_size` to 256 or 128
5. Use a more quantized model variant

### Slow Performance
1. Increase `gpu_layers` if VRAM allows
2. Increase `batch_size` (if VRAM allows)
3. Ensure `offload_kv_cache` is enabled
4. Enable `flash_attention` if supported

### Model Won't Load
1. Check VRAM estimate is below available
2. Close other GPU applications
3. Try `config_low_vram.json` first
4. Consider CPU-only mode

## Contributing

Have a good configuration preset? Submit it with:
- Use case description
- Hardware requirements
- Tested model types
- Performance notes
