#include "ai/provider.hpp"
#include "ai/provider_gemini.hpp"
#include "ai/provider_openai.hpp"
#include "ai/provider_anthropic.hpp"
#include "ai/utils.hpp"

namespace ai {

std::unique_ptr<LLMProvider> ProviderFactory::create(const std::string& raw_name, const std::optional<std::string>& custom_endpoint) {
    std::string name = utils::to_lower(utils::trim(raw_name));

    if (name == "google" || name == "gemini") {
        return std::make_unique<GoogleGeminiProvider>();
    }
    if (name == "anthropic" || name == "claude") {
        return std::make_unique<AnthropicProvider>();
    }
    if (name == "groq") {
        std::string url = custom_endpoint.value_or("https://api.groq.com/openai/v1");
        return std::make_unique<OpenAIProvider>("groq", "llama-3.3-70b-versatile", url);
    }
    if (name == "deepseek") {
        std::string url = custom_endpoint.value_or("https://api.deepseek.com/v1");
        return std::make_unique<OpenAIProvider>("deepseek", "deepseek-chat", url);
    }
    if (name == "ollama") {
        std::string url = custom_endpoint.value_or("http://localhost:11434/v1");
        return std::make_unique<OpenAIProvider>("ollama", "llama3.2", url);
    }
    // Default to OpenAI / OpenAI-compatible
    std::string url = custom_endpoint.value_or("https://api.openai.com/v1");
    return std::make_unique<OpenAIProvider>(name.empty() ? "openai" : name, "gpt-4o-mini", url);
}

std::vector<std::string> ProviderFactory::get_supported_providers() {
    return {"google", "openai", "anthropic", "groq", "deepseek", "ollama", "custom"};
}

} // namespace ai
