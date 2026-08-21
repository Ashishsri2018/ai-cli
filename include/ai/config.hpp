#pragma once

#include <string>
#include <map>
#include <optional>
#include "ai/json.hpp"

namespace ai {

struct Config {
    std::string default_provider{"google"};
    std::map<std::string, std::string> default_models;
    std::map<std::string, std::string> api_keys;
    std::map<std::string, std::string> custom_endpoints;
};

class ConfigManager {
public:
    ConfigManager();
    explicit ConfigManager(std::string config_path);

    bool load();
    bool save();

    void set_api_key(const std::string& provider, const std::string& key);
    bool delete_api_key(const std::string& provider);
    std::optional<std::string> get_api_key(const std::string& provider) const;

    void set_default_provider(const std::string& provider);
    std::string get_default_provider() const;

    void set_default_model(const std::string& provider, const std::string& model);
    std::string get_default_model(const std::string& provider) const;

    void set_custom_endpoint(const std::string& provider, const std::string& url);
    std::optional<std::string> get_custom_endpoint(const std::string& provider) const;

    const Config& get_config() const { return config_; }
    static std::string normalize_provider(const std::string& provider);

private:
    std::string config_path_;
    Config config_;
    void init_defaults();
};

} // namespace ai
