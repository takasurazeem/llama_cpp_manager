#pragma once

#include <string>
#include <vector>
#include <functional>

namespace llama_manager {

class FileBrowser {
public:
    FileBrowser();
    ~FileBrowser();
    
    // Display file browser dialog
    bool Show(const char* title, std::string& selected_path);
    
    // Scan directory for GGUF files
    std::vector<std::string> ScanForGGUF(const std::string& directory) const;
    
    // Get current directory
    std::string GetCurrentDirectory() const { return current_dir_; }
    void SetCurrentDirectory(const std::string& dir) { current_dir_ = dir; }
    
private:
    std::string current_dir_;
    std::vector<std::string> file_list_;
    
    void RefreshFileList();
    bool IsGGUFFile(const std::string& filename) const;
};

} // namespace llama_manager
