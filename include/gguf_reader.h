#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

namespace llama_manager {

enum class GGUFQuantizationType {
    F32 = 0,
    F16 = 1,
    Q4_0 = 2,
    Q4_1 = 3,
    Q5_0 = 6,
    Q5_1 = 7,
    Q8_0 = 8,
    Q8_1 = 9,
    Q2_K = 10,
    Q3_K = 11,
    Q4_K = 12,
    Q5_K = 13,
    Q6_K = 14,
    Q8_K = 15,
    IQ2_XXS = 16,
    IQ2_XS = 17,
    IQ3_XXS = 18,
    IQ1_S = 19,
    IQ4_NL = 20,
    IQ3_S = 21,
    IQ2_S = 22,
    IQ4_XS = 23,
    UNKNOWN = 999
};

struct GGUFModelInfo {
    std::string filename;
    std::string architecture;
    GGUFQuantizationType quantization;
    
    uint64_t parameter_count;
    uint32_t context_length;
    uint32_t embedding_length;
    uint32_t head_count;
    uint32_t head_count_kv;
    uint32_t layer_count;
    uint32_t vocab_size;
    
    // Calculated values
    uint64_t total_size_bytes;
    float size_gb;
    
    bool is_valid;
    std::string error_message;
};

class GGUFReader {
public:
    GGUFReader();
    ~GGUFReader();
    
    // Read GGUF file header and extract metadata
    bool ReadFile(const std::string& filepath);
    
    // Get parsed model information
    const GGUFModelInfo& GetModelInfo() const { return model_info_; }
    
    // Utility functions
    static std::string QuantizationToString(GGUFQuantizationType quant);
    static float GetQuantizationBitsPerWeight(GGUFQuantizationType quant);
    static GGUFQuantizationType DetectQuantization(const std::string& filename);
    
private:
    GGUFModelInfo model_info_;
    
    bool ParseHeader(const uint8_t* data, size_t size);
    bool ReadMetadata(const uint8_t* data, size_t size, size_t& offset);
};

} // namespace llama_manager
