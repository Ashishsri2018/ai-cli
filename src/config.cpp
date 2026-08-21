#include "ai/config.hpp"
#include "ai/utils.hpp"
#include "ai/crypto.hpp"

namespace ai {

std::string ConfigManager::normalize_provider(const std::string& name) {
    std::string p = utils::to_lower(utils::trim(name));
    if (p == "gemini" || p == "google") return "google";
    if (p == "openai" || p == "chatgpt") return "openai";
    if (p == "claude" || p == "anthropic") return "anthropic";
    return p;
}

void ConfigManager::init_defaults() {
    config_.default_provider = "google";
    config_.default_models["google"] = "gemini-3.6-flash";
    config_.default_models["openai"] = "gpt-4o-mini";
    config_.default_models["anthropic"] = "claude-3-5-haiku-20241022";
    config_.default_models["groq"] = "llama-3.3-70b-versatile";
    config_.default_models["deepseek"] = "deepseek-chat";
    config_.default_models["ollama"] = "llama3.2";
    config_.custom_endpoints["ollama"] = "http://localhost:11434/v1";
}

ConfigManager::ConfigManager() : config_path_(utils::get_config_file_path()) {
    init_defaults();
    load();
}

ConfigManager::ConfigManager(std::string path) : config_path_(std::move(path)) {
    init_defaults();
    load();
}

bool ConfigManager::load() {
    std::string content;
    if (!utils::read_file(config_path_, content)) return false;
    std::string err;
    Json root = Json::parse(content, err);
    if (!err.empty() || !root.is_object()) return false;

    if (root.has("default_provider")) {
        config_.default_provider = root["default_provider"].as_string();
    }
    if (root.has("default_models") && root["default_models"].is_object()) {
        for (const auto& [k, v] : root["default_models"].obj_val_) config_.default_models[k] = v.as_string();
    }
    if (root.has("api_keys") && root["api_keys"].is_object()) {
        for (const auto& [k, v] : root["api_keys"].obj_val_) config_.api_keys[k] = v.as_string();
    }
    if (root.has("custom_endpoints") && root["custom_endpoints"].is_object()) {
        for (const auto& [k, v] : root["custom_endpoints"].obj_val_) config_.custom_endpoints[k] = v.as_string();
    }
    return true;
}

bool ConfigManager::save() {
    Json root = Json::object();
    root["default_provider"] = config_.default_provider;

    Json dm = Json::object();
    for (const auto& [k, v] : config_.default_models) dm[k] = v;
    root["default_models"] = dm;

    Json ak = Json::object();
    for (const auto& [k, v] : config_.api_keys) {
        ak[k] = crypto::is_encrypted(v) ? v : crypto::encrypt_key(v);
    }
    root["api_keys"] = ak;

    Json ce = Json::object();
    for (const auto& [k, v] : config_.custom_endpoints) ce[k] = v;
    root["custom_endpoints"] = ce;

    return utils::write_file(config_path_, root.dump(2), true);
}

void ConfigManager::set_api_key(const std::string& provider, const std::string& key) {
    config_.api_keys[normalize_provider(provider)] = crypto::encrypt_key(utils::trim(key));
}

bool ConfigManager::delete_api_key(const std::string& provider) {
    auto norm = normalize_provider(provider);
    auto it = config_.api_keys.find(norm);
    if (it != config_.api_keys.end()) {
        config_.api_keys.erase(it);
        return true;
    }
    return false;
}

std::optional<std::string> ConfigManager::get_api_key(const std::string& provider) const {
    auto norm = normalize_provider(provider);
    auto it = config_.api_keys.find(norm);
    if (it != config_.api_keys.end() && !it->second.empty()) {
        return crypto::decrypt_key(it->second);
    }

    if (norm == "google") {
        if (auto env = utils::get_env("GEMINI_API_KEY")) return env;
        if (auto env = utils::get_env("GOOGLE_API_KEY")) return env;
    } else if (norm == "openai") {
        if (auto env = utils::get_env("OPENAI_API_KEY")) return env;
    } else if (norm == "anthropic") {
        if (auto env = utils::get_env("ANTHROPIC_API_KEY")) return env;
    } else if (norm == "groq") {
        if (auto env = utils::get_env("GROQ_API_KEY")) return env;
    } else if (norm == "deepseek") {
        if (auto env = utils::get_env("DEEPSEEK_API_KEY")) return env;
    }
    return std::nullopt;
}

void ConfigManager::set_default_provider(const std::string& p) { config_.default_provider = normalize_provider(p); }
std::string ConfigManager::get_default_provider() const { return config_.default_provider; }

void ConfigManager::set_default_model(const std::string& p, const std::string& m) {
    config_.default_models[normalize_provider(p)] = utils::trim(m);
}

std::string ConfigManager::get_default_model(const std::string& p) const {
    auto norm = normalize_provider(p);
    auto it = config_.default_models.find(norm);
    return it != config_.default_models.end() ? it->second : "default";
}

void ConfigManager::set_custom_endpoint(const std::string& p, const std::string& u) {
    config_.custom_endpoints[normalize_provider(p)] = utils::trim(u);
}

std::optional<std::string> ConfigManager::get_custom_endpoint(const std::string& p) const {
    auto norm = normalize_provider(p);
    auto it = config_.custom_endpoints.find(norm);
    if (it != config_.custom_endpoints.end() && !it->second.empty()) return it->second;
    return std::nullopt;
}

} // namespace ai
