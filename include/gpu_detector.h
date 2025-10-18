#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace llama_manager {

enum class GPUVendor {
    NVIDIA,
    AMD,
    INTEL,
    APPLE,
    UNKNOWN
};

struct GPUInfo {
    std::string name;
    uint64_t total_vram_bytes;
    uint64_t available_vram_bytes;
    GPUVendor vendor;
    bool is_available;
    
    float GetTotalVRAM_GB() const { return total_vram_bytes / (1024.0f * 1024.0f * 1024.0f); }
    float GetAvailableVRAM_GB() const { return available_vram_bytes / (1024.0f * 1024.0f * 1024.0f); }
};

class GPUDetector {
public:
    GPUDetector();
    ~GPUDetector();
    
    // Detect available GPUs
    bool DetectGPUs();
    
    // Get detected GPU information
    const std::vector<GPUInfo>& GetGPUs() const { return gpus_; }
    
    // Get primary GPU (first available)
    const GPUInfo& GetPrimaryGPU() const;
    
    // Check if any GPU is available
    bool HasGPU() const { return !gpus_.empty(); }
    
private:
    std::vector<GPUInfo> gpus_;
    
    // Platform-specific detection
    bool DetectNVIDIA();
    bool DetectAMD();
    bool DetectMetal();
    bool DetectVulkan();
};

} // namespace llama_manager
