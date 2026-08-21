# Project Memory: AI Terminal Client (`ai`)

## Project Context
Building a dependency-free, high-performance C++17 CLI client for cloud LLMs (Google Gemini, OpenAI, Anthropic, Ollama, Groq, DeepSeek).

## Milestones & Accomplishments
- **2026-08-21**:
  - Implemented complete C++17 architecture with zero external third-party package dependencies (standard C++ STL + system `libcurl`).
  - Built custom RFC-8259 JSON parser and serializer in `include/ai/json.hpp`, `src/json_dump.cpp`, `src/json_parse.cpp`.
  - Implemented secure API key and configuration management with `0600` file permissions in `include/ai/config.hpp` and `src/config.cpp`.
  - Added self-contained hardware/user-bound key derivation and stream encryption (`enc:v1:...`) in `include/ai/crypto.hpp`, `src/crypto.cpp`, `src/crypto_cipher.cpp` to store all keys encrypted on disk with transparent in-memory decryption.
  - Implemented unified `LLMProvider` interface and implementations for Google Gemini, OpenAI, Anthropic Claude, Groq, DeepSeek, and Ollama.
  - Implemented real-time token streaming using Server-Sent Events (SSE) and libcurl write callbacks in `include/ai/http_client.hpp` and `src/http_client.cpp`.
  - Implemented interactive multi-turn REPL chat mode with in-session commands (`/clear`, `/models`, `/model`, `/provider`, `/system`, `/history`, `/help`, `quit`) in `src/repl.cpp`.
  - Added bare-flag inspection (`ai -p` to list providers, `ai -m` to list default models, `ai -p google -m` to list Google models) and dynamic remote model discovery querying `/models` endpoints.
  - Added dynamic model & provider persistent switching (`ai --set model <name> [provider]`, `ai --set provider <name>`).
  - Updated default Gemini model fallback to `gemini-3.6-flash`.
  - Created automated test suite (`run_tests`) covering SHA-256/Base64, crypto roundtrips, encrypted config persistence, JSON parsing/serialization, utility routines, chat session history, LLM provider requests & stream chunk decoding, remote model listing, and bare flag CLI parsing with 22/22 unit tests passing.
  - Verified 100% compliance with modular architecture rules (all source files under 150 lines).
  - Moved legacy/unrelated Python scripts (`agent.py`, `script.py`, `tui_app.py`), text files, and `venv` outside the project folder to `/root/workspace2/harness_python_archive`.
  - Created comprehensive `README.md` documenting installation, configuration, single queries, stdin piping, REPL interactive commands, bare flag discovery, and flag reference.
