#include "ai/provider_openai.hpp"
#include "ai/utils.hpp"

namespace ai {

OpenAIProvider::OpenAIProvider(std::string name, std::string default_model, std::string base_url)
    : name_(std::move(name)), default_model_(std::move(default_model)), base_url_(std::move(base_url)) {}

std::string OpenAIProvider::get_endpoint(const std::string&, const std::string&, bool) const {
    if (utils::ends_with(base_url_, "/chat/completions")) return base_url_;
    if (utils::ends_with(base_url_, "/")) return base_url_ + "chat/completions";
    return base_url_ + "/chat/completions";
}

std::vector<std::string> OpenAIProvider::get_headers(const std::string& api_key) const {
    std::vector<std::string> headers = {"Content-Type: application/json"};
    if (!api_key.empty()) {
        headers.push_back("Authorization: Bearer " + api_key);
    }
    return headers;
}

std::string OpenAIProvider::build_request_body(const ChatSession& session, const RequestOptions& options) const {
    Json root = Json::object();
    root["model"] = options.model.empty() ? default_model_ : options.model;
    root["temperature"] = options.temperature;
    root["stream"] = options.stream;

    if (options.stream) {
        Json stream_options = Json::object();
        stream_options["include_usage"] = true;
        root["stream_options"] = stream_options;
    }

    Json messages = Json::array();
    std::string sys = !options.system_prompt.empty() ? options.system_prompt : session.get_system_prompt();
    if (!sys.empty()) {
        Json s_msg = Json::object();
        s_msg["role"] = "system";
        s_msg["content"] = sys;
        messages.push_back(s_msg);
    }
    for (const auto& msg : session.get_messages()) {
        Json m = Json::object();
        m["role"] = role_to_string(msg.role);
        m["content"] = msg.content;
        messages.push_back(m);
    }
    root["messages"] = messages;
    return root.dump();
}

std::string OpenAIProvider::extract_response_text(const std::string& response_json) const {
    std::string err;
    Json root = Json::parse(response_json, err);
    if (!err.empty() || !root.is_object()) return "";
    const auto& choices = root.get("choices");
    if (!choices.is_array() || choices.size() == 0) return "";
    const auto& choice = choices[0];
    if (choice.has("message") && choice.get("message").has("content")) {
        return choice.get("message").get("content").as_string();
    }
    if (choice.has("delta") && choice.get("delta").has("content")) {
        return choice.get("delta").get("content").as_string();
    }
    return "";
}

UsageInfo OpenAIProvider::extract_usage(const std::string& response_json) const {
    UsageInfo usage;
    std::string err;
    Json root = Json::parse(response_json, err);
    if (!err.empty() || !root.is_object()) return usage;
    const auto& u = root.get("usage");
    if (u.is_object()) {
        usage.prompt_tokens = u.get("prompt_tokens").as_int();
        usage.completion_tokens = u.get("completion_tokens").as_int();
        usage.total_tokens = u.get("total_tokens").as_int();
        if (usage.total_tokens == 0 && (usage.prompt_tokens > 0 || usage.completion_tokens > 0)) {
            usage.total_tokens = usage.prompt_tokens + usage.completion_tokens;
        }
        const auto& details = u.get("prompt_tokens_details");
        if (details.is_object()) {
            usage.cached_tokens = details.get("cached_tokens").as_int();
        }
        usage.has_usage = true;
    }
    return usage;
}

void OpenAIProvider::process_stream_chunk(const std::string& raw_chunk, std::string& line_buffer, StreamCallback callback, UsageCallback usage_callback) {
    line_buffer += raw_chunk;
    size_t pos = 0;
    while ((pos = line_buffer.find('\n')) != std::string::npos) {
        std::string line = line_buffer.substr(0, pos);
        line_buffer.erase(0, pos + 1);
        line = utils::trim(line);
        if (line.empty() || !utils::starts_with(line, "data:")) continue;
        std::string data = utils::trim(line.substr(5));
        if (data == "[DONE]") continue;

        std::string token = extract_response_text(data);
        if (!token.empty() && callback) {
            callback(token);
        }

        if (usage_callback) {
            UsageInfo usage = extract_usage(data);
            if (usage.has_usage) {
                usage_callback(usage);
            }
        }
    }
}

std::vector<std::string> OpenAIProvider::list_models(IHttpClient& client, const std::string& api_key) {
    std::vector<std::string> models;
    std::string base = base_url_;
    if (utils::ends_with(base, "/chat/completions")) {
        base = base.substr(0, base.size() - 17);
    }
    if (utils::ends_with(base, "/")) base.pop_back();
    std::string url = base + "/models";

    auto headers = get_headers(api_key);
    HttpResponse resp = client.get(url, headers);
    if (!resp.success) return models;

    std::string err;
    Json root = Json::parse(resp.body, err);
    if (!err.empty() || !root.is_object()) return models;

    const auto& arr = root.get("data");
    if (!arr.is_array()) return models;

    for (size_t i = 0; i < arr.size(); ++i) {
        std::string id = arr[i].get("id").as_string();
        if (!id.empty()) models.push_back(id);
    }
    return models;
}

QuotaInfo OpenAIProvider::check_quota(IHttpClient& client, const std::string& api_key) {
    QuotaInfo q;
    q.provider = name_;

    if (name_ == "ollama") {
        q.supported = true;
        q.success = true;
        q.status = "Local";
        q.info_message = "Local model runner (no API quota or credit limit).";
        return q;
    }

    if (api_key.empty()) {
        q.success = false;
        q.error_message = "No API key configured for provider '" + name_ + "'.";
        return q;
    }

    auto headers = get_headers(api_key);

    if (name_ == "deepseek") {
        q.console_url = "https://platform.deepseek.com/top_up";
        std::string url = "https://api.deepseek.com/user/balance";
        HttpResponse resp = client.get(url, headers);
        if (resp.success) {
            std::string err;
            Json root = Json::parse(resp.body, err);
            if (err.empty() && root.is_object()) {
                q.supported = true;
                q.success = true;
                bool is_avail = root.get("is_available").as_bool(true);
                q.status = is_avail ? "Active" : "Unavailable / Low Balance";
                const auto& arr = root.get("balance_infos");
                if (arr.is_array() && arr.size() > 0) {
                    const auto& info = arr[0];
                    q.currency = info.get("currency").as_string("USD");
                    q.total_balance = info.get("total_balance").as_string();
                    q.granted_balance = info.get("granted_balance").as_string();
                    q.topped_up_balance = info.get("topped_up_balance").as_string();
                }
                return q;
            }
        }
        q.success = false;
        q.error_message = !resp.error.empty() ? resp.error : ("HTTP " + std::to_string(resp.status_code) + " - " + resp.body);
        return q;
    }

    if (name_ == "openrouter" || base_url_.find("openrouter.ai") != std::string::npos) {
        q.console_url = "https://openrouter.ai/credits";
        std::string url = "https://openrouter.ai/api/v1/credits";
        HttpResponse resp = client.get(url, headers);
        if (resp.success) {
            std::string err;
            Json root = Json::parse(resp.body, err);
            if (err.empty() && root.is_object() && root.has("data")) {
                q.supported = true;
                q.success = true;
                q.status = "Active";
                q.currency = "USD";
                const auto& data = root.get("data");
                double credits = data.get("total_credits").as_number(0.0);
                double usage = data.get("total_usage").as_number(0.0);
                double remaining = credits - usage;
                std::ostringstream ss_bal, ss_use;
                ss_bal << std::fixed << std::setprecision(4) << remaining;
                ss_use << std::fixed << std::setprecision(4) << usage;
                q.total_balance = ss_bal.str();
                q.total_usage = ss_use.str();
                return q;
            }
        }
        q.success = false;
        q.error_message = !resp.error.empty() ? resp.error : ("HTTP " + std::to_string(resp.status_code) + " - " + resp.body);
        return q;
    }

    if (name_ == "groq") {
        q.console_url = "https://console.groq.com/settings/limits";
        auto models = list_models(client, api_key);
        if (!models.empty()) {
            q.success = true;
            q.status = "Active";
            q.info_message = "Standard API key verified. Groq manages rate limits (RPM/TPM) per tier in the Groq console.";
        } else {
            q.success = false;
            q.error_message = "Authentication or connection failed for Groq.";
        }
        return q;
    }

    // Default OpenAI & custom endpoints
    q.console_url = "https://platform.openai.com/usage";
    auto models = list_models(client, api_key);
    if (!models.empty()) {
        q.success = true;
        q.status = "Active";
        q.info_message = "Standard API key verified. OpenAI project keys (sk-...) do not expose billing endpoints. View account balance & costs in the OpenAI usage dashboard.";
    } else {
        q.success = false;
        q.error_message = "Authentication or connection failed for provider '" + name_ + "'.";
    }
    return q;
}

} // namespace ai
