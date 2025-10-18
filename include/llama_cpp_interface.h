#pragma once

#include "gguf_reader.h"
#include <string>
#include <optional>

namespace llama_manager {

struct LlamaCppInstallation {
    std::string install_path;
    std::string llama_cli_path;
    std::string gguf_info_path;
    std::string version;
    bool is_valid;
    std::string error_message;
};

class LlamaCppInterface {
public:
    LlamaCppInterface();
    ~LlamaCppInterface();
    
    // Detect llama.cpp installation
    bool DetectInstallation();
    
    // Check if llama.cpp is available
    bool IsAvailable() const { return installation_.is_valid; }
    
    // Get installation info
    const LlamaCppInstallation& GetInstallation() const { return installation_; }
    
    // Set custom installation path
    bool SetCustomPath(const std::string& path);
    
    // Get model information using llama.cpp tools
    bool GetModelInfo(const std::string& model_path, GGUFModelInfo& info);
    
    // Get detailed model metadata using gguf-dump or similar tools
    std::string GetModelMetadata(const std::string& model_path);
    
    // Estimate VRAM usage using llama-cli --help or similar
    std::optional<uint64_t> EstimateModelVRAM(const std::string& model_path, 
                                                uint32_t context_length,
                                                uint32_t gpu_layers);
    
private:
    LlamaCppInstallation installation_;
    
    // Detection helpers
    bool CheckPath(const std::string& path);
    bool FindInSystemPath();
    bool FindInCommonLocations();
    std::string GetLlamaCppVersion(const std::string& llama_cli_path);
    
    // Execution helpers
    bool ExecuteCommand(const std::string& command, std::string& output, int& exit_code);
    std::string FindExecutable(const std::string& name);
    
    // Common installation paths by platform
    std::vector<std::string> GetCommonInstallPaths() const;
};

} // namespace llama_manager
