#pragma once

#include "ai/cli.hpp"
#include "ai/config.hpp"
#include "ai/http_client.hpp"

namespace ai {

class QueryRunner {
public:
    static void run_single_query(ConfigManager& cm, IHttpClient& http_client, const CliArgs& args);
    static void handle_config_command(ConfigManager& cm, const CliArgs& args);
    static void list_provider_models(ConfigManager& cm, IHttpClient& http_client, const std::string& provider_arg);
    static void list_supported_providers(const ConfigManager& cm);
    static void check_provider_quota(ConfigManager& cm, IHttpClient& http_client, const std::string& provider_arg);
};

} // namespace ai
