# `ai` - Dependency-Free C++ Cloud LLM Terminal Client

A lightweight, ultra-fast, dependency-free CLI client written in modern C++17 to interact with cloud Large Language Models directly from your terminal.

---

## Features

- **Zero External Package Dependencies**: Compiles cleanly on Linux, macOS, Android/Termux, BSD, and Windows using standard C++17 and system `libcurl`. Includes a built-in RFC-8259 JSON parser/serializer.
- **Secure Encrypted API Key Storage**:
  - Automatically encrypts all API keys using hardware/user-bound key derivation and stream encryption (`enc:v1:...`).
  - Stored in `~/.config/ai/config.json` with strict `0600` permissions.
  - Transparent in-memory decryption when executing requests.
- **Bare Flag Inspection & Configuration**:
  - `ai -p` or `ai --provider`: Lists all supported providers and highlights the active default.
  - `ai -m` or `ai --model`: Lists all available models for the current default provider.
  - `ai -p google -m`: Lists all models for Google.
  - `ai -s` or `ai --system`: Displays the currently configured system prompt.
  - `ai -s "<prompt>"`: Persistently saves the default system prompt to `~/.config/ai/config.json`.
  - `ai -t` or `ai --temperature`: Displays the currently configured default temperature.
  - `ai -t <val>`: Persistently saves the default temperature to `~/.config/ai/config.json`.
- **Multi-Cloud Provider Support**:
  - **Google Gemini** (`gemini-3.6-flash`, `gemini-3.7-flash`, `gemini-2.5-flash-lite`, etc.)
  - **OpenAI** (`gpt-4o-mini`, `gpt-4o`, `o3-mini`, etc.)
  - **Anthropic Claude** (`claude-3-5-haiku-20241022`, `claude-3-7-sonnet-20250219`, etc.)
  - **Groq** (`llama-3.3-70b-versatile`, etc.)
  - **DeepSeek** (`deepseek-chat`, `deepseek-reasoner`)
  - **Ollama** (`llama3.2`, `mistral`, local models)
  - Custom OpenAI-compatible REST endpoints
- **Modes of Operation**:
  - **Single Query Mode**: Quick one-off terminal queries.
  - **Continuous Interactive Chat (REPL)**: Multi-turn conversations maintaining conversation context history.
  - **Shell Pipeline Integration**: Pipe files, command outputs, and logs directly into your prompts (`cat file | ai "query"`).
- **Real-Time Token Streaming**: Immediate token-by-token output with Server-Sent Events (SSE).

---

## Installation & Build

### Automatic OS-Aware Build & Global Install
Run `./build.sh` to automatically detect your OS, verify C++17 compiler, run the test suite, and install `ai` to your global PATH:
```bash
./build.sh
```

Or build manually using `make`:
```bash
make
make test
```

---

## Bare Flag Quick Inspection & Configuration

### 1. System Prompt (`-s`)
```bash
# View currently configured system prompt
ai -s
# or
ai --system

# Persistently store a new default system prompt in ~/.config/ai/config.json
ai -s "Omit conversational filler."

# All future queries automatically use this prompt:
ai "What is the capital of France?"
# Output: Paris

# One-off override for a single query (does not change the default in config):
ai -s "You are a poet." "What is 2+2?"
```

---

### 2. Temperature (`-t`)
```bash
# View currently configured temperature
ai -t
# or
ai --temperature

# Persistently store a new default temperature in ~/.config/ai/config.json
ai -t 0.3

# One-off override for a single query:
ai -t 0.9 "Brainstorm 3 creative startup names"
```

---

### 3. Providers (`-p`) & Models (`-m`)
```bash
# List supported providers and view active default
ai -p

# List models for default provider
ai -m

# List models for a specific provider
ai -p google -m
ai -p openai -m
```

---

## Configuration & Key Management

### 1. Setting API Keys (Automatically Encrypted)
API keys are securely encrypted before being written to disk:
```bash
# Google Gemini
ai --set api "AIzaSy..." "google"

# OpenAI
ai --set api "sk-proj-..." "openai"

# Anthropic Claude
ai --set api "sk-ant-..." "anthropic"

# Groq
ai --set api "gsk_..." "groq"

# DeepSeek
ai --set api "sk-..." "deepseek"
```

### 2. Changing Defaults
```bash
# Set default model for a provider
ai --set model gemini-3.6-flash google
ai --set model gpt-4o openai

# Set default provider (used when no -p flag is passed)
ai --set provider google
ai --set provider openai

# Set default system prompt
ai --set system "Be concise and clear."

# Set default temperature
ai --set temp 0.5
```

### 3. Listing & Deleting Configuration
```bash
# List all configuration settings
ai --list

# Delete an API key
ai --del api "openai"

# Clear default system prompt
ai --del system
```

---

## Usage & Examples

### 1. Single Query Mode

Ask questions directly from your terminal:
```bash
# Using default provider, model, and system prompt
ai "What is the capital of France?"

# Specify a provider
ai -p openai "Explain quantum computing in 2 sentences"

# Specify a provider and specific model
ai -p google -m gemini-3.6-flash "What is 15 * 12?"
ai -p anthropic -m claude-3-5-haiku-20241022 "Write a python function to reverse a linked list"

# Provide a one-off system prompt
ai -s "You are a Linux kernel engineer. Be concise." "Why is eBPF useful?"

# Adjust sampling temperature (0.0 to 2.0)
ai -t 0.2 "Solve this math problem: 24 * 17"
```

---

### 2. Stdin Piping & Shell Scripting

Pipe files, scripts, logs, or command outputs into `ai`:

```bash
# Code review
cat src/main.cpp | ai "Review this C++ code for potential memory leaks"

# Log analysis
ai "What caused this error?" < /var/log/syslog

# Command output explanation
git diff | ai "Generate a concise git commit message for these changes"

# Disable streaming or ANSI colors for scripting
ai --no-stream --raw "List 5 prime numbers separated by commas"
```

---

### 3. Continuous Multi-Turn Interactive Chat (REPL)

Start an interactive session:
```bash
ai
# or
ai --chat
# or with specific provider/model
ai -p openai -m gpt-4o --chat
```

**Interactive REPL Session Example:**
```text
🤖 AI Terminal Assistant (Provider: google | Model: gemini-3.6-flash)
Type your message. Commands: /clear, /models, /model <name>, /provider <name>, /system <prompt>, /help, quit.

> What is the capital of France?
Paris

> /models
Fetching models for google...
  * gemini-3.6-flash (active)
  - gemini-3.7-flash
  - gemini-3.5-flash

> /model gemini-3.7-flash
✓ Switched model to: gemini-3.7-flash

> /clear
✓ Conversation history cleared.

> quit
Goodbye!
Type your message. Commands: /clear, /models, /model <name>, /provider <name>, /system <prompt>, /usage [on|off], /help, quit.
```

---

## All Command-Line Flags

| Flag | Short | Description | Example |
| :--- | :--- | :--- | :--- |
| `-s [prompt]`, `--system [prompt]` | `-s` | Show prompt (if empty), set default (no query), or use one-off (with query) | `ai -s` or `ai -s "Omit filler."` |
| `-t [val]`, `--temperature [val]` | `-t` | Show temp (if empty), set default (no query), or use one-off (with query) | `ai -t` or `ai -t 0.3` |
| `-u`, `--usage` | `-u` | Display API token usage statistics (prompt, completion, total, cached) | `ai -u "hello"` or `ai --usage` |
| `-p [name]`, `--provider [name]` | `-p` | Select provider, or **list providers** if name omitted | `ai -p` or `ai -p google "hi"` |
| `-m [name]`, `--model [name]` | `-m` | Select model, or **list models** if name omitted | `ai -m` or `ai -p google -m` |
| `--models [provider]` | | Fetch and list available models from provider API | `ai --models google` |
| `--chat` | `-c` | Start interactive multi-turn REPL chat | `ai -c` |
| `--no-stream` | | Disable real-time token streaming | `ai --no-stream "hello"` |
| `--raw`, `--no-color`| | Disable ANSI terminal colors (useful for pipelines) | `ai --raw "hello"` |
| `--set <args...>` | | Configure settings (`api`, `model`, `provider`, `system`, `temp`) | `ai --set system "Be brief."` |
| `--del <args...>` | | Remove settings (`api`, `system`) | `ai --del system` |
| `--list` | `-l` | Display current configuration and keys | `ai --list` |
| `--help` | `-h` | Display help screen | `ai --help` |
| `--version` | `-v` | Display version information | `ai --version` |

---

## Running Automated Tests

```bash
make test
```

Test coverage (31 unit tests):
Test coverage (35 unit tests):
- Self-contained SHA-256 and Base64 cryptographic routines
- AES/ChaCha stream cipher key encryption & decryption with machine-binding
- Encrypted configuration persistence & transparent decryption
- System prompt and temperature persistence and bare flag handling
- Bare flag parsing (`ai -p`, `ai -m`, `ai -s`, `ai -t`, `ai -p google -m`)
- Bare flag parsing (`ai -p`, `ai -m`, `ai -s`, `ai -t`, `ai -p google -m`, `ai -u`, `ai --usage`)
- Remote model listing JSON decoding across Google Gemini, OpenAI, and Anthropic
- Multi-turn chat session history tracking
- Gemini, OpenAI, and Anthropic request payload construction & response extraction
- Real-time Server-Sent Events (SSE) streaming token chunk decoders
- Token usage extraction & streaming token counters across Gemini, OpenAI, and Anthropic

---

## License
MIT License
