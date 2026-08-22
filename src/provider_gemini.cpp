#include "ai/provider_gemini.hpp"
#include "ai/utils.hpp"

namespace ai {

std::string GoogleGeminiProvider::get_endpoint(const std::string& model, const std::string& key, bool stream) const {
    std::string m = model.empty() ? get_default_model() : model;
    std::string action = stream ? "streamGenerateContent?alt=sse&key=" : "generateContent?key=";
    return "https://generativelanguage.googleapis.com/v1beta/models/" + m + ":" + action + key;
}

std::vector<std::string> GoogleGeminiProvider::get_headers(const std::string&) const {
    return {"Content-Type: application/json"};
}

std::string GoogleGeminiProvider::build_request_body(const ChatSession& session, const RequestOptions& options) const {
    Json root = Json::object();
    Json contents = Json::array();

    for (const auto& msg : session.get_messages()) {
        Json turn = Json::object();
        turn["role"] = (msg.role == Role::Assistant) ? "model" : "user";
        Json parts = Json::array();
        Json part = Json::object();
        part["text"] = msg.content;
        parts.push_back(part);
        turn["parts"] = parts;
        contents.push_back(turn);
    }
    root["contents"] = contents;

    std::string sys = !options.system_prompt.empty() ? options.system_prompt : session.get_system_prompt();
    if (!sys.empty()) {
        Json sys_obj = Json::object();
        Json parts = Json::array();
        Json p = Json::object();
        p["text"] = sys;
        parts.push_back(p);
        sys_obj["parts"] = parts;
        root["systemInstruction"] = sys_obj;
    }

    Json gen_config = Json::object();
    gen_config["temperature"] = options.temperature;
    root["generationConfig"] = gen_config;
    return root.dump();
}

std::string GoogleGeminiProvider::extract_response_text(const std::string& response_json) const {
    std::string err;
    Json root = Json::parse(response_json, err);
    if (!err.empty() || !root.is_object()) return "";
    const auto& cand = root.get("candidates");
    if (cand.is_array() && cand.size() > 0) {
        const auto& parts = cand[0].get("content").get("parts");
        if (parts.is_array() && parts.size() > 0) {
            return parts[0].get("text").as_string();
        }
    }
    return "";
}

UsageInfo GoogleGeminiProvider::extract_usage(const std::string& response_json) const {
    UsageInfo usage;
    std::string err;
    Json root = Json::parse(response_json, err);
    if (!err.empty() || !root.is_object()) return usage;
    const auto& meta = root.get("usageMetadata");
    if (meta.is_object()) {
        usage.prompt_tokens = meta.get("promptTokenCount").as_int();
        usage.completion_tokens = meta.get("candidatesTokenCount").as_int();
        usage.total_tokens = meta.get("totalTokenCount").as_int();
        usage.cached_tokens = meta.get("cachedContentTokenCount").as_int();
        if (usage.total_tokens == 0 && (usage.prompt_tokens > 0 || usage.completion_tokens > 0)) {
            usage.total_tokens = usage.prompt_tokens + usage.completion_tokens;
        }
        usage.has_usage = true;
    }
    return usage;
}

void GoogleGeminiProvider::process_stream_chunk(const std::string& raw_chunk, std::string& line_buffer, StreamCallback callback, UsageCallback usage_callback) {
    line_buffer += raw_chunk;
    size_t pos = 0;
    while ((pos = line_buffer.find('\n')) != std::string::npos) {
        std::string line = line_buffer.substr(0, pos);
        line_buffer.erase(0, pos + 1);
        line = utils::trim(line);
        if (line.empty() || !utils::starts_with(line, "data:")) continue;
        std::string data = utils::trim(line.substr(5));
        if (data == "[DONE]") continue;

        std::string text = extract_response_text(data);
        if (!text.empty() && callback) {
            callback(text);
        }

        if (usage_callback) {
            UsageInfo usage = extract_usage(data);
            if (usage.has_usage) {
                usage_callback(usage);
            }
        }
    }
}

std::vector<std::string> GoogleGeminiProvider::list_models(IHttpClient& client, const std::string& api_key) {
    std::vector<std::string> models;
    std::string url = "https://generativelanguage.googleapis.com/v1beta/models?key=" + api_key;
    HttpResponse resp = client.get(url, {"Content-Type: application/json"});
    if (!resp.success) return models;

    std::string err;
    Json root = Json::parse(resp.body, err);
    if (!err.empty() || !root.is_object()) return models;

    const auto& arr = root.get("models");
    if (!arr.is_array()) return models;

    for (size_t i = 0; i < arr.size(); ++i) {
        std::string name = arr[i].get("name").as_string();
        if (utils::starts_with(name, "models/")) name = name.substr(7);

        const auto& methods = arr[i].get("supportedGenerationMethods");
        bool supports_gen = false;
        if (methods.is_array()) {
            for (size_t j = 0; j < methods.size(); ++j) {
                if (methods[j].as_string() == "generateContent") { supports_gen = true; break; }
            }
        } else {
            supports_gen = true;
        }
        if (supports_gen && !name.empty()) models.push_back(name);
    }
    return models;
}

QuotaInfo GoogleGeminiProvider::check_quota(IHttpClient& client, const std::string& api_key) {
    QuotaInfo q;
    q.provider = "google";
    q.console_url = "https://aistudio.google.com/app/plan_information";

    if (api_key.empty()) {
        q.success = false;
        q.error_message = "No API key configured for Google Gemini.";
        return q;
    }

    std::string url = "https://generativelanguage.googleapis.com/v1beta/models?key=" + api_key;
    HttpResponse resp = client.get(url, {"Content-Type: application/json"});
    if (resp.success && resp.status_code == 200) {
        q.success = true;
        q.status = "Active";
        q.info_message = "Standard API key verified. Gemini uses rate-limit quotas (e.g. 15 RPM / 1M TPM on Free Tier, pay-as-you-go on Blaze).";
    } else {
        q.success = false;
        q.status = "Error / Inactive";
        q.error_message = !resp.error.empty() ? resp.error : ("HTTP " + std::to_string(resp.status_code) + " - " + resp.body);
    }
    return q;
}

} // namespace ai
