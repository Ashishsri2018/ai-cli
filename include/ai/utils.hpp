#pragma once

#include <string>
#include <vector>
#include <optional>

namespace ai::utils {

std::string trim(const std::string& str);
std::string to_lower(const std::string& str);
bool starts_with(const std::string& str, const std::string& prefix);
bool ends_with(const std::string& str, const std::string& suffix);
std::vector<std::string> split(const std::string& str, char delimiter);
std::string replace_all(std::string str, const std::string& from, const std::string& to);

std::string get_home_dir();
std::string get_config_dir();
std::string get_config_file_path();

bool read_file(const std::string& path, std::string& out_content);
bool write_file(const std::string& path, const std::string& content, bool secure = false);
bool create_directories(const std::string& path);
std::optional<std::string> get_env(const std::string& var_name);

} // namespace ai::utils
