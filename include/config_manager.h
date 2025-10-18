#pragma once

#include "vram_estimator.h"
#include <string>
#include <vector>
#include <map>

namespace llama_manager {

class ConfigManager {
public:
    ConfigManager();
    ~ConfigManager();
    
    // Load/save configuration
    bool LoadConfig(const std::string& filepath);
    bool SaveConfig(const std::string& filepath) const;
    
    // Get/set configuration
    InferenceConfig& GetConfig() { return config_; }
    const InferenceConfig& GetConfig() const { return config_; }
    
    void SetConfig(const InferenceConfig& config) { config_ = config; }
    
    // Model path management
    void SetLastModelPath(const std::string& path) { last_model_path_ = path; }
    const std::string& GetLastModelPath() const { return last_model_path_; }
    
    // Recent models management (up to max_recent_models)
    void AddRecentModel(const std::string& path);
    const std::vector<std::string>& GetRecentModels() const { return recent_models_; }
    void ClearRecentModels() { recent_models_.clear(); }
    
    // Per-model settings (stored in separate files)
    void SaveModelSettings(const std::string& model_path, const InferenceConfig& config);
    bool LoadModelSettings(const std::string& model_path, InferenceConfig& config);
    bool HasModelSettings(const std::string& model_path) const;
    void ClearModelSettings(const std::string& model_path);
    
    // Remember settings flag
    void SetRememberSettings(bool remember) { remember_settings_ = remember; }
    bool GetRememberSettings() const { return remember_settings_; }
    
private:
    InferenceConfig config_;
    std::string last_model_path_;
    std::vector<std::string> recent_models_;
    bool remember_settings_ = true;
    
    static constexpr size_t max_recent_models_ = 10;
    
    std::string GetDefaultConfigPath() const;
    std::string GetModelConfigPath(const std::string& model_path) const;
    std::string GenerateModelHash(const std::string& model_path) const;
};

} // namespace llama_manager
