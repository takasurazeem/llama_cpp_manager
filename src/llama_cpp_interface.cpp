#include "llama_cpp_interface.h"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <regex>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace fs = std::filesystem;

namespace llama_manager {

LlamaCppInterface::LlamaCppInterface() {
    installation_.is_valid = false;
}

LlamaCppInterface::~LlamaCppInterface() {
}

std::vector<std::string> LlamaCppInterface::GetCommonInstallPaths() const {
    std::vector<std::string> paths;
    
#ifdef _WIN32
    paths.push_back("C:\\Program Files\\llama.cpp");
    paths.push_back("C:\\llama.cpp");
    paths.push_back(std::string(getenv("USERPROFILE") ? getenv("USERPROFILE") : "") + "\\llama.cpp");
    paths.push_back(std::string(getenv("LOCALAPPDATA") ? getenv("LOCALAPPDATA") : "") + "\\llama.cpp");
#elif __APPLE__
    paths.push_back("/usr/local/bin");
    paths.push_back("/opt/homebrew/bin");
    paths.push_back("/opt/local/bin");
    paths.push_back(std::string(getenv("HOME") ? getenv("HOME") : "") + "/llama.cpp");
    paths.push_back(std::string(getenv("HOME") ? getenv("HOME") : "") + "/.local/bin");
#else
    // Linux
    paths.push_back("/usr/local/bin");
    paths.push_back("/usr/bin");
    paths.push_back("/opt/llama.cpp");
    paths.push_back(std::string(getenv("HOME") ? getenv("HOME") : "") + "/llama.cpp");
    paths.push_back(std::string(getenv("HOME") ? getenv("HOME") : "") + "/.local/bin");
    paths.push_back("/snap/bin");
#endif
    
    return paths;
}

std::string LlamaCppInterface::FindExecutable(const std::string& name) {
#ifdef _WIN32
    std::string exe_name = name + ".exe";
#else
    std::string exe_name = name;
#endif
    
    // Check in system PATH first
    std::string path_env = getenv("PATH") ? getenv("PATH") : "";
    
#ifdef _WIN32
    char delimiter = ';';
#else
    char delimiter = ':';
#endif
    
    std::stringstream ss(path_env);
    std::string path_dir;
    
    while (std::getline(ss, path_dir, delimiter)) {
        fs::path full_path = fs::path(path_dir) / exe_name;
        if (fs::exists(full_path) && fs::is_regular_file(full_path)) {
            return full_path.string();
        }
    }
    
    // Check common locations
    for (const auto& dir : GetCommonInstallPaths()) {
        fs::path full_path = fs::path(dir) / exe_name;
        if (fs::exists(full_path) && fs::is_regular_file(full_path)) {
            return full_path.string();
        }
        
        // Also check in bin subdirectory
        fs::path bin_path = fs::path(dir) / "bin" / exe_name;
        if (fs::exists(bin_path) && fs::is_regular_file(bin_path)) {
            return bin_path.string();
        }
    }
    
    return "";
}

bool LlamaCppInterface::ExecuteCommand(const std::string& command, std::string& output, int& exit_code) {
#ifdef _WIN32
    // Windows implementation
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
    
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        return false;
    }
    
    STARTUPINFOA si = {sizeof(STARTUPINFOA)};
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;
    si.wShowWindow = SW_HIDE;
    
    PROCESS_INFORMATION pi;
    
    if (!CreateProcessA(NULL, const_cast<char*>(command.c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return false;
    }
    
    CloseHandle(hWritePipe);
    
    char buffer[4096];
    DWORD bytesRead;
    std::ostringstream oss;
    
    while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        oss << buffer;
    }
    
    WaitForSingleObject(pi.hProcess, INFINITE);
    
    DWORD dwExitCode;
    GetExitCodeProcess(pi.hProcess, &dwExitCode);
    exit_code = static_cast<int>(dwExitCode);
    
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hReadPipe);
    
    output = oss.str();
    return true;
#else
    // Unix implementation
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return false;
    }
    
    char buffer[4096];
    std::ostringstream oss;
    
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        oss << buffer;
    }
    
    exit_code = pclose(pipe);
    output = oss.str();
    return true;
#endif
}

std::string LlamaCppInterface::GetLlamaCppVersion(const std::string& llama_cli_path) {
    std::string output;
    int exit_code;
    
    std::string command = "\"" + llama_cli_path + "\" --version 2>&1";
    
    if (ExecuteCommand(command, output, exit_code)) {
        // Parse version from output
        std::regex version_regex(R"((\d+\.\d+\.\d+))");
        std::smatch match;
        if (std::regex_search(output, match, version_regex)) {
            return match[1].str();
        }
    }
    
    return "unknown";
}

bool LlamaCppInterface::DetectInstallation() {
    installation_.is_valid = false;
    
    // Try to find llama-cli (newer) or main (older)
    std::string llama_cli = FindExecutable("llama-cli");
    if (llama_cli.empty()) {
        llama_cli = FindExecutable("main");
    }
    
    if (llama_cli.empty()) {
        installation_.error_message = "llama.cpp not found in system PATH or common locations. "
                                     "Please install llama.cpp or specify its location.";
        return false;
    }
    
    installation_.llama_cli_path = llama_cli;
    
    // Try to find gguf-dump or llama-gguf-dump
    std::string gguf_dump = FindExecutable("llama-gguf-dump");
    if (gguf_dump.empty()) {
        gguf_dump = FindExecutable("gguf-dump");
    }
    
    installation_.gguf_info_path = gguf_dump;
    
    // Get install path (parent directory)
    fs::path install_path = fs::path(llama_cli).parent_path();
    if (install_path.filename() == "bin") {
        install_path = install_path.parent_path();
    }
    installation_.install_path = install_path.string();
    
    // Get version
    installation_.version = GetLlamaCppVersion(llama_cli);
    
    installation_.is_valid = true;
    installation_.error_message = "";
    
    return true;
}

bool LlamaCppInterface::SetCustomPath(const std::string& path) {
    if (!fs::exists(path)) {
        installation_.error_message = "Path does not exist: " + path;
        return false;
    }
    
#ifdef _WIN32
    std::string exe_name = "llama-cli.exe";
    std::string alt_exe = "main.exe";
#else
    std::string exe_name = "llama-cli";
    std::string alt_exe = "main";
#endif
    
    fs::path llama_cli_path = fs::path(path) / exe_name;
    if (!fs::exists(llama_cli_path)) {
        llama_cli_path = fs::path(path) / alt_exe;
    }
    if (!fs::exists(llama_cli_path)) {
        llama_cli_path = fs::path(path) / "bin" / exe_name;
    }
    if (!fs::exists(llama_cli_path)) {
        llama_cli_path = fs::path(path) / "bin" / alt_exe;
    }
    
    if (!fs::exists(llama_cli_path)) {
        installation_.error_message = "Could not find llama-cli or main executable in: " + path;
        return false;
    }
    
    installation_.install_path = path;
    installation_.llama_cli_path = llama_cli_path.string();
    installation_.version = GetLlamaCppVersion(llama_cli_path.string());
    installation_.is_valid = true;
    installation_.error_message = "";
    
    return true;
}

bool LlamaCppInterface::GetModelInfo(const std::string& model_path, GGUFModelInfo& info) {
    if (!installation_.is_valid) {
        info.error_message = "llama.cpp is not available";
        return false;
    }
    
    // Use gguf-dump if available
    if (!installation_.gguf_info_path.empty()) {
        std::string output;
        int exit_code;
        
        std::string command = "\"" + installation_.gguf_info_path + "\" \"" + model_path + "\" 2>&1";
        
        if (ExecuteCommand(command, output, exit_code) && exit_code == 0) {
            // Parse the output to extract model info
            info.filename = fs::path(model_path).filename().string();
            
            // Extract architecture
            std::regex arch_regex(R"(architecture\s*[:=]\s*(\w+))");
            std::smatch match;
            if (std::regex_search(output, match, arch_regex)) {
                info.architecture = match[1].str();
            }
            
            // Extract layer count
            std::regex layer_regex(R"(block_count\s*[:=]\s*(\d+))");
            if (std::regex_search(output, match, layer_regex)) {
                info.layer_count = std::stoul(match[1].str());
            }
            
            // Extract context length
            std::regex ctx_regex(R"(context_length\s*[:=]\s*(\d+))");
            if (std::regex_search(output, match, ctx_regex)) {
                info.context_length = std::stoul(match[1].str());
            }
            
            // Extract embedding length
            std::regex emb_regex(R"(embedding_length\s*[:=]\s*(\d+))");
            if (std::regex_search(output, match, emb_regex)) {
                info.embedding_length = std::stoul(match[1].str());
            }
            
            // Extract head count
            std::regex head_regex(R"(attention\.head_count\s*[:=]\s*(\d+))");
            if (std::regex_search(output, match, head_regex)) {
                info.head_count = std::stoul(match[1].str());
            }
            
            // Get file size
            info.total_size_bytes = fs::file_size(model_path);
            info.size_gb = info.total_size_bytes / (1024.0f * 1024.0f * 1024.0f);
            
            info.is_valid = true;
            return true;
        }
    }
    
    // Fallback: just get basic file info
    if (fs::exists(model_path)) {
        info.filename = fs::path(model_path).filename().string();
        info.total_size_bytes = fs::file_size(model_path);
        info.size_gb = info.total_size_bytes / (1024.0f * 1024.0f * 1024.0f);
        info.quantization = GGUFReader().DetectQuantization(info.filename);
        info.is_valid = true;
        return true;
    }
    
    info.error_message = "Failed to read model file";
    return false;
}

std::string LlamaCppInterface::GetModelMetadata(const std::string& model_path) {
    if (!installation_.is_valid || installation_.gguf_info_path.empty()) {
        return "";
    }
    
    std::string output;
    int exit_code;
    
    std::string command = "\"" + installation_.gguf_info_path + "\" \"" + model_path + "\" 2>&1";
    
    if (ExecuteCommand(command, output, exit_code) && exit_code == 0) {
        return output;
    }
    
    return "";
}

std::optional<uint64_t> LlamaCppInterface::EstimateModelVRAM(const std::string& model_path,
                                                               uint32_t context_length,
                                                               uint32_t gpu_layers) {
    if (!installation_.is_valid) {
        return std::nullopt;
    }
    
    // Run llama-cli with --verbose to get memory estimation
    std::string output;
    int exit_code;
    
    std::ostringstream cmd;
    cmd << "\"" << installation_.llama_cli_path << "\" "
        << "-m \"" << model_path << "\" "
        << "-c " << context_length << " "
        << "-ngl " << gpu_layers << " "
        << "--verbose "
        << "-n 0 2>&1";  // Don't generate, just load
    
    if (ExecuteCommand(cmd.str(), output, exit_code)) {
        // Parse VRAM usage from output
        std::regex vram_regex(R"(VRAM\s+used:\s+(\d+(?:\.\d+)?)\s*([GM]B))");
        std::smatch match;
        
        if (std::regex_search(output, match, vram_regex)) {
            float value = std::stof(match[1].str());
            std::string unit = match[2].str();
            
            uint64_t bytes = 0;
            if (unit == "GB") {
                bytes = static_cast<uint64_t>(value * 1024 * 1024 * 1024);
            } else if (unit == "MB") {
                bytes = static_cast<uint64_t>(value * 1024 * 1024);
            }
            
            return bytes;
        }
    }
    
    return std::nullopt;
}

} // namespace llama_manager
