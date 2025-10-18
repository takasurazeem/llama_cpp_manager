#include "gguf_reader.h"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <regex>

namespace llama_manager {

GGUFReader::GGUFReader() {
    model_info_.is_valid = false;
}

GGUFReader::~GGUFReader() {
}

std::string GGUFReader::QuantizationToString(GGUFQuantizationType quant) {
    switch (quant) {
        case GGUFQuantizationType::F32: return "F32";
        case GGUFQuantizationType::F16: return "F16";
        case GGUFQuantizationType::Q4_0: return "Q4_0";
        case GGUFQuantizationType::Q4_1: return "Q4_1";
        case GGUFQuantizationType::Q5_0: return "Q5_0";
        case GGUFQuantizationType::Q5_1: return "Q5_1";
        case GGUFQuantizationType::Q8_0: return "Q8_0";
        case GGUFQuantizationType::Q8_1: return "Q8_1";
        case GGUFQuantizationType::Q2_K: return "Q2_K";
        case GGUFQuantizationType::Q3_K: return "Q3_K";
        case GGUFQuantizationType::Q4_K: return "Q4_K";
        case GGUFQuantizationType::Q5_K: return "Q5_K";
        case GGUFQuantizationType::Q6_K: return "Q6_K";
        case GGUFQuantizationType::Q8_K: return "Q8_K";
        case GGUFQuantizationType::IQ2_XXS: return "IQ2_XXS";
        case GGUFQuantizationType::IQ2_XS: return "IQ2_XS";
        case GGUFQuantizationType::IQ3_XXS: return "IQ3_XXS";
        case GGUFQuantizationType::IQ1_S: return "IQ1_S";
        case GGUFQuantizationType::IQ4_NL: return "IQ4_NL";
        case GGUFQuantizationType::IQ3_S: return "IQ3_S";
        case GGUFQuantizationType::IQ2_S: return "IQ2_S";
        case GGUFQuantizationType::IQ4_XS: return "IQ4_XS";
        default: return "Unknown";
    }
}

float GGUFReader::GetQuantizationBitsPerWeight(GGUFQuantizationType quant) {
    switch (quant) {
        case GGUFQuantizationType::F32: return 32.0f;
        case GGUFQuantizationType::F16: return 16.0f;
        case GGUFQuantizationType::Q8_0:
        case GGUFQuantizationType::Q8_1:
        case GGUFQuantizationType::Q8_K: return 8.5f;
        case GGUFQuantizationType::Q6_K: return 6.5f;
        case GGUFQuantizationType::Q5_0:
        case GGUFQuantizationType::Q5_1:
        case GGUFQuantizationType::Q5_K: return 5.5f;
        case GGUFQuantizationType::Q4_0:
        case GGUFQuantizationType::Q4_1:
        case GGUFQuantizationType::Q4_K:
        case GGUFQuantizationType::IQ4_NL:
        case GGUFQuantizationType::IQ4_XS: return 4.5f;
        case GGUFQuantizationType::Q3_K:
        case GGUFQuantizationType::IQ3_XXS:
        case GGUFQuantizationType::IQ3_S: return 3.5f;
        case GGUFQuantizationType::Q2_K:
        case GGUFQuantizationType::IQ2_XXS:
        case GGUFQuantizationType::IQ2_XS:
        case GGUFQuantizationType::IQ2_S: return 2.5f;
        case GGUFQuantizationType::IQ1_S: return 1.5f;
        default: return 4.5f; // Default to Q4
    }
}

GGUFQuantizationType GGUFReader::DetectQuantization(const std::string& filename) {
    std::string lower = filename;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    // Extract quantization from filename patterns
    if (lower.find("f32") != std::string::npos) return GGUFQuantizationType::F32;
    if (lower.find("f16") != std::string::npos) return GGUFQuantizationType::F16;
    if (lower.find("q8_0") != std::string::npos) return GGUFQuantizationType::Q8_0;
    if (lower.find("q8_1") != std::string::npos) return GGUFQuantizationType::Q8_1;
    if (lower.find("q8_k") != std::string::npos) return GGUFQuantizationType::Q8_K;
    if (lower.find("q6_k") != std::string::npos) return GGUFQuantizationType::Q6_K;
    if (lower.find("q5_0") != std::string::npos) return GGUFQuantizationType::Q5_0;
    if (lower.find("q5_1") != std::string::npos) return GGUFQuantizationType::Q5_1;
    if (lower.find("q5_k") != std::string::npos) return GGUFQuantizationType::Q5_K;
    if (lower.find("q4_0") != std::string::npos) return GGUFQuantizationType::Q4_0;
    if (lower.find("q4_1") != std::string::npos) return GGUFQuantizationType::Q4_1;
    if (lower.find("q4_k") != std::string::npos) return GGUFQuantizationType::Q4_K;
    if (lower.find("iq4_nl") != std::string::npos) return GGUFQuantizationType::IQ4_NL;
    if (lower.find("iq4_xs") != std::string::npos) return GGUFQuantizationType::IQ4_XS;
    if (lower.find("q3_k") != std::string::npos) return GGUFQuantizationType::Q3_K;
    if (lower.find("iq3_xxs") != std::string::npos) return GGUFQuantizationType::IQ3_XXS;
    if (lower.find("iq3_s") != std::string::npos) return GGUFQuantizationType::IQ3_S;
    if (lower.find("q2_k") != std::string::npos) return GGUFQuantizationType::Q2_K;
    if (lower.find("iq2_xxs") != std::string::npos) return GGUFQuantizationType::IQ2_XXS;
    if (lower.find("iq2_xs") != std::string::npos) return GGUFQuantizationType::IQ2_XS;
    if (lower.find("iq2_s") != std::string::npos) return GGUFQuantizationType::IQ2_S;
    if (lower.find("iq1_s") != std::string::npos) return GGUFQuantizationType::IQ1_S;
    
    return GGUFQuantizationType::UNKNOWN;
}

bool GGUFReader::ReadFile(const std::string& filepath) {
    model_info_ = GGUFModelInfo();
    model_info_.is_valid = false;
    
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        model_info_.error_message = "Failed to open file: " + filepath;
        return false;
    }
    
    // Read GGUF magic number (first 4 bytes should be "GGUF")
    char magic[5] = {0};
    file.read(magic, 4);
    
    if (std::string(magic) != "GGUF") {
        model_info_.error_message = "Not a valid GGUF file (magic number mismatch)";
        return false;
    }
    
    // Get file size
    file.seekg(0, std::ios::end);
    model_info_.total_size_bytes = file.tellg();
    model_info_.size_gb = model_info_.total_size_bytes / (1024.0f * 1024.0f * 1024.0f);
    file.seekg(0, std::ios::beg);
    
    // Extract basic info from filename
    model_info_.filename = filepath.substr(filepath.find_last_of("/\\") + 1);
    model_info_.quantization = DetectQuantization(model_info_.filename);
    
    // Set some defaults (these should ideally be read from GGUF metadata)
    // Use a high default to support extended context models
    // Users can adjust down if needed
    model_info_.context_length = 262144; // Max supported, user can adjust
    model_info_.layer_count = 48; // Reasonable default
    model_info_.embedding_length = 4096;
    model_info_.parameter_count = 7000000000; // 7B default estimate
    model_info_.architecture = "llama"; // Generic default
    model_info_.is_valid = true;
    
    return true;
}

} // namespace llama_manager
