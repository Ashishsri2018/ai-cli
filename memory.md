# Project Memory: AI Terminal Client (`ai`)

## Project Context
Building a dependency-free, high-performance C++17 CLI client for cloud LLMs (Google Gemini, OpenAI, Anthropic, Ollama, Groq, DeepSeek).

## Milestones & Accomplishments
- **2026-08-21**:
  - Implemented complete C++17 architecture with zero external third-party package dependencies (standard C++ STL + system `libcurl`).
  - Built custom RFC-8259 JSON parser and serializer in `include/ai/json.hpp`, `src/json_dump.cpp`, `src/json_parse.cpp`.
  - Implemented secure API key and configuration management with `0600` file permissions in `include/ai/config.hpp` and `src/config.cpp`.
  - Added self-contained hardware/user-bound key derivation and stream encryption (`enc:v1:...`) in `include/ai/crypto.hpp`, `src/crypto.cpp`, `src/crypto_cipher.cpp` to store all keys encrypted on disk with transparent in-memory decryption.
  - Added persistent system prompt and temperature configuration in `~/.config/ai/config.json`.
  - Added bare-flag inspection & direct setting for:
    - `-p` / `--provider`: bare flag lists providers; sets default provider with `--set provider <name>`.
    - `-m` / `--model`: bare flag lists models; sets default model with `--set model <name> [provider]`.
    - `-s` / `--system`: bare flag (`ai -s`) displays current system prompt; `ai -s "<prompt>"` without query saves default system prompt; `ai -s "<prompt>" "query"` applies one-off prompt.
    - `-t` / `--temperature`: bare flag (`ai -t`) displays current temperature; `ai -t <val>` without query saves default temperature; `ai -t <val> "query"` applies one-off temperature.
  - Updated default Gemini model fallback to `gemini-3.6-flash`.
  - Created automated test suite (`run_tests`) with 31/31 unit tests passing across crypto, JSON, config, sessions, providers, and CLI parsing.
  - Verified 100% compliance with modular architecture rules (all source files under 150 lines).
  - Created comprehensive `README.md` documenting installation, configuration, single queries, stdin piping, REPL interactive commands, bare flag discovery, system prompt / temperature persistence, and flag reference.
  - Added `.gitignore` to prevent tracking compiled binaries (`ai`, `run_tests`) and intermediate object files (`*.o`).
  - Created remote GitHub repository and pushed `master` branch via `gh repo create`.

