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

## Config JSON File Schema (`~/.config/ai/config.json`)
```json
{
  "default_provider": "google",
  "default_models": {
    "google": "gemini-2.5-flash",
    "openai": "gpt-4o-mini",
    "anthropic": "claude-3-5-haiku-20241022",
    "groq": "llama-3.3-70b-versatile",
    "deepseek": "deepseek-chat",
    "ollama": "llama3.2"
  },
  "api_keys": {
    "google": "...",
    "openai": "...",
    "anthropic": "...",
    "groq": "...",
    "deepseek": "..."
  },
  "custom_endpoints": {
    "ollama": "http://localhost:11434/v1"
  }
}
```
