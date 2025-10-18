#pragma once

#include "gguf_reader.h"
#include <cstdint>

namespace llama_manager {

struct VRAMEstimate {
    // Memory breakdown in bytes
    uint64_t model_weights = 0;      // Model parameters
    uint64_t kv_cache = 0;            // KV cache for context
    uint64_t input_embeddings = 0;   // Input layer
    uint64_t output_buffer = 0;      // Output computation buffer
    uint64_t overhead = 0;            // CUDA/driver overhead
    
    // Totals
    uint64_t total_vram = 0;         // GPU memory
    uint64_t total_ram = 0;          // System RAM (for CPU layers)
    
    // Helper to get values in GB
    float GetTotalVRAM_GB() const { return total_vram / (1024.0f * 1024.0f * 1024.0f); }
    float GetTotalRAM_GB() const { return total_ram / (1024.0f * 1024.0f * 1024.0f); }
    float GetTotal_GB() const { return (total_vram + total_ram) / (1024.0f * 1024.0f * 1024.0f); }
};

struct InferenceConfig {
    // Core parameters
    uint32_t context_length = 2048;
    uint32_t gpu_layers = 0;
    uint32_t batch_size = 512;
    uint32_t threads = 8;
    
    // RoPE settings
    float rope_freq_base = 0.0f;    // 0 = auto
    float rope_freq_scale = 0.0f;   // 0 = auto
    
    // Memory optimization
    bool offload_kv_cache = true;
    bool use_mmap = true;
    bool keep_model_in_memory = true;
    
    // MoE settings
    uint32_t num_experts = 0;       // 0 = not MoE
    bool force_experts_cpu = false;
    
    // Experimental features
    bool flash_attention = false;
    bool quantize_k_cache = false;
    bool quantize_v_cache = false;
    
    // Seed
    int32_t seed = -1;  // -1 = random
};

class VRAMEstimator {
public:
    VRAMEstimator();
    ~VRAMEstimator();
    
    // Calculate memory requirements
    VRAMEstimate Calculate(const GGUFModelInfo& model, const InferenceConfig& config) const;
    
    // Individual component calculations
    uint64_t CalculateModelWeights(const GGUFModelInfo& model, uint32_t gpu_layers) const;
    uint64_t CalculateKVCache(const GGUFModelInfo& model, const InferenceConfig& config) const;
    uint64_t CalculateInputEmbeddings(const GGUFModelInfo& model) const;
    uint64_t CalculateOutputBuffer(const GGUFModelInfo& model, uint32_t batch_size) const;
    uint64_t CalculateOverhead() const;
    
    // Intelligent defaults calculation
    InferenceConfig CalculateOptimalConfig(const GGUFModelInfo& model, uint64_t available_vram_bytes, uint64_t available_ram_bytes) const;
    uint32_t CalculateOptimalGPULayers(const GGUFModelInfo& model, uint64_t available_vram_bytes, uint32_t context_length) const;
    uint32_t CalculateOptimalContextLength(const GGUFModelInfo& model, uint64_t available_vram_bytes, uint64_t available_ram_bytes) const;
    
private:
    // Constants for estimation
    static constexpr uint64_t CUDA_OVERHEAD_MB = 512;
    static constexpr float KV_CACHE_OVERHEAD_FACTOR = 1.2f;
    static constexpr float VRAM_SAFETY_MARGIN = 0.85f; // Use 85% of available VRAM
    static constexpr float RAM_SAFETY_MARGIN = 0.75f;  // Use 75% of available RAM
};

} // namespace llama_manager
