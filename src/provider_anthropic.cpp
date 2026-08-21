#include "ai/provider_anthropic.hpp"
#include "ai/utils.hpp"

namespace ai {

std::string AnthropicProvider::get_endpoint(const std::string&, const std::string&, bool) const {
    return "https://api.anthropic.com/v1/messages";
}

std::vector<std::string> AnthropicProvider::get_headers(const std::string& api_key) const {
    return {
        "x-api-key: " + api_key,
        "anthropic-version: 2023-06-01",
        "Content-Type: application/json"
    };
}

std::string AnthropicProvider::build_request_body(const ChatSession& session, const RequestOptions& options) const {
    Json root = Json::object();
    root["model"] = options.model.empty() ? get_default_model() : options.model;
    root["max_tokens"] = options.max_tokens > 0 ? options.max_tokens : 4096;
    root["temperature"] = options.temperature;
    root["stream"] = options.stream;

    std::string sys = !options.system_prompt.empty() ? options.system_prompt : session.get_system_prompt();
    if (!sys.empty()) {
        root["system"] = sys;
    }

    Json messages = Json::array();
    for (const auto& msg : session.get_messages()) {
        // Anthropic forbids system role in messages array; skip if leaked
        if (msg.role == Role::System) continue;
        Json m = Json::object();
        m["role"] = (msg.role == Role::Assistant) ? "assistant" : "user";
        m["content"] = msg.content;
        messages.push_back(m);
    }
    root["messages"] = messages;
    return root.dump();
}

std::string AnthropicProvider::extract_response_text(const std::string& response_json) const {
    std::string err;
    Json root = Json::parse(response_json, err);
    if (!err.empty() || !root.is_object()) return "";

    // Stream event: content_block_delta
    if (root.get("type").as_string() == "content_block_delta") {
        const auto& delta = root.get("delta");
        if (delta.get("type").as_string() == "text_delta") {
            return delta.get("text").as_string();
        }
    }
    // Full response
    const auto& content = root.get("content");
    if (content.is_array() && content.size() > 0) {
        return content[0].get("text").as_string();
    }
    return "";
}

void AnthropicProvider::process_stream_chunk(const std::string& raw_chunk, std::string& line_buffer, StreamCallback callback) {
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
    }
}

std::vector<std::string> AnthropicProvider::list_models(IHttpClient& client, const std::string& api_key) {
    std::vector<std::string> models;
    std::string url = "https://api.anthropic.com/v1/models";
    auto headers = get_headers(api_key);
    HttpResponse resp = client.get(url, headers);
    if (resp.success) {
        std::string err;
        Json root = Json::parse(resp.body, err);
        if (err.empty() && root.is_object() && root.has("data")) {
            const auto& arr = root.get("data");
            if (arr.is_array()) {
                for (size_t i = 0; i < arr.size(); ++i) {
                    std::string id = arr[i].get("id").as_string();
                    if (!id.empty()) models.push_back(id);
                }
            }
        }
    }
    if (models.empty()) {
        models = {"claude-3-7-sonnet-20250219", "claude-3-5-sonnet-20241022", "claude-3-5-haiku-20241022", "claude-3-opus-20240229"};
    }
    return models;
}

} // namespace ai
