#include "ai/query_runner.hpp"
#include "ai/provider.hpp"
#include "ai/terminal.hpp"
#include <iostream>

namespace ai {

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
