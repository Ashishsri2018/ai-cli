#pragma once

#include <string>
#include <memory>
#include "ai/config.hpp"
#include "ai/session.hpp"
#include "ai/provider.hpp"
#include "ai/http_client.hpp"

namespace ai {

class ReplSession {
public:
    ReplSession(ConfigManager& config_mgr, IHttpClient& http_client, RequestOptions options);
    void run();

private:
    bool handle_command(const std::string& input);
    void send_turn(const std::string& user_input);
    bool check_and_update_provider();

    ConfigManager& config_mgr_;
    IHttpClient& http_client_;
    RequestOptions options_;
    std::string current_provider_name_;
    std::unique_ptr<LLMProvider> provider_;
    ChatSession session_;
    UsageInfo session_total_usage_;
    UsageInfo last_turn_usage_;
};

} // namespace ai
