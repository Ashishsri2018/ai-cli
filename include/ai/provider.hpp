#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include "ai/types.hpp"
#include "ai/session.hpp"
#include "ai/json.hpp"
#include "ai/http_client.hpp"

namespace ai {

class LLMProvider {
public:
    virtual ~LLMProvider() = default;
    virtual std::string get_name() const = 0;
    virtual std::string get_default_model() const = 0;
    virtual std::string get_endpoint(const std::string& model, const std::string& api_key, bool stream) const = 0;
    virtual std::vector<std::string> get_headers(const std::string& api_key) const = 0;
    virtual std::string build_request_body(const ChatSession& session, const RequestOptions& options) const = 0;
    virtual std::string extract_response_text(const std::string& response_json) const = 0;
    virtual void process_stream_chunk(const std::string& raw_chunk, std::string& line_buffer, StreamCallback callback) = 0;
    virtual std::vector<std::string> list_models(IHttpClient& client, const std::string& api_key) = 0;
};

class ProviderFactory {
public:
    static std::unique_ptr<LLMProvider> create(const std::string& name, const std::optional<std::string>& custom_endpoint = std::nullopt);
    static std::vector<std::string> get_supported_providers();
};

} // namespace ai
