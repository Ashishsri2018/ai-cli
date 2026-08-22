#pragma once

#include "ai/provider.hpp"

namespace ai {

class GoogleGeminiProvider : public LLMProvider {
public:
    std::string get_name() const override { return "google"; }
    std::string get_default_model() const override { return "gemini-3.6-flash"; }
    std::string get_endpoint(const std::string& model, const std::string& api_key, bool stream) const override;
    std::vector<std::string> get_headers(const std::string& api_key) const override;
    std::string build_request_body(const ChatSession& session, const RequestOptions& options) const override;
    std::string extract_response_text(const std::string& response_json) const override;
    void process_stream_chunk(const std::string& raw_chunk, std::string& line_buffer, StreamCallback callback) override;
    UsageInfo extract_usage(const std::string& response_json) const override;
    void process_stream_chunk(const std::string& raw_chunk, std::string& line_buffer, StreamCallback callback, UsageCallback usage_callback = nullptr) override;
    std::vector<std::string> list_models(IHttpClient& client, const std::string& api_key) override;
};

} // namespace ai
