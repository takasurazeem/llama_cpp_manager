#include "config_manager.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <iostream>

// Simple JSON serialization (you could use nlohmann/json or similar for production)
namespace fs = std::filesystem;

namespace llama_manager {

ConfigManager::ConfigManager() {
    // Set defaults
    config_.context_length = 2048;
    config_.gpu_layers = 0;
    config_.batch_size = 512;
    config_.threads = 8;
    config_.offload_kv_cache = true;
    config_.use_mmap = true;
    config_.keep_model_in_memory = true;
    config_.flash_attention = false;
    config_.seed = -1;
}

ConfigManager::~ConfigManager() {
}

std::string ConfigManager::GetDefaultConfigPath() const {
    // Get config path in user's home directory
#ifdef _WIN32
    const char* home = getenv("USERPROFILE");
#else
    const char* home = getenv("HOME");
#endif
    
    if (!home) {
        return "llama_manager_config.json";
    }
    
    fs::path config_dir = fs::path(home) / ".config" / "llama_manager";
    
    // Create directory if it doesn't exist
    if (!fs::exists(config_dir)) {
        fs::create_directories(config_dir);
    }
    
    return (config_dir / "config.json").string();
}

bool ConfigManager::LoadConfig(const std::string& filepath) {
    std::string path = filepath.empty() ? GetDefaultConfigPath() : filepath;
    
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    // Clear recent models before loading
    recent_models_.clear();
    
    // Simple key-value parser with array support
    std::string line;
    bool in_recent_models_array = false;
    
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#' || line[0] == '/') {
            continue;
        }
        
        // Check if we're entering the recent_models array
        if (line.find("\"recent_models\"") != std::string::npos) {
            in_recent_models_array = true;
            continue;
        }
        
        // If we're in the array, parse model paths
        if (in_recent_models_array) {
            // Check for array end
            if (line.find(']') != std::string::npos) {
                in_recent_models_array = false;
                continue;
            }
            
            // Extract model path from array element: "    "/path/to/model.gguf","
            size_t first_quote = line.find('"');
            if (first_quote != std::string::npos) {
                size_t second_quote = line.find('"', first_quote + 1);
                if (second_quote != std::string::npos) {
                    std::string model_path = line.substr(first_quote + 1, second_quote - first_quote - 1);
                    if (!model_path.empty() && recent_models_.size() < max_recent_models_) {
                        recent_models_.push_back(model_path);
                    }
                }
            }
            continue;
        }
        
        size_t colon = line.find(':');
        if (colon == std::string::npos) {
            continue;
        }
        
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        
        // Trim whitespace and quotes
        auto trim = [](std::string& s) {
            s.erase(0, s.find_first_not_of(" \t\n\r\""));
            s.erase(s.find_last_not_of(" \t\n\r\",") + 1);
        };
        
        trim(key);
        trim(value);
        
        // Parse values
        try {
            if (key == "context_length") config_.context_length = std::stoul(value);
            else if (key == "gpu_layers") config_.gpu_layers = std::stoul(value);
            else if (key == "batch_size") config_.batch_size = std::stoul(value);
            else if (key == "threads") config_.threads = std::stoul(value);
            else if (key == "rope_freq_base") config_.rope_freq_base = std::stof(value);
            else if (key == "rope_freq_scale") config_.rope_freq_scale = std::stof(value);
            else if (key == "offload_kv_cache") config_.offload_kv_cache = (value == "true");
            else if (key == "use_mmap") config_.use_mmap = (value == "true");
            else if (key == "keep_model_in_memory") config_.keep_model_in_memory = (value == "true");
            else if (key == "num_experts") config_.num_experts = std::stoul(value);
            else if (key == "force_experts_cpu") config_.force_experts_cpu = (value == "true");
            else if (key == "flash_attention") config_.flash_attention = (value == "true");
            else if (key == "quantize_k_cache") config_.quantize_k_cache = (value == "true");
            else if (key == "quantize_v_cache") config_.quantize_v_cache = (value == "true");
            else if (key == "seed") config_.seed = std::stoi(value);
            else if (key == "last_model_path") last_model_path_ = value;
            else if (key == "remember_settings") remember_settings_ = (value == "true");
        } catch (...) {
            // Skip invalid values
        }
    }
    
    return true;
}

bool ConfigManager::SaveConfig(const std::string& filepath) const {
    std::string path = filepath.empty() ? GetDefaultConfigPath() : filepath;
    
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    // Write configuration as simple JSON
    file << "{\n";
    file << "  \"context_length\": " << config_.context_length << ",\n";
    file << "  \"gpu_layers\": " << config_.gpu_layers << ",\n";
    file << "  \"batch_size\": " << config_.batch_size << ",\n";
    file << "  \"threads\": " << config_.threads << ",\n";
    file << "  \"rope_freq_base\": " << config_.rope_freq_base << ",\n";
    file << "  \"rope_freq_scale\": " << config_.rope_freq_scale << ",\n";
    file << "  \"offload_kv_cache\": " << (config_.offload_kv_cache ? "true" : "false") << ",\n";
    file << "  \"use_mmap\": " << (config_.use_mmap ? "true" : "false") << ",\n";
    file << "  \"keep_model_in_memory\": " << (config_.keep_model_in_memory ? "true" : "false") << ",\n";
    file << "  \"num_experts\": " << config_.num_experts << ",\n";
    file << "  \"force_experts_cpu\": " << (config_.force_experts_cpu ? "true" : "false") << ",\n";
    file << "  \"flash_attention\": " << (config_.flash_attention ? "true" : "false") << ",\n";
    file << "  \"quantize_k_cache\": " << (config_.quantize_k_cache ? "true" : "false") << ",\n";
    file << "  \"quantize_v_cache\": " << (config_.quantize_v_cache ? "true" : "false") << ",\n";
    file << "  \"seed\": " << config_.seed << ",\n";
    file << "  \"last_model_path\": \"" << last_model_path_ << "\",\n";
    file << "  \"remember_settings\": " << (remember_settings_ ? "true" : "false") << ",\n";
    file << "  \"recent_models\": [\n";
    
    // Write recent models array
    for (size_t i = 0; i < recent_models_.size(); ++i) {
        file << "    \"" << recent_models_[i] << "\"";
        if (i < recent_models_.size() - 1) {
            file << ",";
        }
        file << "\n";
    }
    
    file << "  ]\n";
    file << "}\n";
    
    return true;
}

void ConfigManager::AddRecentModel(const std::string& path) {
    if (path.empty()) return;
    
    // Remove if already exists (move to front)
    auto it = std::find(recent_models_.begin(), recent_models_.end(), path);
    if (it != recent_models_.end()) {
        recent_models_.erase(it);
    }
    
    // Add to front
    recent_models_.insert(recent_models_.begin(), path);
    
    // Keep only max_recent_models_
    if (recent_models_.size() > max_recent_models_) {
        recent_models_.resize(max_recent_models_);
    }
}

std::string ConfigManager::GenerateModelHash(const std::string& model_path) const {
    // Simple hash based on the full path - in production use proper hash like SHA256
    std::hash<std::string> hasher;
    size_t hash = hasher(model_path);
    
    std::stringstream ss;
    ss << std::hex << hash;
    return ss.str();
}

std::string ConfigManager::GetModelConfigPath(const std::string& model_path) const {
#ifdef _WIN32
    const char* home = getenv("USERPROFILE");
#else
    const char* home = getenv("HOME");
#endif
    
    if (!home) {
        return "";
    }
    
    fs::path config_dir = fs::path(home) / ".config" / "llama_manager" / "models";
    
    // Create directory if it doesn't exist
    if (!fs::exists(config_dir)) {
        fs::create_directories(config_dir);
    }
    
    std::string hash = GenerateModelHash(model_path);
    return (config_dir / (hash + ".json")).string();
}

void ConfigManager::SaveModelSettings(const std::string& model_path, const InferenceConfig& config) {
    if (model_path.empty()) return;
    
    std::string config_path = GetModelConfigPath(model_path);
    if (config_path.empty()) return;
    
    std::ofstream file(config_path);
    if (!file.is_open()) {
        return;
    }
    
    // Write model-specific configuration
    file << "{\n";
    file << "  \"model_path\": \"" << model_path << "\",\n";
    file << "  \"context_length\": " << config.context_length << ",\n";
    file << "  \"gpu_layers\": " << config.gpu_layers << ",\n";
    file << "  \"batch_size\": " << config.batch_size << ",\n";
    file << "  \"threads\": " << config.threads << ",\n";
    file << "  \"rope_freq_base\": " << config.rope_freq_base << ",\n";
    file << "  \"rope_freq_scale\": " << config.rope_freq_scale << ",\n";
    file << "  \"offload_kv_cache\": " << (config.offload_kv_cache ? "true" : "false") << ",\n";
    file << "  \"use_mmap\": " << (config.use_mmap ? "true" : "false") << ",\n";
    file << "  \"keep_model_in_memory\": " << (config.keep_model_in_memory ? "true" : "false") << ",\n";
    file << "  \"num_experts\": " << config.num_experts << ",\n";
    file << "  \"force_experts_cpu\": " << (config.force_experts_cpu ? "true" : "false") << ",\n";
    file << "  \"flash_attention\": " << (config.flash_attention ? "true" : "false") << ",\n";
    file << "  \"quantize_k_cache\": " << (config.quantize_k_cache ? "true" : "false") << ",\n";
    file << "  \"quantize_v_cache\": " << (config.quantize_v_cache ? "true" : "false") << ",\n";
    file << "  \"seed\": " << config.seed << "\n";
    file << "}\n";
}

bool ConfigManager::LoadModelSettings(const std::string& model_path, InferenceConfig& config) {
    if (model_path.empty()) return false;
    
    std::string config_path = GetModelConfigPath(model_path);
    if (config_path.empty() || !fs::exists(config_path)) {
        return false;
    }
    
    std::ifstream file(config_path);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    bool found_settings = false;
    
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#' || line[0] == '/' || line[0] == '{' || line[0] == '}') {
            continue;
        }
        
        size_t colon = line.find(':');
        if (colon == std::string::npos) {
            continue;
        }
        
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        
        // Trim whitespace and quotes
        auto trim = [](std::string& s) {
            s.erase(0, s.find_first_not_of(" \t\n\r\""));
            s.erase(s.find_last_not_of(" \t\n\r\",") + 1);
        };
        
        trim(key);
        trim(value);
        
        // Parse values
        try {
            if (key == "context_length") { config.context_length = std::stoul(value); found_settings = true; }
            else if (key == "gpu_layers") { config.gpu_layers = std::stoul(value); found_settings = true; }
            else if (key == "batch_size") { config.batch_size = std::stoul(value); found_settings = true; }
            else if (key == "threads") { config.threads = std::stoul(value); found_settings = true; }
            else if (key == "rope_freq_base") { config.rope_freq_base = std::stof(value); found_settings = true; }
            else if (key == "rope_freq_scale") { config.rope_freq_scale = std::stof(value); found_settings = true; }
            else if (key == "offload_kv_cache") { config.offload_kv_cache = (value == "true"); found_settings = true; }
            else if (key == "use_mmap") { config.use_mmap = (value == "true"); found_settings = true; }
            else if (key == "keep_model_in_memory") { config.keep_model_in_memory = (value == "true"); found_settings = true; }
            else if (key == "num_experts") { config.num_experts = std::stoul(value); found_settings = true; }
            else if (key == "force_experts_cpu") { config.force_experts_cpu = (value == "true"); found_settings = true; }
            else if (key == "flash_attention") { config.flash_attention = (value == "true"); found_settings = true; }
            else if (key == "quantize_k_cache") { config.quantize_k_cache = (value == "true"); found_settings = true; }
            else if (key == "quantize_v_cache") { config.quantize_v_cache = (value == "true"); found_settings = true; }
            else if (key == "seed") { config.seed = std::stoi(value); found_settings = true; }
        } catch (...) {
            // Skip invalid values
        }
    }
    
    return found_settings;
}

bool ConfigManager::HasModelSettings(const std::string& model_path) const {
    if (model_path.empty()) return false;
    
    std::string config_path = GetModelConfigPath(model_path);
    return !config_path.empty() && fs::exists(config_path);
}

void ConfigManager::ClearModelSettings(const std::string& model_path) {
    if (model_path.empty()) return;
    
    std::string config_path = GetModelConfigPath(model_path);
    if (!config_path.empty() && fs::exists(config_path)) {
        fs::remove(config_path);
    }
}

} // namespace llama_manager
