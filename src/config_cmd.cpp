#include "ai/query_runner.hpp"
#include "ai/provider.hpp"
#include "ai/terminal.hpp"
#include "ai/utils.hpp"
#include <iostream>

namespace ai {

namespace {

void handle_list_config(const ConfigManager& cm) {
    const auto& cfg = cm.get_config();
    std::cout << term::colorize("=== AI CLI Configuration ===", term::Color::Bold) << "\n";
    std::cout << "Default Provider: " << term::colorize(cfg.default_provider, term::Color::Cyan) << "\n";
    std::cout << "\n" << term::colorize("Configured API Keys:", term::Color::Yellow) << "\n";
    for (const auto& [prov, key] : cfg.api_keys) {
        std::string masked = (key.size() > 8) ? key.substr(0, 4) + "..." + key.substr(key.size() - 4) : "****";
        std::cout << "  - " << prov << ": " << masked << "\n";
    }
    std::cout << "\n" << term::colorize("Default Models:", term::Color::Yellow) << "\n";
    for (const auto& [prov, model] : cfg.default_models) {
        std::cout << "  - " << prov << ": " << model << "\n";
    }
}

void handle_set_config(ConfigManager& cm, const CliArgs& args) {
    std::string sub = args.config_subcommand;
    if (sub == "api" && args.config_args.size() >= 2) {
        std::string a1 = args.config_args[0], a2 = args.config_args[1], key, prov;
        if (a1.find("sk-") == 0 || a1.find("AIza") == 0 || a1.size() > 20) { key = a1; prov = a2; }
        else { prov = a1; key = a2; }
        cm.set_api_key(prov, key); cm.save();
        term::print_success("Saved API key for provider '" + ConfigManager::normalize_provider(prov) + "'");
    } else if ((sub == "default-provider" || sub == "provider") && !args.config_args.empty()) {
        cm.set_default_provider(args.config_args[0]); cm.save();
        term::print_success("Updated default provider to '" + args.config_args[0] + "'");
    } else if ((sub == "default-model" || sub == "model") && !args.config_args.empty()) {
        std::string model = args.config_args[0];
        std::string prov = (args.config_args.size() > 1) ? args.config_args[1] : cm.get_default_provider();
        cm.set_default_model(prov, model); cm.save();
        term::print_success("Updated default model for '" + prov + "' to '" + model + "'");
    } else {
        term::print_error("Usage: ai --set api <key> <provider> | ai --set model <name> [provider] | ai --set provider <name>");
    }
}

void handle_del_config(ConfigManager& cm, const CliArgs& args) {
    if (args.config_subcommand == "api" && !args.config_args.empty()) {
        if (cm.delete_api_key(args.config_args[0])) {
            cm.save();
            term::print_success("Deleted API key for provider '" + args.config_args[0] + "'");
        } else {
            term::print_error("No API key found for provider '" + args.config_args[0] + "'");
        }
    } else {
        term::print_error("Usage: ai --del api <provider>");
    }
}

} // namespace

void QueryRunner::handle_config_command(ConfigManager& cm, const CliArgs& args) {
    if (args.mode == CliMode::ListConfig) handle_list_config(cm);
    else if (args.mode == CliMode::SetConfig) handle_set_config(cm, args);
    else if (args.mode == CliMode::DelConfig) handle_del_config(cm, args);
}

void QueryRunner::list_provider_models(ConfigManager& cm, IHttpClient& http_client, const std::string& provider_arg) {
    std::string prov = provider_arg.empty() ? cm.get_default_provider() : provider_arg;
    prov = ConfigManager::normalize_provider(prov);

    auto api_key_opt = cm.get_api_key(prov);
    std::string api_key = api_key_opt.value_or("");
    if (api_key.empty() && prov != "ollama") {
        term::print_error("No API key for provider '" + prov + "'. Set via: ai --set api <key> " + prov);
        return;
    }

    auto custom_endpoint = cm.get_custom_endpoint(prov);
    auto provider = ProviderFactory::create(prov, custom_endpoint);
    if (!provider) { term::print_error("Unsupported provider: " + prov); return; }

    std::cout << term::colorize("Fetching models from " + prov + " API...", term::Color::Dim) << "\n";
    auto models = provider->list_models(http_client, api_key);
    if (models.empty()) {
        term::print_error("Could not retrieve models or no models returned for provider '" + prov + "'.");
        return;
    }

    std::string current_def = cm.get_default_model(prov);
    std::cout << term::colorize("Available models for provider '" + prov + "':", term::Color::Bold) << "\n";
    for (const auto& m : models) {
        if (m == current_def) {
            std::cout << "  " << term::colorize("* " + m + " (current default)", term::Color::Green) << "\n";
        } else {
            std::cout << "  - " << m << "\n";
        }
    }
    std::cout << "\nTo change default model: " << term::colorize("ai --set model <name> " + prov, term::Color::Cyan) << "\n";
}

void QueryRunner::list_supported_providers(const ConfigManager& cm) {
    auto providers = ProviderFactory::get_supported_providers();
    std::string current_def = cm.get_default_provider();
    std::cout << term::colorize("Supported LLM Providers:", term::Color::Bold) << "\n";
    for (const auto& p : providers) {
        if (p == current_def) {
            std::cout << "  " << term::colorize("* " + p + " (current default)", term::Color::Green) << "\n";
        } else {
            std::cout << "  - " << p << "\n";
        }
    }
    std::cout << "\nTo change default provider: " << term::colorize("ai --set provider <name>", term::Color::Cyan) << "\n";
    std::cout << "To list models for a provider: " << term::colorize("ai -p <provider> -m", term::Color::Cyan) << "\n";
}

} // namespace ai
