#pragma once

#include "ai/provider.hpp"

namespace ai {

class OpenAIProvider : public LLMProvider {
public:
    explicit OpenAIProvider(std::string name = "openai",
                            std::string default_model = "gpt-4o-mini",
                            std::string base_url = "https://api.openai.com/v1");

    std::string get_name() const override { return name_; }
    std::string get_default_model() const override { return default_model_; }
    std::string get_endpoint(const std::string& model, const std::string& api_key, bool stream) const override;
    std::vector<std::string> get_headers(const std::string& api_key) const override;
    std::string build_request_body(const ChatSession& session, const RequestOptions& options) const override;
    std::string extract_response_text(const std::string& response_json) const override;
    void process_stream_chunk(const std::string& raw_chunk, std::string& line_buffer, StreamCallback callback) override;
    std::vector<std::string> list_models(IHttpClient& client, const std::string& api_key) override;

private:
    std::string name_;
    std::string default_model_;
    std::string base_url_;
};

} // namespace ai
