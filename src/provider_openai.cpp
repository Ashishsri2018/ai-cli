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

void OpenAIProvider::process_stream_chunk(const std::string& raw_chunk, std::string& line_buffer, StreamCallback callback) {
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

} // namespace ai
