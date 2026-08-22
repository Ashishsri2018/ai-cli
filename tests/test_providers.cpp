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

    StreamCallback cb = [&](const std::string& token) { collected += token; };

    // Simulate SSE chunks arriving
    std::string sse_data = "data: {\"type\":\"content_block_delta\",\"delta\":{\"type\":\"text_delta\",\"text\":\"Hi\"}}\n\n"
                           "data: {\"type\":\"content_block_delta\",\"delta\":{\"type\":\"text_delta\",\"text\":\" there\"}}\n\n"
                           "data: [DONE]\n\n";
    p->process_stream_chunk(sse_data, buffer, cb);
    ASSERT_STREQ(collected.c_str(), "Hi there");
}

AI_TEST(GeminiUsageExtraction) {
    auto p = ProviderFactory::create("google");

    std::string resp_json = R"({
        "candidates":[{"content":{"parts":[{"text":"Hello"}]}}],
        "usageMetadata":{
            "promptTokenCount":12,
            "candidatesTokenCount":5,
            "totalTokenCount":17,
            "cachedContentTokenCount":4
        }
    })";

    UsageInfo usage = p->extract_usage(resp_json);
    ASSERT_TRUE(usage.has_usage);
    ASSERT_EQ(usage.prompt_tokens, 12);
    ASSERT_EQ(usage.completion_tokens, 5);
    ASSERT_EQ(usage.total_tokens, 17);
    ASSERT_EQ(usage.cached_tokens, 4);

    // Test stream usage callback
    std::string buffer;
    std::string text;
    UsageInfo stream_usage;
    std::string sse = "data: {\"candidates\":[{\"content\":{\"parts\":[{\"text\":\"Hello\"}]}}],\"usageMetadata\":{\"promptTokenCount\":12,\"candidatesTokenCount\":5,\"totalTokenCount\":17}}\n\n";
    p->process_stream_chunk(sse, buffer, [&](const std::string& t) { text += t; }, [&](const UsageInfo& u) { stream_usage = u; });

    ASSERT_STREQ(text.c_str(), "Hello");
    ASSERT_TRUE(stream_usage.has_usage);
    ASSERT_EQ(stream_usage.total_tokens, 17);
}

AI_TEST(OpenAIUsageExtraction) {
    auto p = ProviderFactory::create("openai");

    std::string resp_json = R"({
        "choices":[{"message":{"content":"Hello"}}],
        "usage":{
            "prompt_tokens":10,
            "completion_tokens":20,
            "total_tokens":30,
            "prompt_tokens_details":{"cached_tokens":3}
        }
    })";

    UsageInfo usage = p->extract_usage(resp_json);
    ASSERT_TRUE(usage.has_usage);
    ASSERT_EQ(usage.prompt_tokens, 10);
    ASSERT_EQ(usage.completion_tokens, 20);
    ASSERT_EQ(usage.total_tokens, 30);
    ASSERT_EQ(usage.cached_tokens, 3);

    // Test stream chunk with usage
    std::string buffer;
    std::string text;
    UsageInfo stream_usage;
    std::string sse = "data: {\"choices\":[{\"delta\":{\"content\":\"Hi\"}}]}\n\n"
                      "data: {\"choices\":[],\"usage\":{\"prompt_tokens\":10,\"completion_tokens\":20,\"total_tokens\":30}}\n\n"
                      "data: [DONE]\n\n";
    p->process_stream_chunk(sse, buffer, [&](const std::string& t) { text += t; }, [&](const UsageInfo& u) { stream_usage = u; });

    ASSERT_STREQ(text.c_str(), "Hi");
    ASSERT_TRUE(stream_usage.has_usage);
    ASSERT_EQ(stream_usage.prompt_tokens, 10);
    ASSERT_EQ(stream_usage.completion_tokens, 20);
    ASSERT_EQ(stream_usage.total_tokens, 30);
}

AI_TEST(AnthropicUsageExtraction) {
    auto p = ProviderFactory::create("anthropic");

    std::string resp_json = R"({
        "content":[{"type":"text","text":"Hello"}],
        "usage":{
            "input_tokens":15,
            "output_tokens":35,
            "cache_read_input_tokens":5
        }
    })";

    UsageInfo usage = p->extract_usage(resp_json);
    ASSERT_TRUE(usage.has_usage);
    ASSERT_EQ(usage.prompt_tokens, 15);
    ASSERT_EQ(usage.completion_tokens, 35);
    ASSERT_EQ(usage.total_tokens, 50);
    ASSERT_EQ(usage.cached_tokens, 5);

    // Test stream chunks with message_start and message_delta
    std::string buffer;
    std::string text;
    UsageInfo stream_usage;
    std::string sse = "data: {\"type\":\"message_start\",\"message\":{\"id\":\"msg_1\",\"usage\":{\"input_tokens\":15,\"output_tokens\":1,\"cache_read_input_tokens\":5}}}\n\n"
                      "data: {\"type\":\"content_block_delta\",\"delta\":{\"type\":\"text_delta\",\"text\":\"Claude response\"}}\n\n"
                      "data: {\"type\":\"message_delta\",\"usage\":{\"output_tokens\":35}}\n\n";
    p->process_stream_chunk(sse, buffer, [&](const std::string& t) { text += t; }, [&](const UsageInfo& u) { stream_usage = u; });

    ASSERT_STREQ(text.c_str(), "Claude response");
    ASSERT_TRUE(stream_usage.has_usage);
    ASSERT_EQ(stream_usage.prompt_tokens, 15);
    ASSERT_EQ(stream_usage.completion_tokens, 35);
    ASSERT_EQ(stream_usage.total_tokens, 50);
    ASSERT_EQ(stream_usage.cached_tokens, 5);
}

AI_TEST(DeepSeekQuotaCheck) {
    auto p = ProviderFactory::create("deepseek");
    MockHttpClient mock;
    mock.mock_resp.success = true;
    mock.mock_resp.status_code = 200;
    mock.mock_resp.body = R"({
        "is_available": true,
        "balance_infos": [
            {
                "currency": "USD",
                "total_balance": "15.4200",
                "granted_balance": "5.0000",
                "topped_up_balance": "10.4200"
            }
        ]
    })";

    QuotaInfo q = p->check_quota(mock, "sk-deepseek-key");
    ASSERT_TRUE(q.supported);
    ASSERT_TRUE(q.success);
    ASSERT_STREQ(q.currency.c_str(), "USD");
    ASSERT_STREQ(q.total_balance.c_str(), "15.4200");
    ASSERT_STREQ(q.granted_balance.c_str(), "5.0000");
    ASSERT_STREQ(q.topped_up_balance.c_str(), "10.4200");
    ASSERT_STREQ(q.status.c_str(), "Active");
}

AI_TEST(OpenRouterQuotaCheck) {
    auto p = ProviderFactory::create("openrouter");
    MockHttpClient mock;
    mock.mock_resp.success = true;
    mock.mock_resp.status_code = 200;
    mock.mock_resp.body = R"({
        "data": {
            "total_credits": 25.50,
            "total_usage": 5.25
        }
    })";

    QuotaInfo q = p->check_quota(mock, "sk-or-v1-key");
    ASSERT_TRUE(q.supported);
    ASSERT_TRUE(q.success);
    ASSERT_STREQ(q.currency.c_str(), "USD");
    ASSERT_STREQ(q.total_balance.c_str(), "20.2500");
    ASSERT_STREQ(q.total_usage.c_str(), "5.2500");
}

AI_TEST(GeminiQuotaCheck) {
    auto p = ProviderFactory::create("google");
    MockHttpClient mock;
    mock.mock_resp.success = true;
    mock.mock_resp.status_code = 200;
    mock.mock_resp.body = R"({"models":[{"name":"models/gemini-2.5-flash"}]})";

    QuotaInfo q = p->check_quota(mock, "AIzaSyFakeKey");
    ASSERT_TRUE(q.success);
    ASSERT_STREQ(q.status.c_str(), "Active");
    ASSERT_TRUE(!q.console_url.empty());
}

