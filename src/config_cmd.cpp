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
    std::cout << "Default System Prompt: " << (cfg.system_prompt.empty() ? term::colorize("(none)", term::Color::Dim) : term::colorize("\"" + cfg.system_prompt + "\"", term::Color::Green)) << "\n";
    std::cout << "Default Temperature: " << term::colorize(std::to_string(cfg.temperature), term::Color::Cyan) << "\n";
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
        auto known = ProviderFactory::get_supported_providers();
        auto is_prov = [&](const std::string& s) {
            for (const auto& p : known) if (utils::to_lower(p) == utils::to_lower(s)) return true;
            return false;
        };
        if (is_prov(a2) && !is_prov(a1)) { key = a1; prov = a2; }
        else if (is_prov(a1) && !is_prov(a2)) { prov = a1; key = a2; }
        else if (a1.find("sk-") == 0 || a1.find("AIza") == 0 || a1.size() > 20) { key = a1; prov = a2; }
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
    } else if ((sub == "system" || sub == "prompt") && !args.config_args.empty()) {
        cm.set_system_prompt(args.config_args[0]); cm.save();
        term::print_success("Updated default system prompt to: \"" + args.config_args[0] + "\"");
    } else if ((sub == "temperature" || sub == "temp") && !args.config_args.empty()) {
        cm.set_temperature(std::stod(args.config_args[0])); cm.save();
        term::print_success("Updated default temperature to " + args.config_args[0]);
    } else {
        term::print_error("Usage: ai --set api <key> <provider> | ai --set model <name> [provider] | ai --set provider <name> | ai --set system <prompt> | ai --set temp <val>");
    }
}

void handle_del_config(ConfigManager& cm, const CliArgs& args) {
    std::string sub = args.config_subcommand;
    if (sub == "api" && !args.config_args.empty()) {
        if (cm.delete_api_key(args.config_args[0])) {
            cm.save(); term::print_success("Deleted API key for provider '" + args.config_args[0] + "'");
        } else term::print_error("No API key found for provider '" + args.config_args[0] + "'");
    } else if (sub == "system" || sub == "prompt") {
        cm.set_system_prompt(""); cm.save();
        term::print_success("Cleared default system prompt.");
    } else {
        term::print_error("Usage: ai --del api <provider> | ai --del system");
    }
}

} // namespace

void QueryRunner::handle_config_command(ConfigManager& cm, const CliArgs& args) {
    if (args.mode == CliMode::ListConfig) {
        handle_list_config(cm);
    } else if (args.mode == CliMode::SetConfig) {
        handle_set_config(cm, args);
    } else if (args.mode == CliMode::DelConfig) {
        handle_del_config(cm, args);
    } else if (args.mode == CliMode::ShowSystemPrompt) {
        std::string sp = cm.get_system_prompt();
        if (sp.empty()) std::cout << "Current System Prompt: " << term::colorize("(none)", term::Color::Dim) << "\n";
        else std::cout << "Current System Prompt: " << term::colorize("\"" + sp + "\"", term::Color::Green) << "\n";
    } else if (args.mode == CliMode::SetSystemPrompt) {
        cm.set_system_prompt(args.system_prompt);
        cm.save();
        term::print_success("Saved default system prompt: \"" + args.system_prompt + "\"");
    } else if (args.mode == CliMode::ShowTemperature) {
        std::cout << "Current Temperature: " << term::colorize(std::to_string(cm.get_temperature()), term::Color::Cyan) << "\n";
    } else if (args.mode == CliMode::SetTemperature) {
        cm.set_temperature(args.temperature);
        cm.save();
        term::print_success("Saved default temperature: " + std::to_string(args.temperature));
    }
}

} // namespace ai
