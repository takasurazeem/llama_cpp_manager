#include "gpu_detector.h"
#include <cstring>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <Metal/Metal.h>
#else
#include <cstdlib>
#endif

namespace llama_manager {

GPUDetector::GPUDetector() {
}

GPUDetector::~GPUDetector() {
}

bool GPUDetector::DetectGPUs() {
    gpus_.clear();
    
    // Try different detection methods
    bool found = false;
    
    found |= DetectNVIDIA();
    found |= DetectAMD();
    found |= DetectMetal();
    found |= DetectVulkan();
    
    return found;
}

const GPUInfo& GPUDetector::GetPrimaryGPU() const {
    static GPUInfo dummy;
    if (gpus_.empty()) {
        return dummy;
    }
    return gpus_[0];
}

bool GPUDetector::DetectNVIDIA() {
#ifdef _WIN32
    // Windows NVIDIA detection via nvidia-smi
    FILE* pipe = _popen("nvidia-smi --query-gpu=name,memory.total --format=csv,noheader,nounits", "r");
#else
    // Linux/macOS NVIDIA detection via nvidia-smi
    FILE* pipe = popen("nvidia-smi --query-gpu=name,memory.total --format=csv,noheader,nounits 2>/dev/null", "r");
#endif
    
    if (!pipe) {
        return false;
    }
    
    char buffer[256];
    bool found = false;
    
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line(buffer);
        size_t comma_pos = line.find(',');
        
        if (comma_pos != std::string::npos) {
            GPUInfo gpu;
            gpu.name = line.substr(0, comma_pos);
            
            // Trim whitespace
            gpu.name.erase(0, gpu.name.find_first_not_of(" \t\n\r"));
            gpu.name.erase(gpu.name.find_last_not_of(" \t\n\r") + 1);
            
            std::string mem_str = line.substr(comma_pos + 1);
            try {
                float mem_mb = std::stof(mem_str);
                gpu.total_vram_bytes = static_cast<uint64_t>(mem_mb * 1024 * 1024);
                gpu.available_vram_bytes = gpu.total_vram_bytes; // Simplified
                gpu.vendor = GPUVendor::NVIDIA;
                gpu.is_available = true;
                
                gpus_.push_back(gpu);
                found = true;
            } catch (...) {
                // Parsing failed
            }
        }
    }
    
#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    
    return found;
}

bool GPUDetector::DetectAMD() {
#ifdef _WIN32
    // Windows AMD detection
    // TODO: Implement Windows AMD detection
    return false;
#elif __linux__
    // Linux AMD detection via rocm-smi
    FILE* pipe = popen("rocm-smi --showmeminfo vram --csv 2>/dev/null", "r");
    if (!pipe) {
        return false;
    }
    
    char buffer[256];
    bool found = false;
    
    // Skip header
    fgets(buffer, sizeof(buffer), pipe);
    
    while (fgets(buffer, sizeof(buffer), pipe)) {
        GPUInfo gpu;
        gpu.name = "AMD GPU";
        gpu.vendor = GPUVendor::AMD;
        gpu.is_available = true;
        
        // Parse VRAM info (simplified)
        std::string line(buffer);
        // TODO: Parse actual VRAM size
        gpu.total_vram_bytes = 8ULL * 1024 * 1024 * 1024; // Default to 8GB
        gpu.available_vram_bytes = gpu.total_vram_bytes;
        
        gpus_.push_back(gpu);
        found = true;
    }
    
    pclose(pipe);
    return found;
#else
    return false;
#endif
}

bool GPUDetector::DetectMetal() {
#ifdef __APPLE__
    // macOS Metal detection
    @autoreleasepool {
        NSArray<id<MTLDevice>>* devices = MTLCopyAllDevices();
        
        for (id<MTLDevice> device in devices) {
            GPUInfo gpu;
            gpu.name = std::string([device.name UTF8String]);
            gpu.vendor = GPUVendor::APPLE;
            gpu.is_available = true;
            
            // Get recommended working set size (available memory)
            gpu.total_vram_bytes = [device recommendedMaxWorkingSetSize];
            gpu.available_vram_bytes = gpu.total_vram_bytes;
            
            gpus_.push_back(gpu);
        }
        
        return !gpus_.empty();
    }
#else
    return false;
#endif
}

bool GPUDetector::DetectVulkan() {
    // Vulkan detection as fallback
    // This is simplified - would need vulkan SDK
    
#ifdef __linux__
    // Check if vulkan is available via vulkaninfo
    FILE* pipe = popen("vulkaninfo --summary 2>/dev/null | grep -i 'deviceName\\|memory'", "r");
    if (!pipe) {
        return false;
    }
    
    char buffer[256];
    bool found = false;
    std::string device_name;
    
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line(buffer);
        
        if (line.find("deviceName") != std::string::npos) {
            size_t eq_pos = line.find('=');
            if (eq_pos != std::string::npos) {
                device_name = line.substr(eq_pos + 1);
                device_name.erase(0, device_name.find_first_not_of(" \t\n\r"));
                device_name.erase(device_name.find_last_not_of(" \t\n\r") + 1);
            }
        }
        
        if (line.find("heapSize") != std::string::npos && !device_name.empty()) {
            GPUInfo gpu;
            gpu.name = device_name;
            gpu.vendor = GPUVendor::UNKNOWN;
            gpu.is_available = true;
            gpu.total_vram_bytes = 4ULL * 1024 * 1024 * 1024; // Default to 4GB
            gpu.available_vram_bytes = gpu.total_vram_bytes;
            
            gpus_.push_back(gpu);
            found = true;
            break;
        }
    }
    
    pclose(pipe);
    return found;
#endif
    
    return false;
}

} // namespace llama_manager
