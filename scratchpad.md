# Scratchpad & Brainstorming

## Provider API Formats

### 1. Google Gemini
- URL: `https://generativelanguage.googleapis.com/v1beta/models/{model}:streamGenerateContent?key={apiKey}&alt=sse`
- Payload:
```json
{
  "contents": [
    {
      "role": "user",
      "parts": [{"text": "hello"}]
    }
  ],
  "systemInstruction": {
    "parts": [{"text": "You are a helpful assistant."}]
  },
  "generationConfig": {
    "temperature": 0.7
  }
}
```
- Stream response chunk:
```json
data: {"candidates": [{"content": {"parts": [{"text": "Hello! How can I assist you today?"}], "role": "model"}, "finishReason": "STOP"}]}
```

### 2. OpenAI & OpenAI-Compatible
- URL: `https://api.openai.com/v1/chat/completions` (or `{base_url}/chat/completions`)
- Headers: `Authorization: Bearer {apiKey}`, `Content-Type: application/json`
- Payload:
```json
{
  "model": "gpt-4o-mini",
  "messages": [
    {"role": "system", "content": "You are a helpful assistant."},
    {"role": "user", "content": "hello"}
  ],
  "temperature": 0.7,
  "stream": true
}
```
- Stream response chunk:
```text
data: {"choices":[{"delta":{"content":"Hello"},"index":0,"finish_reason":null}]}
```

### 3. Anthropic Claude
- URL: `https://api.anthropic.com/v1/messages`
- Headers: `x-api-key: {apiKey}`, `anthropic-version: 2023-06-01`, `Content-Type: application/json`
- Payload:
```json
{
  "model": "claude-3-5-haiku-20241022",
  "system": "You are a helpful assistant.",
  "messages": [
    {"role": "user", "content": "hello"}
  ],
  "max_tokens": 4096,
  "temperature": 0.7,
  "stream": true
}
```
- Stream response chunk:
```text
data: {"type": "content_block_delta", "index": 0, "delta": {"type": "text_delta", "text": "Hello"}}
```

## Provider Quota & Balance API Formats

### 1. DeepSeek User Balance API
- Endpoint: `GET https://api.deepseek.com/user/balance`
- Headers: `Authorization: Bearer {apiKey}`
- Response format:
```json
{
  "is_available": true,
  "balance_infos": [
    {
      "currency": "USD",
      "total_balance": "15.4200",
      "granted_balance": "5.0000",
      "topped_up_balance": "10.4200"
    }
  ]
}
```

### 2. OpenRouter Credits API
- Endpoint: `GET https://openrouter.ai/api/v1/credits`
- Headers: `Authorization: Bearer {apiKey}`
- Response format:
```json
{
  "data": {
    "total_credits": 25.50,
    "total_usage": 5.25
  }
}
```
Remaining balance is computed as: `total_credits - total_usage`.

### 3. Rate-Limit Quotas & Console Links
- **Google Gemini**: Models endpoint check + `https://aistudio.google.com/app/plan_information`
- **Anthropic Claude**: Models endpoint check + `https://console.anthropic.com/settings/billing`
- **OpenAI**: Auth check + `https://platform.openai.com/usage`
- **Groq**: Auth check + `https://console.groq.com/settings/limits`
- **Ollama**: Local model execution (no external quotas).

