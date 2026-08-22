#include "ai/repl.hpp"
#include "ai/terminal.hpp"
#include "ai/utils.hpp"
#include <iostream>

namespace ai {

ReplSession::ReplSession(ConfigManager& config_mgr, IHttpClient& http_client, RequestOptions options)
    : config_mgr_(config_mgr), http_client_(http_client), options_(std::move(options)) {
    if (!options_.system_prompt.empty()) {
        session_.set_system_prompt(options_.system_prompt);
    }
}

bool ReplSession::check_and_update_provider() {
    std::string prov = current_provider_name_.empty() ? config_mgr_.get_default_provider() : current_provider_name_;
    current_provider_name_ = ConfigManager::normalize_provider(prov);

    auto custom_endpoint = config_mgr_.get_custom_endpoint(current_provider_name_);
    provider_ = ProviderFactory::create(current_provider_name_, custom_endpoint);
    if (!provider_) {
        term::print_error("Unsupported provider: " + current_provider_name_);
        return false;
    }
    if (options_.model.empty()) {
        options_.model = config_mgr_.get_default_model(current_provider_name_);
    }
    return true;
}

bool ReplSession::handle_command(const std::string& input) {
    std::string cmd = utils::trim(input);
    if (cmd == "quit" || cmd == "exit" || cmd == "/quit" || cmd == "/exit") return false;

    if (cmd == "/clear") {
        session_.clear();
        term::print_success("Conversation history cleared.");
    } else if (utils::starts_with(cmd, "/model")) {
        auto parts = utils::split(cmd, ' ');
        if (parts.size() > 1 && !parts[1].empty()) {
            options_.model = parts[1];
            term::print_success("Switched model to: " + options_.model);
        } else {
            term::print_info("Current model: " + options_.model);
        }
    } else if (utils::starts_with(cmd, "/provider")) {
        auto parts = utils::split(cmd, ' ');
        if (parts.size() > 1 && !parts[1].empty()) {
            current_provider_name_ = parts[1];
            options_.model = "";
            check_and_update_provider();
            term::print_success("Switched provider to: " + current_provider_name_ + " (model: " + options_.model + ")");
        } else {
            term::print_info("Current provider: " + current_provider_name_);
        }
    } else if (utils::starts_with(cmd, "/system")) {
        std::string prompt = utils::trim(cmd.substr(7));
        session_.set_system_prompt(prompt);
        term::print_success("Updated system prompt.");
    } else if (cmd == "/models") {
        auto key = config_mgr_.get_api_key(current_provider_name_).value_or("");
        std::cout << term::colorize("Fetching models for " + current_provider_name_ + "...", term::Color::Dim) << "\n";
        auto models = provider_->list_models(http_client_, key);
        if (models.empty()) term::print_error("No models returned from API.");
        for (const auto& m : models) {
            std::cout << "  " << (m == options_.model ? term::colorize("* " + m + " (active)", term::Color::Green) : "- " + m) << "\n";
        }
    } else if (cmd == "/history") {
        term::print_info("Total turns in history: " + std::to_string(session_.size()));
    } else if (utils::starts_with(cmd, "/usage")) {
        auto parts = utils::split(cmd, ' ');
        if (parts.size() > 1) {
            std::string sub = utils::to_lower(parts[1]);
            if (sub == "on" || sub == "true" || sub == "1") {
                options_.show_usage = true;
                term::print_success("Per-turn token usage display enabled.");
            } else if (sub == "off" || sub == "false" || sub == "0") {
                options_.show_usage = false;
                term::print_success("Per-turn token usage display disabled.");
            } else {
                term::print_error("Usage: /usage [on|off]");
            }
        } else {
            if (session_total_usage_.has_usage) {
                std::cout << term::colorize("Session Total Token Usage:", term::Color::Bold) << "\n";
                term::print_usage(session_total_usage_);
                if (last_turn_usage_.has_usage) {
                    std::cout << term::colorize("Last Turn Token Usage:", term::Color::Dim) << "\n";
                    term::print_usage(last_turn_usage_);
                }
            } else {
                term::print_info("No token usage recorded yet for this session.");
            }
        }
    } else if (cmd == "/quota" || cmd == "/balance") {
        auto key = config_mgr_.get_api_key(current_provider_name_).value_or("");
        std::cout << term::colorize("Checking quota / balance for " + current_provider_name_ + "...", term::Color::Dim) << "\n";
        QuotaInfo q = provider_->check_quota(http_client_, key);
        term::print_quota(q);
    } else if (cmd == "/help") {
        std::cout << "Commands: /clear, /models, /model <name>, /provider <name>, /system <prompt>, /usage [on|off], /quota, /history, /help, quit\n";
    } else {
        term::print_error("Unknown command. Type /help for available commands.");
    }
    return true;
}

void ReplSession::send_turn(const std::string& user_input) {
    auto api_key_opt = config_mgr_.get_api_key(current_provider_name_);
    std::string api_key = api_key_opt.value_or("");
    if (api_key.empty() && current_provider_name_ != "ollama") {
        term::print_error("No API key configured for provider '" + current_provider_name_ + "'. Set via: ai --set api <key> " + current_provider_name_);
        return;
    }

    session_.add_user_message(user_input);
    std::string url = provider_->get_endpoint(options_.model, api_key, options_.stream);
    auto headers = provider_->get_headers(api_key);
    std::string body = provider_->build_request_body(session_, options_);

    std::string assistant_reply;
    std::string sse_buffer;
    UsageInfo turn_usage;

    if (options_.stream) {
        auto on_chunk = [&](const std::string& raw) {
            provider_->process_stream_chunk(raw, sse_buffer, [&](const std::string& token) {
                std::cout << token;
                std::cout.flush();
                assistant_reply += token;
            }, [&](const UsageInfo& u) {
                turn_usage = u;
            });
        };
        HttpResponse resp = http_client_.post_stream(url, headers, body, on_chunk);
        std::cout << "\n\n";
        if (!resp.success) {
            term::print_error("Request failed: " + (!resp.error.empty() ? resp.error : ("HTTP " + std::to_string(resp.status_code) + " - " + resp.body)));
            return;
        }
    } else {
        HttpResponse resp = http_client_.post(url, headers, body);
        if (!resp.success) {
            term::print_error("Request failed: " + (!resp.error.empty() ? resp.error : ("HTTP " + std::to_string(resp.status_code) + " - " + resp.body)));
            return;
        }
        assistant_reply = provider_->extract_response_text(resp.body);
        std::cout << assistant_reply << "\n\n";
        turn_usage = provider_->extract_usage(resp.body);
    }

    if (turn_usage.has_usage) {
        last_turn_usage_ = turn_usage;
        session_total_usage_.prompt_tokens += turn_usage.prompt_tokens;
        session_total_usage_.completion_tokens += turn_usage.completion_tokens;
        session_total_usage_.total_tokens += turn_usage.total_tokens;
        session_total_usage_.cached_tokens += turn_usage.cached_tokens;
        session_total_usage_.has_usage = true;
        if (options_.show_usage) {
            term::print_usage(turn_usage);
            std::cout << "\n";
        }
    }

    if (!assistant_reply.empty()) {
        session_.add_assistant_message(assistant_reply);
    }
}

void ReplSession::run() {
    if (!check_and_update_provider()) return;
    term::print_banner(current_provider_name_, options_.model);

    std::string input;
    while (true) {
        std::cout << term::colorize("> ", term::Color::Cyan);
        if (!std::getline(std::cin, input)) break;
        input = utils::trim(input);
        if (input.empty()) continue;

        if (input[0] == '/' || input == "quit" || input == "exit") {
            if (!handle_command(input)) break;
            continue;
        }
        send_turn(input);
    }
    std::cout << term::colorize("Goodbye!\n", term::Color::Dim);
}

} // namespace ai
