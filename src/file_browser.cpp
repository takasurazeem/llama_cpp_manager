#include "file_browser.h"
#include "imgui.h"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace llama_manager {

FileBrowser::FileBrowser() {
#ifdef _WIN32
    current_dir_ = "C:\\";
#else
    const char* home = getenv("HOME");
    current_dir_ = home ? home : "/";
#endif
}

FileBrowser::~FileBrowser() {
}

bool FileBrowser::IsGGUFFile(const std::string& filename) const {
    std::string lower = filename;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    // C++17 compatible ends_with
    const std::string suffix = ".gguf";
    if (lower.length() >= suffix.length()) {
        return lower.compare(lower.length() - suffix.length(), suffix.length(), suffix) == 0;
    }
    return false;
}

void FileBrowser::RefreshFileList() {
    file_list_.clear();
    
    try {
        for (const auto& entry : fs::directory_iterator(current_dir_)) {
            if (entry.is_directory() || IsGGUFFile(entry.path().filename().string())) {
                file_list_.push_back(entry.path().string());
            }
        }
    } catch (...) {
        // Directory access error
    }
    
    // Sort: directories first, then files
    std::sort(file_list_.begin(), file_list_.end(), [](const std::string& a, const std::string& b) {
        bool a_is_dir = fs::is_directory(a);
        bool b_is_dir = fs::is_directory(b);
        
        if (a_is_dir && !b_is_dir) return true;
        if (!a_is_dir && b_is_dir) return false;
        return a < b;
    });
}

bool FileBrowser::Show(const char* title, std::string& selected_path) {
    RefreshFileList();
    
    bool file_selected = false;
    static bool is_open = true;
    
    ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(title, &is_open, ImGuiWindowFlags_NoCollapse)) {
        // Current directory display
        ImGui::Text("Current Directory:");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "%s", current_dir_.c_str());
        
        ImGui::Separator();
        
        // File list
        ImGui::BeginChild("FileList", ImVec2(0, -30), true);
        
        // Parent directory option
        if (current_dir_ != "/" && current_dir_ != "C:\\") {
            if (ImGui::Selectable(".. (Parent Directory)", false, ImGuiSelectableFlags_DontClosePopups)) {
                try {
                    fs::path parent = fs::path(current_dir_).parent_path();
                    if (!parent.empty()) {
                        current_dir_ = parent.string();
                        RefreshFileList();
                    }
                } catch (...) {
                    // Ignore navigation errors
                }
            }
        }
        
        // Display files and directories
        int item_id = 0;
        for (const auto& item : file_list_) {
            fs::path p(item);
            std::string display_name = p.filename().string();
            
            // Skip if filename is empty (shouldn't happen but safety check)
            if (display_name.empty()) {
                continue;
            }
            
            bool is_dir = fs::is_directory(item);
            
            if (is_dir) {
                display_name = "[DIR] " + display_name;
            }
            
            // Use unique ID to avoid collision
            ImGui::PushID(item_id++);
            if (ImGui::Selectable(display_name.c_str(), false, ImGuiSelectableFlags_DontClosePopups)) {
                if (is_dir) {
                    current_dir_ = item;
                    RefreshFileList();
                } else {
                    // File selected
                    selected_path = item;
                    file_selected = true;
                    is_open = false;
                }
            }
            ImGui::PopID();
        }
        
        ImGui::EndChild();
        
        // Bottom buttons
        ImGui::Separator();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            is_open = false;
        }
        
        ImGui::SameLine();
        ImGui::Text("Select a .gguf file to continue");
    }
    ImGui::End();
    
    if (!is_open) {
        is_open = true;  // Reset for next time
    }
    
    return file_selected;
}

std::vector<std::string> FileBrowser::ScanForGGUF(const std::string& directory) const {
    std::vector<std::string> gguf_files;
    
    try {
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (entry.is_regular_file() && IsGGUFFile(entry.path().filename().string())) {
                gguf_files.push_back(entry.path().string());
            }
        }
    } catch (...) {
        // Directory access error
    }
    
    return gguf_files;
}

} // namespace llama_manager
