#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include "llama_cpp_interface.h"
#include "gguf_reader.h"
#include "vram_estimator.h"
#include "gpu_detector.h"
#include "config_manager.h"
#include "file_browser.h"

#include <SDL2/SDL.h>
#include <GL/gl.h>
#include <string>
#include <memory>
#include <filesystem>

namespace fs = std::filesystem;

using namespace llama_manager;

// Application state
struct AppState {
    // llama.cpp integration
    std::unique_ptr<LlamaCppInterface> llama_interface;
    bool llama_cpp_available = false;
    std::string llama_cpp_error;
    bool show_llama_setup = false;
    char custom_llama_path[512] = "";
    
    // Model information
    std::unique_ptr<GGUFModelInfo> model_info;
    std::string selected_model_path;
    bool model_loaded = false;
    
    // Configuration
    std::unique_ptr<ConfigManager> config_manager;
    InferenceConfig config;
    
    // GPU information
    std::unique_ptr<GPUDetector> gpu_detector;
    GPUInfo primary_gpu;
    
    // VRAM estimation
    std::unique_ptr<VRAMEstimator> vram_estimator;
    VRAMEstimate vram_estimate;
    
    // File browser
    std::unique_ptr<FileBrowser> file_browser;
    bool show_file_browser = false;
    
    // UI state
    bool show_advanced_settings = true;  // Show by default
    bool remember_settings = true;
};

// Helper function to get system RAM
uint64_t GetSystemRAM() {
#ifdef __linux__
    FILE* fp = fopen("/proc/meminfo", "r");
    if (!fp) return 16ULL * 1024 * 1024 * 1024; // Default 16GB
    
    char line[256];
    uint64_t mem_total = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "MemTotal: %lu kB", &mem_total) == 1) {
            fclose(fp);
            return mem_total * 1024; // Convert KB to bytes
        }
    }
    fclose(fp);
    return 16ULL * 1024 * 1024 * 1024; // Default 16GB
#elif defined(__APPLE__)
    int mib[2] = {CTL_HW, HW_MEMSIZE};
    uint64_t size = 0;
    size_t len = sizeof(size);
    if (sysctl(mib, 2, &size, &len, NULL, 0) == 0) {
        return size;
    }
    return 16ULL * 1024 * 1024 * 1024; // Default 16GB
#elif defined(_WIN32)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
#else
    return 16ULL * 1024 * 1024 * 1024; // Default 16GB
#endif
}

// Helper function to apply intelligent defaults
void ApplyIntelligentDefaults(AppState& state) {
    if (!state.model_loaded || !state.model_info) return;
    
    // Get available resources
    uint64_t available_vram = 0;
    if (state.gpu_detector->HasGPU()) {
        available_vram = state.primary_gpu.available_vram_bytes;
        // If available_vram is 0 or seems wrong, use total_vram
        if (available_vram == 0) {
            available_vram = state.primary_gpu.total_vram_bytes;
        }
    }
    
    uint64_t system_ram = GetSystemRAM();
    
    // Calculate optimal configuration
    InferenceConfig optimal = state.vram_estimator->CalculateOptimalConfig(
        *state.model_info,
        available_vram,
        system_ram
    );
    
    // Apply optimal settings to current config
    state.config.context_length = optimal.context_length;
    state.config.gpu_layers = optimal.gpu_layers;
    state.config.offload_kv_cache = optimal.offload_kv_cache;
    
    // Keep user preferences for other settings if they exist
    // Only update if they're at defaults
    if (state.config.batch_size == 512) {
        state.config.batch_size = optimal.batch_size;
    }
}

void RenderLlamaCppSetup(AppState& state) {
    ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("llama.cpp Setup Required", &state.show_llama_setup)) {
        ImGui::TextWrapped("This utility requires llama.cpp to be installed on your system.");
        ImGui::Separator();
        
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Error:");
        ImGui::TextWrapped("%s", state.llama_cpp_error.c_str());
        ImGui::Spacing();
        
        ImGui::TextWrapped("Please choose one of the following options:");
        ImGui::Spacing();
        
        if (ImGui::CollapsingHeader("Option 1: Install llama.cpp", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent();
            ImGui::TextWrapped("Visit: https://github.com/ggerganov/llama.cpp");
            ImGui::Spacing();
            ImGui::Text("Installation instructions:");
            ImGui::BulletText("Linux/macOS: Clone repo, run 'make' or 'cmake'");
            ImGui::BulletText("Windows: Download prebuilt binaries or build with CMake");
            ImGui::BulletText("Ensure llama-cli is in your system PATH");
            ImGui::Unindent();
        }
        
        ImGui::Spacing();
        
        if (ImGui::CollapsingHeader("Option 2: Specify llama.cpp Location")) {
            ImGui::Indent();
            ImGui::Text("llama.cpp installation path:");
            ImGui::InputText("##llamapath", state.custom_llama_path, sizeof(state.custom_llama_path));
            ImGui::SameLine();
            if (ImGui::Button("Browse...")) {
                // TODO: Open directory browser
            }
            
            if (ImGui::Button("Set Path")) {
                if (state.llama_interface->SetCustomPath(state.custom_llama_path)) {
                    state.llama_cpp_available = true;
                    state.show_llama_setup = false;
                } else {
                    state.llama_cpp_error = state.llama_interface->GetInstallation().error_message;
                }
            }
            ImGui::Unindent();
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        
        if (ImGui::Button("Retry Detection")) {
            if (state.llama_interface->DetectInstallation()) {
                state.llama_cpp_available = true;
                state.show_llama_setup = false;
            } else {
                state.llama_cpp_error = state.llama_interface->GetInstallation().error_message;
            }
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Exit")) {
            SDL_Event quit_event;
            quit_event.type = SDL_QUIT;
            SDL_PushEvent(&quit_event);
        }
    }
    ImGui::End();
}

void RenderModelSelection(AppState& state) {
    ImGui::Text("Model Selection");
    ImGui::Separator();
    
    // Browse button at the top
    if (ImGui::Button("Browse GGUF Model...", ImVec2(-1, 30))) {
        state.show_file_browser = true;
    }
    
    ImGui::Spacing();
    
    // Show currently selected model
    ImGui::Text("Selected Model:");
    if (state.model_loaded && state.model_info) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", state.model_info->filename.c_str());
    } else {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "None");
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // Show recently selected models section - always show if there are any
    const auto& recent_models = state.config_manager->GetRecentModels();
    if (!recent_models.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Recently Selected Models:");
        
        ImGui::BeginChild("RecentModels", ImVec2(0, 120), true);
        
        for (size_t i = 0; i < recent_models.size(); ++i) {
            const std::string& model_path = recent_models[i];
            
            // Check if file still exists
            bool exists = fs::exists(model_path);
            
            // Extract filename for display
            std::string display_name = fs::path(model_path).filename().string();
            
            // Show model entry
            ImGui::PushID(static_cast<int>(i));
            
            if (!exists) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
            }
            
            // Highlight currently loaded model
            bool is_current = (model_path == state.selected_model_path);
            if (is_current) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 0.6f));
            }
            
            if (ImGui::Button(display_name.c_str(), ImVec2(-1, 0))) {
                if (exists) {
                    state.selected_model_path = model_path;
                    
                    // Load model info
                    GGUFReader reader;
                    if (reader.ReadFile(model_path)) {
                        state.model_info = std::make_unique<GGUFModelInfo>();
                        *state.model_info = reader.GetModelInfo();
                        state.model_loaded = state.model_info->is_valid;
                        
                        if (state.model_loaded) {
                            // Try to load saved settings for this model first
                            InferenceConfig model_config;
                            if (state.config_manager->HasModelSettings(model_path) && 
                                state.config_manager->LoadModelSettings(model_path, model_config)) {
                                // Restore saved settings for this model
                                state.config = model_config;
                            } else {
                                // No saved settings - apply intelligent defaults
                                ApplyIntelligentDefaults(state);
                            }
                        }
                    } else {
                        state.model_loaded = false;
                    }
                }
            }
            
            if (is_current) {
                ImGui::PopStyleColor();
            }
            
            if (!exists) {
                ImGui::PopStyleColor();
            }
            
            // Show tooltip with full path
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("%s", model_path.c_str());
                if (!exists) {
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "(File not found)");
                }
                ImGui::EndTooltip();
            }
            
            ImGui::PopID();
        }
        
        ImGui::EndChild();
    }
    
    // Show file browser dialog
    if (state.show_file_browser) {
        std::string selected_path;
        if (state.file_browser->Show("Select GGUF Model", selected_path)) {
            state.selected_model_path = selected_path;
            state.show_file_browser = false;
            
            // Load model info
            GGUFReader reader;
            if (reader.ReadFile(selected_path)) {
                state.model_info = std::make_unique<GGUFModelInfo>();
                *state.model_info = reader.GetModelInfo();
                state.model_loaded = state.model_info->is_valid;
                
                if (state.model_loaded) {
                    // Try to load saved settings for this model first
                    InferenceConfig model_config;
                    if (state.config_manager->HasModelSettings(selected_path) && 
                        state.config_manager->LoadModelSettings(selected_path, model_config)) {
                        // Restore saved settings for this model
                        state.config = model_config;
                    } else {
                        // No saved settings - apply intelligent defaults
                        ApplyIntelligentDefaults(state);
                    }
                }
                
                // Add to recent models
                state.config_manager->AddRecentModel(selected_path);
                state.config_manager->SetLastModelPath(selected_path);
                state.config_manager->SaveConfig("");
            } else {
                state.model_loaded = false;
            }
        }
    }
    
    if (!state.selected_model_path.empty()) {
        ImGui::Text("Path: %s", state.selected_model_path.c_str());
    }
    
    // Show model info if loaded
    if (state.model_loaded && state.model_info && state.model_info->is_valid) {
        ImGui::Spacing();
        ImGui::BeginChild("ModelInfo", ImVec2(0, 100), true);
        ImGui::Text("Architecture: %s", state.model_info->architecture.c_str());
        ImGui::Text("Quantization: %s", GGUFReader::QuantizationToString(state.model_info->quantization).c_str());
        
        // Show parameter count - format in billions for readability
        if (state.model_info->parameter_count >= 1000000000) {
            float params_b = state.model_info->parameter_count / 1000000000.0f;
            ImGui::Text("Parameters: %.1fB", params_b);
        } else if (state.model_info->parameter_count >= 1000000) {
            float params_m = state.model_info->parameter_count / 1000000.0f;
            ImGui::Text("Parameters: %.1fM", params_m);
        } else {
            ImGui::Text("Parameters: %llu", state.model_info->parameter_count);
        }
        
        ImGui::Text("Layers: %u", state.model_info->layer_count);
        ImGui::Text("File Size: %.2f GB", state.model_info->size_gb);
        ImGui::EndChild();
    }
}

void RenderMemoryEstimate(AppState& state) {
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Estimated Memory Usage");
    ImGui::Separator();
    
    // Calculate current estimate only if model is loaded
    if (state.model_loaded && state.model_info) {
        state.vram_estimate = state.vram_estimator->Calculate(*state.model_info, state.config);
    } else {
        // Reset estimate when no model loaded
        state.vram_estimate = VRAMEstimate();
    }
    
    float gpu_gb = state.vram_estimate.GetTotalVRAM_GB();
    float total_gb = state.vram_estimate.GetTotal_GB();
    
    // GPU usage bar
    ImGui::Text("GPU:");
    ImGui::SameLine(100);
    
    if (state.model_loaded) {
        ImGui::Text("%.2f GB", gpu_gb);
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "N/A (no model loaded)");
    }
    
    if (state.gpu_detector->HasGPU() && state.model_loaded) {
        float available = state.primary_gpu.GetTotalVRAM_GB();
        float usage_percent = (gpu_gb / available) * 100.0f;
        
        ImVec4 color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        if (usage_percent > 90.0f) {
            color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        } else if (usage_percent > 75.0f) {
            color = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
        }
        
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, color);
        ImGui::ProgressBar(gpu_gb / available, ImVec2(-1, 0));
        ImGui::PopStyleColor();
        
        if (usage_percent > 100.0f) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 
                             "WARNING: Configuration exceeds available VRAM!");
        }
    }
    
    // Total usage
    ImGui::Text("Total:");
    ImGui::SameLine(100);
    if (state.model_loaded) {
        ImGui::Text("%.2f GB", total_gb);
    } else {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "N/A (no model loaded)");
    }
}

void RenderInferenceSettings(AppState& state) {
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Inference Settings");
    ImGui::Separator();
    
    // Context Length
    ImGui::Text("Context Length");
    int ctx_len = static_cast<int>(state.config.context_length);
    int max_ctx = state.model_info ? state.model_info->context_length : 262144;
    ImGui::SliderInt("##ctx", &ctx_len, 512, max_ctx);
    state.config.context_length = static_cast<uint32_t>(ctx_len);
    ImGui::SameLine();
    ImGui::Text("Model supports up to %d tokens", max_ctx);
    
    // GPU Offload
    ImGui::Text("GPU Offload");
    int gpu_layers = static_cast<int>(state.config.gpu_layers);
    int max_layers = state.model_info ? state.model_info->layer_count : 48;
    ImGui::SliderInt("##gpuoffload", &gpu_layers, 0, max_layers);
    state.config.gpu_layers = static_cast<uint32_t>(gpu_layers);
    ImGui::SameLine();
    ImGui::Text("/ %d", max_layers);
    
    // CPU Thread Pool Size
    ImGui::Text("CPU Thread Pool Size");
    int threads = static_cast<int>(state.config.threads);
    ImGui::SliderInt("##threads", &threads, 1, 32);
    state.config.threads = static_cast<uint32_t>(threads);
    
    // Evaluation Batch Size
    ImGui::Text("Evaluation Batch Size");
    int batch = static_cast<int>(state.config.batch_size);
    ImGui::SliderInt("##batch", &batch, 32, 2048);
    state.config.batch_size = static_cast<uint32_t>(batch);
    
    // RoPE settings
    ImGui::Text("RoPE Frequency Base");
    const char* rope_base_items[] = { "Auto", "Custom" };
    static int rope_base_idx = 0;
    ImGui::Combo("##ropebase", &rope_base_idx, rope_base_items, 2);
    if (rope_base_idx == 1) {
        ImGui::InputFloat("##ropebaseval", &state.config.rope_freq_base);
    } else {
        state.config.rope_freq_base = 0.0f;
    }
    
    ImGui::Text("RoPE Frequency Scale");
    const char* rope_scale_items[] = { "Auto", "Custom" };
    static int rope_scale_idx = 0;
    ImGui::Combo("##ropescale", &rope_scale_idx, rope_scale_items, 2);
    if (rope_scale_idx == 1) {
        ImGui::InputFloat("##ropescaleval", &state.config.rope_freq_scale);
    } else {
        state.config.rope_freq_scale = 0.0f;
    }
    
    // Toggles
    ImGui::Checkbox("Offload KV Cache to GPU Memory", &state.config.offload_kv_cache);
    ImGui::Checkbox("Keep Model in Memory", &state.config.keep_model_in_memory);
    ImGui::Checkbox("Try mmap()", &state.config.use_mmap);
    
    // Seed
    ImGui::Text("Seed");
    ImGui::SameLine();
    if (state.config.seed < 0) {
        ImGui::Text("Random Seed");
    } else {
        ImGui::InputInt("##seed", &state.config.seed);
    }
}

std::string GenerateLlamaCppCommand(const AppState& state) {
    if (!state.model_loaded || state.selected_model_path.empty()) {
        return "# No model loaded";
    }
    
    std::string cmd = "llama-cli";
    
    // Model path
    cmd += " -m \"" + state.selected_model_path + "\"";
    
    // Context length
    cmd += " -c " + std::to_string(state.config.context_length);
    
    // GPU layers
    if (state.config.gpu_layers > 0) {
        cmd += " -ngl " + std::to_string(state.config.gpu_layers);
    }
    
    // Threads
    cmd += " -t " + std::to_string(state.config.threads);
    
    // Batch size
    cmd += " -b " + std::to_string(state.config.batch_size);
    
    // RoPE settings
    if (state.config.rope_freq_base > 0.0f) {
        cmd += " --rope-freq-base " + std::to_string(state.config.rope_freq_base);
    }
    if (state.config.rope_freq_scale > 0.0f) {
        cmd += " --rope-freq-scale " + std::to_string(state.config.rope_freq_scale);
    }
    
    // Memory options
    if (!state.config.offload_kv_cache) {
        cmd += " --no-kv-offload";
    }
    if (!state.config.use_mmap) {
        cmd += " --no-mmap";
    }
    if (!state.config.keep_model_in_memory) {
        cmd += " --no-warmup";
    }
    
    // Seed
    if (state.config.seed >= 0) {
        cmd += " -s " + std::to_string(state.config.seed);
    }
    
    // Advanced options
    if (state.config.flash_attention) {
        cmd += " --flash-attn on";
    }
    if (state.config.quantize_k_cache) {
        cmd += " --cache-type-k q8_0";
    }
    if (state.config.quantize_v_cache) {
        cmd += " --cache-type-v q8_0";
    }
    
    // MoE settings
    if (state.config.num_experts > 0) {
        cmd += " --num-experts " + std::to_string(state.config.num_experts);
    }
    if (state.config.force_experts_cpu) {
        cmd += " --no-moe";
    }
    
    // Add prompt placeholder
    cmd += " -p \"Your prompt here\"";
    
    return cmd;
}

void RenderAdvancedSettings(AppState& state) {
    if (!state.show_advanced_settings) return;
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Advanced Settings");
    ImGui::Separator();
    
    // Number of Experts (MoE)
    if (state.model_info && state.model_info->architecture.find("mixtral") != std::string::npos) {
        ImGui::Text("Number of Experts");
        int experts = static_cast<int>(state.config.num_experts);
        ImGui::SliderInt("##experts", &experts, 1, 16);
        state.config.num_experts = static_cast<uint32_t>(experts);
        
        ImGui::Checkbox("Force Model Expert Weights onto CPU", &state.config.force_experts_cpu);
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Experimental");
    }
    
    // Flash Attention
    ImGui::Checkbox("Flash Attention", &state.config.flash_attention);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Experimental");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Flash Attention is an experimental feature that may cause issues with some models.");
    }
    
    // Cache Quantization
    ImGui::Checkbox("K Cache Quantization Type", &state.config.quantize_k_cache);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Experimental");
    
    ImGui::Checkbox("V Cache Quantization Type", &state.config.quantize_v_cache);
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Experimental");
}

void RenderMainWindow(AppState& state) {
    ImGuiIO& io = ImGui::GetIO();
    
    // Main window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar |
                                    ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoCollapse;
    
    ImGui::Begin("LLAMA.CPP Manager", nullptr, window_flags);
    
    // Menu bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save Configuration", "Ctrl+S")) {
                // Save current configuration
                state.config_manager->SetConfig(state.config);
                state.config_manager->SetRememberSettings(state.remember_settings);
                if (state.model_loaded && !state.selected_model_path.empty()) {
                    state.config_manager->SetLastModelPath(state.selected_model_path);
                    
                    // Save model-specific settings if remember_settings is enabled
                    if (state.remember_settings) {
                        state.config_manager->SaveModelSettings(state.selected_model_path, state.config);
                    }
                }
                state.config_manager->SaveConfig("");
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("Quit", "Alt+F4")) {
                // Save configuration before quitting
                state.config_manager->SetConfig(state.config);
                state.config_manager->SetRememberSettings(state.remember_settings);
                if (state.model_loaded && !state.selected_model_path.empty()) {
                    state.config_manager->SetLastModelPath(state.selected_model_path);
                    
                    // Save model-specific settings if remember_settings is enabled
                    if (state.remember_settings) {
                        state.config_manager->SaveModelSettings(state.selected_model_path, state.config);
                    }
                }
                state.config_manager->SaveConfig("");
                
                // Send quit event
                SDL_Event quit_event;
                quit_event.type = SDL_QUIT;
                SDL_PushEvent(&quit_event);
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                ImGui::OpenPopup("About");
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMenuBar();
    }
    
    // About popup
    if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("LLAMA.CPP Manager");
        ImGui::Separator();
        ImGui::Text("A configuration utility for llama.cpp models");
        ImGui::Spacing();
        ImGui::Text("Features:");
        ImGui::BulletText("Intelligent resource detection");
        ImGui::BulletText("Automatic optimal configuration");
        ImGui::BulletText("Recent models history");
        ImGui::BulletText("Memory usage estimation");
        ImGui::Spacing();
        
        if (ImGui::Button("Close", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
    
    // Title
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "LLAMA.CPP Model Configuration");
    ImGui::Separator();
    
    // Model selection at top
    RenderModelSelection(state);
    
    ImGui::Spacing();
    
    // Memory estimate
    RenderMemoryEstimate(state);
    
    ImGui::Spacing();
    
    // Inference Settings
    RenderInferenceSettings(state);
    
    // Advanced settings - show by default with toggle
    ImGui::Spacing();
    ImGui::Checkbox("Show Advanced Settings", &state.show_advanced_settings);
    RenderAdvancedSettings(state);
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // Bottom controls
    ImGui::Checkbox("Remember settings for", &state.remember_settings);
    ImGui::SameLine();
    if (state.model_info) {
        ImGui::Text("%s", state.model_info->filename.c_str());
    } else {
        ImGui::Text("this model");
    }
    
    ImGui::Spacing();
    
    if (ImGui::Button("Save Configuration")) {
        // Save current configuration
        state.config_manager->SetConfig(state.config);
        state.config_manager->SetRememberSettings(state.remember_settings);
        if (state.model_loaded && !state.selected_model_path.empty()) {
            state.config_manager->SetLastModelPath(state.selected_model_path);
            
            // Save model-specific settings if remember_settings is enabled
            if (state.remember_settings) {
                state.config_manager->SaveModelSettings(state.selected_model_path, state.config);
            }
        }
        state.config_manager->SaveConfig("");
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Reset to Defaults")) {
        state.config = InferenceConfig();
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Export CLI Command")) {
        ImGui::OpenPopup("CLI Command");
    }
    
    // Export CLI Command popup
    if (ImGui::BeginPopupModal("CLI Command", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Copy this command to use with llama.cpp:");
        ImGui::Separator();
        
        std::string cmd = GenerateLlamaCppCommand(state);
        
        ImGui::BeginChild("CommandText", ImVec2(700, 150), true);
        ImGui::TextWrapped("%s", cmd.c_str());
        ImGui::EndChild();
        
        ImGui::Spacing();
        
        if (ImGui::Button("Copy to Clipboard", ImVec2(200, 0))) {
            SDL_SetClipboardText(cmd.c_str());
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Close", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
    
    // GPU info footer
    ImGui::Spacing();
    ImGui::Separator();
    if (state.gpu_detector->HasGPU()) {
        ImGui::Text("GPU: %s (%.1f GB VRAM)", 
                   state.primary_gpu.name.c_str(), 
                   state.primary_gpu.GetTotalVRAM_GB());
    } else {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No GPU detected - CPU mode only");
    }
    
    // llama.cpp info
    if (state.llama_cpp_available) {
        ImGui::SameLine();
        ImGui::Text("| llama.cpp: %s", state.llama_interface->GetInstallation().version.c_str());
    }
    
    ImGui::End();
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        return -1;
    }
    
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    
    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    SDL_Window* window = SDL_CreateWindow(
        "LLAMA.CPP Manager",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1000, 800,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );
    
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Setup style
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    // Initialize application state
    AppState state;
    state.llama_interface = std::make_unique<LlamaCppInterface>();
    state.config_manager = std::make_unique<ConfigManager>();
    state.gpu_detector = std::make_unique<GPUDetector>();
    state.vram_estimator = std::make_unique<VRAMEstimator>();
    state.file_browser = std::make_unique<FileBrowser>();
    
    // Detect llama.cpp installation
    if (!state.llama_interface->DetectInstallation()) {
        state.llama_cpp_available = false;
        state.llama_cpp_error = state.llama_interface->GetInstallation().error_message;
        state.show_llama_setup = true;
    } else {
        state.llama_cpp_available = true;
    }
    
    // Detect GPU
    state.gpu_detector->DetectGPUs();
    if (state.gpu_detector->HasGPU()) {
        state.primary_gpu = state.gpu_detector->GetPrimaryGPU();
        state.config.gpu_layers = 32; // Default to some GPU offload
    }
    
    // Load configuration (empty string uses default path)
    state.config_manager->LoadConfig("");
    state.config = state.config_manager->GetConfig();
    
    // DO NOT auto-load last model - user should select from recent list
    // This ensures proper settings management per model
    
    // Main loop
    bool done = false;
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                // Save configuration before quitting
                state.config_manager->SetConfig(state.config);
                state.config_manager->SetRememberSettings(state.remember_settings);
                if (state.model_loaded && !state.selected_model_path.empty()) {
                    state.config_manager->SetLastModelPath(state.selected_model_path);
                    
                    // Save model-specific settings if remember_settings is enabled
                    if (state.remember_settings) {
                        state.config_manager->SaveModelSettings(state.selected_model_path, state.config);
                    }
                }
                state.config_manager->SaveConfig("");
                done = true;
            }
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window)) {
                // Save configuration before quitting
                state.config_manager->SetConfig(state.config);
                state.config_manager->SetRememberSettings(state.remember_settings);
                if (state.model_loaded && !state.selected_model_path.empty()) {
                    state.config_manager->SetLastModelPath(state.selected_model_path);
                    
                    // Save model-specific settings if remember_settings is enabled
                    if (state.remember_settings) {
                        state.config_manager->SaveModelSettings(state.selected_model_path, state.config);
                    }
                }
                state.config_manager->SaveConfig("");
                done = true;
            }
        }
        
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        
        // Render UI
        if (state.show_llama_setup) {
            RenderLlamaCppSetup(state);
        } else {
            RenderMainWindow(state);
        }
        
        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
    
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
