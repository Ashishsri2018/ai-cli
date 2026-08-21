#include "ai/query_runner.hpp"
#include "ai/session.hpp"
#include "ai/provider.hpp"
#include "ai/terminal.hpp"
#include "ai/utils.hpp"
#include <iostream>

namespace ai {

void QueryRunner::run_single_query(ConfigManager& cm, IHttpClient& http_client, const CliArgs& args) {
    std::string prov_name = args.provider.empty() ? cm.get_default_provider() : args.provider;
    prov_name = ConfigManager::normalize_provider(prov_name);

    auto api_key_opt = cm.get_api_key(prov_name);
    std::string api_key = api_key_opt.value_or("");
    if (api_key.empty() && prov_name != "ollama") {
        term::print_error("No API key for provider '" + prov_name + "'. Set via: ai --set api <key> " + prov_name);
        return;
    }

    auto custom_endpoint = cm.get_custom_endpoint(prov_name);
    auto provider = ProviderFactory::create(prov_name, custom_endpoint);
    if (!provider) { term::print_error("Unsupported provider: " + prov_name); return; }

    std::string model = args.model.empty() ? cm.get_default_model(prov_name) : args.model;
    ChatSession session(args.system_prompt);

    std::string full_query = args.query;
    if (!args.stdin_content.empty()) {
        full_query = full_query.empty() ? args.stdin_content : args.stdin_content + "\n\n" + full_query;
    }
    session.add_user_message(full_query);

    RequestOptions opt;
    opt.model = model;
    opt.system_prompt = args.system_prompt;
    opt.temperature = args.temperature;
    opt.stream = args.stream;

    std::string url = provider->get_endpoint(model, api_key, opt.stream);
    auto headers = provider->get_headers(api_key);
    std::string body = provider->build_request_body(session, opt);

    if (opt.stream) {
        std::string buffer;
        HttpResponse resp = http_client.post_stream(url, headers, body, [&](const std::string& raw) {
            provider->process_stream_chunk(raw, buffer, [&](const std::string& tok) {
                std::cout << tok;
                std::cout.flush();
            });
        });
        std::cout << "\n";
        if (!resp.success) {
            term::print_error("Request failed: " + (!resp.error.empty() ? resp.error : ("HTTP " + std::to_string(resp.status_code) + " - " + resp.body)));
        }
    } else {
        HttpResponse resp = http_client.post(url, headers, body);
        if (!resp.success) {
            term::print_error("Request failed: " + (!resp.error.empty() ? resp.error : ("HTTP " + std::to_string(resp.status_code) + " - " + resp.body)));
            return;
        }
        std::cout << provider->extract_response_text(resp.body) << "\n";
    }
}

} // namespace ai
