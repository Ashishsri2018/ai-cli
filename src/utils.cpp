#include "ai/utils.hpp"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace ai::utils {

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

std::string to_lower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

bool starts_with(const std::string& str, const std::string& prefix) {
    return str.rfind(prefix, 0) == 0;
}

bool ends_with(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(str);
    while (std::getline(token_stream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string replace_all(std::string str, const std::string& from, const std::string& to) {
    if (from.empty()) return str;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

std::string get_home_dir() {
    const char* home = std::getenv("HOME");
    if (home) return std::string(home);
    const char* userprofile = std::getenv("USERPROFILE");
    if (userprofile) return std::string(userprofile);
    return ".";
}

std::string get_config_dir() {
    return get_home_dir() + "/.config/ai";
}

std::string get_config_file_path() {
    return get_config_dir() + "/config.json";
}

bool read_file(const std::string& path, std::string& out_content) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file.is_open()) return false;
    std::ostringstream ss;
    ss << file.rdbuf();
    out_content = ss.str();
    return true;
}

bool write_file(const std::string& path, const std::string& content, bool secure) {
    std::error_code ec;
    fs::path p(path);
    if (p.has_parent_path()) {
        fs::create_directories(p.parent_path(), ec);
    }
    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file.is_open()) return false;
    file << content;
    file.close();

    if (secure) {
        chmod(path.c_str(), S_IRUSR | S_IWUSR); // 0600
    }
    return true;
}

bool create_directories(const std::string& path) {
    std::error_code ec;
    return fs::create_directories(path, ec);
}

std::optional<std::string> get_env(const std::string& var_name) {
    const char* val = std::getenv(var_name.c_str());
    if (val && *val) return std::string(val);
    return std::nullopt;
}

} // namespace ai::utils
