#include "test_runner.hpp"
#include "ai/provider.hpp"
#include "ai/session.hpp"

using namespace ai;

class MockHttpClient : public IHttpClient {
public:
    HttpResponse mock_resp;
    HttpResponse get(const std::string&, const std::vector<std::string>&, int) override {
        return mock_resp;
    }
    HttpResponse post(const std::string&, const std::vector<std::string>&, const std::string&, int) override {
        return mock_resp;
    }
    HttpResponse post_stream(const std::string&, const std::vector<std::string>&, const std::string&, StreamChunkCallback, int) override {
        return mock_resp;
    }
};

AI_TEST(ProviderFactoryCreation) {
    auto p_gemini = ProviderFactory::create("google");
    ASSERT_STREQ(p_gemini->get_name().c_str(), "google");
    ASSERT_STREQ(p_gemini->get_default_model().c_str(), "gemini-3.6-flash");

    auto p_openai = ProviderFactory::create("openai");
    ASSERT_STREQ(p_openai->get_name().c_str(), "openai");
    ASSERT_STREQ(p_openai->get_default_model().c_str(), "gpt-4o-mini");

    auto p_anthropic = ProviderFactory::create("anthropic");
    ASSERT_STREQ(p_anthropic->get_name().c_str(), "anthropic");
    ASSERT_STREQ(p_anthropic->get_default_model().c_str(), "claude-3-5-haiku-20241022");
}

AI_TEST(GeminiPayloadAndExtraction) {
    auto p = ProviderFactory::create("google");
    ChatSession session("Be concise.");
    session.add_user_message("What is 2+2?");

    RequestOptions opt;
    opt.model = "gemini-3.6-flash";
    std::string body = p->build_request_body(session, opt);

    std::string err;
    Json root = Json::parse(body, err);
    ASSERT_TRUE(err.empty());
    ASSERT_TRUE(root.has("contents"));
    ASSERT_TRUE(root.has("systemInstruction"));

    std::string mock_resp = R"({"candidates":[{"content":{"parts":[{"text":"4"}],"role":"model"}}]})";
    ASSERT_STREQ(p->extract_response_text(mock_resp).c_str(), "4");
}

AI_TEST(GeminiListModelsParsing) {
    auto p = ProviderFactory::create("google");
    MockHttpClient mock;
    mock.mock_resp.success = true;
    mock.mock_resp.status_code = 200;
    mock.mock_resp.body = R"({
        "models": [
            {"name": "models/gemini-3.6-flash", "supportedGenerationMethods": ["generateContent"]},
            {"name": "models/gemini-1.5-pro", "supportedGenerationMethods": ["generateContent"]},
            {"name": "models/embedding-001", "supportedGenerationMethods": ["embedContent"]}
        ]
    })";

    auto models = p->list_models(mock, "dummy_key");
    ASSERT_EQ(models.size(), 2);
    ASSERT_STREQ(models[0].c_str(), "gemini-3.6-flash");
    ASSERT_STREQ(models[1].c_str(), "gemini-1.5-pro");
}

AI_TEST(OpenAIListModelsParsing) {
    auto p = ProviderFactory::create("openai");
    MockHttpClient mock;
    mock.mock_resp.success = true;
    mock.mock_resp.status_code = 200;
    mock.mock_resp.body = R"({
        "data": [
            {"id": "gpt-4o"},
            {"id": "gpt-4o-mini"}
        ]
    })";

    auto models = p->list_models(mock, "dummy_key");
    ASSERT_EQ(models.size(), 2);
    ASSERT_STREQ(models[0].c_str(), "gpt-4o");
    ASSERT_STREQ(models[1].c_str(), "gpt-4o-mini");
}

AI_TEST(AnthropicResponseParsing) {
    auto p = ProviderFactory::create("anthropic");

    // Test full (non-streaming) response extraction
    std::string full_resp = R"({"content":[{"type":"text","text":"Hello world"}],"model":"claude-3-5-haiku-20241022"})";
    ASSERT_STREQ(p->extract_response_text(full_resp).c_str(), "Hello world");

    // Test stream content_block_delta extraction
    std::string stream_delta = R"({"type":"content_block_delta","delta":{"type":"text_delta","text":"streaming"}})";
    ASSERT_STREQ(p->extract_response_text(stream_delta).c_str(), "streaming");

    // Test empty/irrelevant stream event returns empty
    std::string ping_event = R"({"type":"ping"})";
    ASSERT_STREQ(p->extract_response_text(ping_event).c_str(), "");
}

AI_TEST(AnthropicStreamChunkProcessing) {
    auto p = ProviderFactory::create("anthropic");
    std::string buffer;
    std::string collected;

    auto cb = [&](const std::string& token) { collected += token; };

    // Simulate SSE chunks arriving
    std::string sse_data = "data: {\"type\":\"content_block_delta\",\"delta\":{\"type\":\"text_delta\",\"text\":\"Hi\"}}\n\n"
                           "data: {\"type\":\"content_block_delta\",\"delta\":{\"type\":\"text_delta\",\"text\":\" there\"}}\n\n"
                           "data: [DONE]\n\n";
    p->process_stream_chunk(sse_data, buffer, cb);
    ASSERT_STREQ(collected.c_str(), "Hi there");
}
