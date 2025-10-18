#include "vram_estimator.h"
#include "gguf_reader.h"

namespace llama_manager {

VRAMEstimator::VRAMEstimator() {
}

VRAMEstimator::~VRAMEstimator() {
}

VRAMEstimate VRAMEstimator::Calculate(const GGUFModelInfo& model, const InferenceConfig& config) const {
    VRAMEstimate estimate;
    
    // Calculate model weights on GPU
    estimate.model_weights = CalculateModelWeights(model, config.gpu_layers);
    
    // Calculate KV cache
    estimate.kv_cache = CalculateKVCache(model, config);
    
    // Calculate input embeddings
    estimate.input_embeddings = CalculateInputEmbeddings(model);
    
    // Calculate output buffer
    estimate.output_buffer = CalculateOutputBuffer(model, config.batch_size);
    
    // Calculate overhead
    estimate.overhead = CalculateOverhead();
    
    // Calculate total VRAM (GPU memory)
    uint64_t vram_used = 0;
    
    // Model weights on GPU
    if (config.gpu_layers > 0) {
        vram_used += estimate.model_weights;
    }
    
    // KV cache
    if (config.offload_kv_cache && config.gpu_layers > 0) {
        vram_used += estimate.kv_cache;
    }
    
    // Input/output buffers (always on GPU if using GPU)
    if (config.gpu_layers > 0) {
        vram_used += estimate.input_embeddings;
        vram_used += estimate.output_buffer;
        vram_used += estimate.overhead;
    }
    
    estimate.total_vram = vram_used;
    
    // Calculate total RAM (CPU memory)
    uint64_t ram_used = 0;
    
    // Model weights on CPU
    if (config.gpu_layers < model.layer_count) {
        float cpu_layers_ratio = 1.0f - (static_cast<float>(config.gpu_layers) / model.layer_count);
        ram_used += static_cast<uint64_t>(model.total_size_bytes * cpu_layers_ratio);
    }
    
    // KV cache on CPU
    if (!config.offload_kv_cache || config.gpu_layers == 0) {
        ram_used += estimate.kv_cache;
    }
    
    // Add base overhead for llama.cpp
    ram_used += 512 * 1024 * 1024; // 512 MB base overhead
    
    estimate.total_ram = ram_used;
    
    return estimate;
}

uint64_t VRAMEstimator::CalculateModelWeights(const GGUFModelInfo& model, uint32_t gpu_layers) const {
    if (gpu_layers == 0 || model.layer_count == 0) {
        return 0;
    }
    
    // Calculate proportion of model on GPU
    float gpu_ratio = static_cast<float>(gpu_layers) / model.layer_count;
    if (gpu_ratio > 1.0f) gpu_ratio = 1.0f;
    
    // Model size in bytes, scaled by GPU layers
    uint64_t gpu_weights = static_cast<uint64_t>(model.total_size_bytes * gpu_ratio);
    
    return gpu_weights;
}

uint64_t VRAMEstimator::CalculateKVCache(const GGUFModelInfo& model, const InferenceConfig& config) const {
    // KV cache formula: 2 * n_layers * context_length * embedding_dim * bytes_per_element
    // For quantized cache, bytes_per_element might be less than FP16
    
    float bytes_per_element = 2.0f; // FP16
    
    if (config.quantize_k_cache || config.quantize_v_cache) {
        bytes_per_element = 1.0f; // Q8 or similar
    }
    
    uint64_t kv_size = 2 * // K and V
                       model.layer_count *
                       config.context_length *
                       model.embedding_length *
                       static_cast<uint64_t>(bytes_per_element);
    
    // Add overhead factor
    kv_size = static_cast<uint64_t>(kv_size * KV_CACHE_OVERHEAD_FACTOR);
    
    return kv_size;
}

uint64_t VRAMEstimator::CalculateInputEmbeddings(const GGUFModelInfo& model) const {
    // Input embeddings: vocab_size * embedding_dim * bytes_per_element
    uint64_t embedding_size = model.vocab_size * model.embedding_length * 2; // FP16
    return embedding_size;
}

uint64_t VRAMEstimator::CalculateOutputBuffer(const GGUFModelInfo& model, uint32_t batch_size) const {
    // Output buffer for computation: batch_size * embedding_dim * bytes_per_element
    uint64_t buffer_size = batch_size * model.embedding_length * 4; // FP32 for computation
    
    // Multiply by a factor for intermediate activations
    buffer_size *= 4;
    
    return buffer_size;
}

uint64_t VRAMEstimator::CalculateOverhead() const {
    // CUDA/driver overhead
    return CUDA_OVERHEAD_MB * 1024 * 1024;
}

InferenceConfig VRAMEstimator::CalculateOptimalConfig(const GGUFModelInfo& model, uint64_t available_vram_bytes, uint64_t available_ram_bytes) const {
    InferenceConfig config;
    
    // Start with a reasonable context length
    config.context_length = CalculateOptimalContextLength(model, available_vram_bytes, available_ram_bytes);
    
    // Calculate optimal GPU layers for this context length
    config.gpu_layers = CalculateOptimalGPULayers(model, available_vram_bytes, config.context_length);
    
    // Set reasonable defaults for other parameters
    config.batch_size = 512;
    config.threads = 8; // Will be adjusted based on CPU cores in main
    config.offload_kv_cache = (config.gpu_layers > 0);
    config.use_mmap = true;
    config.keep_model_in_memory = true;
    config.rope_freq_base = 0.0f; // auto
    config.rope_freq_scale = 0.0f; // auto
    config.seed = -1; // random
    
    // Disable experimental features by default
    config.flash_attention = false;
    config.quantize_k_cache = false;
    config.quantize_v_cache = false;
    config.num_experts = 0;
    config.force_experts_cpu = false;
    
    return config;
}

uint32_t VRAMEstimator::CalculateOptimalGPULayers(const GGUFModelInfo& model, uint64_t available_vram_bytes, uint32_t context_length) const {
    if (model.layer_count == 0 || available_vram_bytes == 0) {
        return 0;
    }
    
    // Apply safety margin to available VRAM
    uint64_t usable_vram = static_cast<uint64_t>(available_vram_bytes * VRAM_SAFETY_MARGIN);
    
    // Calculate fixed costs (overhead, embeddings, output buffer)
    uint64_t fixed_cost = CalculateOverhead() + CalculateInputEmbeddings(model) + CalculateOutputBuffer(model, 512);
    
    // Calculate KV cache size for given context
    InferenceConfig temp_config;
    temp_config.context_length = context_length;
    temp_config.offload_kv_cache = true;
    uint64_t kv_cache_size = CalculateKVCache(model, temp_config);
    
    // Subtract fixed costs from available VRAM
    if (usable_vram <= fixed_cost + kv_cache_size) {
        return 0; // Not enough VRAM even for overhead
    }
    
    uint64_t remaining_vram = usable_vram - fixed_cost - kv_cache_size;
    
    // Calculate bytes per layer
    uint64_t bytes_per_layer = model.total_size_bytes / model.layer_count;
    
    // Calculate how many layers fit
    uint32_t max_layers = static_cast<uint32_t>(remaining_vram / bytes_per_layer);
    
    // Cap at total layer count
    if (max_layers > model.layer_count) {
        max_layers = model.layer_count;
    }
    
    return max_layers;
}

uint32_t VRAMEstimator::CalculateOptimalContextLength(const GGUFModelInfo& model, uint64_t available_vram_bytes, uint64_t available_ram_bytes) const {
    // Define context length tiers to test
    const uint32_t context_tiers[] = {2048, 4096, 8192, 16384, 32768, 65536, 131072};
    const size_t num_tiers = sizeof(context_tiers) / sizeof(context_tiers[0]);
    
    // Start with model's max context as upper bound
    uint32_t max_context = model.context_length;
    if (max_context == 0) max_context = 2048; // Default if unknown
    
    // Find the largest context that fits
    uint32_t optimal_context = 2048; // Minimum reasonable default
    
    for (size_t i = 0; i < num_tiers; ++i) {
        uint32_t test_context = context_tiers[i];
        
        // Don't exceed model's capability
        if (test_context > max_context) {
            break;
        }
        
        // Calculate GPU layers we could use with this context
        uint32_t gpu_layers = CalculateOptimalGPULayers(model, available_vram_bytes, test_context);
        
        // Create a test config
        InferenceConfig test_config;
        test_config.context_length = test_context;
        test_config.gpu_layers = gpu_layers;
        test_config.offload_kv_cache = (gpu_layers > 0);
        test_config.batch_size = 512;
        
        // Calculate total memory requirements
        VRAMEstimate estimate = Calculate(model, test_config);
        
        // Check if it fits in available memory (with safety margins)
        uint64_t usable_vram = static_cast<uint64_t>(available_vram_bytes * VRAM_SAFETY_MARGIN);
        uint64_t usable_ram = static_cast<uint64_t>(available_ram_bytes * RAM_SAFETY_MARGIN);
        
        if (estimate.total_vram <= usable_vram && estimate.total_ram <= usable_ram) {
            optimal_context = test_context;
        } else {
            // This tier doesn't fit, so previous tier was optimal
            break;
        }
    }
    
    return optimal_context;
}

} // namespace llama_manager
