# Architecture Documentation: AI Terminal Client (`ai`)

## Overview
`ai` is a lightweight, dependency-free C++17 command-line tool for interfacing with cloud Large Language Models (LLMs). It provides a unified terminal interface for single queries, shell pipelines, and continuous multi-turn interactive chat sessions across multiple LLM providers, complete with dynamic remote model discovery, bare-flag inspection, and machine-bound encrypted key storage.

## System Architecture

```mermaid
graph TD
    CLI[CLI Entrypoint: main.cpp / cli.cpp] --> |Parse Flags| CFG[ConfigManager: config.cpp]
    CLI --> |Execute| ROUTE{Execution Mode}
    
    ROUTE --> |Single Query / Pipe| EXEC_SINGLE[QueryRunner: query_runner.cpp]
    ROUTE --> |Interactive Chat| EXEC_REPL[REPL Interactive Mode: repl.cpp]
    ROUTE --> |Config Operations| EXEC_CFG[Config Commands: config_cmd.cpp]
    ROUTE --> |Model Discovery| EXEC_MODELS[QueryRunner::list_provider_models: config_cmd.cpp]
    ROUTE --> |Provider Listing| EXEC_PROVS[QueryRunner::list_supported_providers: config_cmd.cpp]
    
    CFG --> |Encrypt / Decrypt| CRYPTO[Crypto Engine: crypto.cpp / crypto_cipher.cpp]
    
    EXEC_SINGLE --> SESS[ChatSession: session.cpp]
    EXEC_REPL --> SESS
    
    SESS --> FACTORY[ProviderFactory: provider.cpp]
    EXEC_MODELS --> FACTORY
    
    FACTORY --> P_GEMINI[GoogleGeminiProvider: provider_gemini.cpp]
    FACTORY --> P_OPENAI[OpenAIProvider: provider_openai.cpp]
    FACTORY --> P_ANTHROPIC[AnthropicProvider: provider_anthropic.cpp]
    
    P_GEMINI --> HTTP[HttpClient: http_client.cpp]
    P_OPENAI --> HTTP
    P_ANTHROPIC --> HTTP
    
    HTTP --> |SSE Chunks / GET Responses| STREAM[Stream & JSON Processor]
    STREAM --> |Formatted Output| TTY[Terminal stdout]
```

## Module Breakdown

1. **`crypto.hpp` / `crypto.cpp` / `crypto_cipher.cpp`**: Hardware/user-bound key derivation, SHA-256, Base64, and stream cipher for encrypted API key storage (`enc:v1:...`) with zero external crypto package dependencies.
2. **`types.hpp`**: Common types (`ChatMessage`, `Role`, `RequestOptions`, `HttpResponse`, `StreamCallback`).
3. **`json.hpp` / `json_dump.cpp` / `json_parse.cpp`**: Zero-dependency RFC 8259 JSON parser and serializer with clean C++ interface.
4. **`utils.hpp` / `utils.cpp`**: Path resolution, home directory detection, secure file permissions (0600), string trimming, environment variable lookups.
5. **`config.hpp` / `config.cpp`**: Encrypted configuration loading and persistence to `~/.config/ai/config.json`.
6. **`session.hpp` / `session.cpp`**: Conversation history state management, message appending, context trimming.
7. **`http_client.hpp` / `http_client.cpp`**: Libcurl abstraction supporting GET, POST, and real-time SSE streaming callbacks.
8. **`provider.hpp` / `provider.cpp`**: Base interface `LLMProvider` with `list_models` and factory method `create`.
9. **`provider_gemini.hpp` / `provider_gemini.cpp`**: Google Gemini REST API implementation (`streamGenerateContent`, `generateContent`, `models` list).
10. **`provider_openai.hpp` / `provider_openai.cpp`**: OpenAI / OpenAI-compatible REST API implementation (Groq, DeepSeek, Ollama, custom base URLs, `models` list).
11. **`provider_anthropic.hpp` / `provider_anthropic.cpp`**: Anthropic Claude REST API implementation (`messages`, `models` list).
12. **`terminal.hpp` / `terminal.cpp`**: ANSI color formatting, TTY detection, banner display.
13. **`cli.hpp` / `cli.cpp`**: Command-line flag parsing with bare flag support (`ai -p`, `ai -m`, `ai -p google -m`).
14. **`repl.hpp` / `repl.cpp`**: Interactive prompt loop, multiline input, in-chat slash commands (`/clear`, `/models`, `/model`, `/provider`, `/system`, `/help`, `/history`, `quit`).
15. **`query_runner.hpp` / `query_runner.cpp`**: Single query execution.
16. **`config_cmd.cpp`**: Config setting, listing, deletion, provider listing, and remote model listing.
17. **`main.cpp`**: Entry point orchestrating CLI parsing and mode dispatch.
