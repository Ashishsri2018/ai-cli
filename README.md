# `ai` - Dependency-Free C++ Cloud LLM Terminal Client

A lightweight, ultra-fast, dependency-free CLI client written in modern C++17 to interact with cloud Large Language Models directly from your terminal.

---

## Features

- **Zero External Package Dependencies**: Compiles cleanly on Linux, macOS, Android/Termux, BSD, and Windows using standard C++17 and system `libcurl`. Includes a built-in RFC-8259 JSON parser/serializer.
- **Secure Encrypted API Key Storage**:
  - Automatically encrypts all API keys using hardware/user-bound key derivation and stream encryption (`enc:v1:...`).
  - Stored in `~/.config/ai/config.json` with strict `0600` permissions.
  - Transparent in-memory decryption when executing requests.
- **Bare Flag Provider & Model Discovery**:
  - `ai -p` or `ai --provider`: Lists all supported providers and highlights the active default.
  - `ai -m` or `ai --model`: Queries the API and lists all available models for the current default provider.
  - `ai -p google -m`: Queries the Google API and lists all models for Google.
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

## Bare Flag Quick Inspection & Discovery

### 1. Listing Supported Providers
Run `-p` with no arguments to list all supported providers and view the active default:
```bash
ai -p
# or
ai --provider
```
*Output:*
```text
Supported LLM Providers:
  * google (current default)
  - openai
  - anthropic
  - groq
  - deepseek
  - ollama
  - custom

To change default provider: ai --set provider <name>
To list models for a provider: ai -p <provider> -m
```

---

### 2. Listing Available Models for Your Key
Run `-m` with no arguments to query the remote API and list all accessible models:
```bash
# List models for current default provider
ai -m
# or
ai --model

# List models for a specific provider
ai -p google -m
ai -p openai -m
ai -p anthropic -m
```
*Output:*
```text
Fetching models from google API...
Available models for provider 'google':
  * gemini-3.6-flash (current default)
  - gemini-3.7-flash
  - gemini-3.5-flash
  - gemini-3.1-flash-lite
  - gemini-2.5-flash-lite

To change default model: ai --set model <name> google
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

### 2. Changing Default Provider & Models
```bash
# Set default model for a provider
ai --set model gemini-3.6-flash google
ai --set model gpt-4o openai

# Set default provider (used when no -p flag is passed)
ai --set provider google
ai --set provider openai
```

### 3. Listing Configured Keys & Providers
```bash
ai --list
```

### 4. Deleting an API Key
```bash
ai --del api "openai"
```

---

## Usage & Examples

### 1. Single Query Mode

Ask questions directly from your terminal:
```bash
# Using default provider and model
ai "What is the capital of France?"

# Specify a provider
ai -p openai "Explain quantum computing in 2 sentences"

# Specify a provider and specific model
ai -p google -m gemini-3.6-flash "What is 15 * 12?"
ai -p anthropic -m claude-3-5-haiku-20241022 "Write a python function to reverse a linked list"

# Provide a custom system prompt
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
Paris is the capital of France.

> /models
Fetching models for google...
  * gemini-3.6-flash (active)
  - gemini-3.7-flash
  - gemini-3.5-flash

> /model gemini-3.7-flash
✓ Switched model to: gemini-3.7-flash

> Give me 3 must-visit landmarks there.
1. The Eiffel Tower
2. The Louvre Museum
3. Notre-Dame Cathedral

> /clear
✓ Conversation history cleared.

> quit
Goodbye!
```

---

## All Command-Line Flags

| Flag | Short | Description | Example |
| :--- | :--- | :--- | :--- |
| `-p [name]`, `--provider [name]` | `-p` | Select provider, or **list providers** if name omitted | `ai -p` or `ai -p google "hi"` |
| `-m [name]`, `--model [name]` | `-m` | Select model, or **list models** if name omitted | `ai -m` or `ai -p google -m` |
| `--models [provider]` | | Fetch and list available models from provider API | `ai --models google` |
| `--system <prompt>` | `-s` | Provide system instruction prompt | `ai -s "Be brief" "hello"` |
| `--temperature <val>`| `-t` | Sampling temperature (default: `0.7`) | `ai -t 0.3 "hello"` |
| `--chat` | `-c` | Start interactive multi-turn REPL chat | `ai -c` |
| `--no-stream` | | Disable real-time token streaming | `ai --no-stream "hello"` |
| `--raw`, `--no-color`| | Disable ANSI terminal colors (useful for pipelines) | `ai --raw "hello"` |
| `--set <args...>` | | Configure settings (`api`, `model`, `provider`) | `ai --set model gemini-3.6-flash google` |
| `--del <args...>` | | Remove settings | `ai --del api "openai"` |
| `--list` | `-l` | Display current configuration and keys | `ai --list` |
| `--help` | `-h` | Display help screen | `ai --help` |
| `--version` | `-v` | Display version information | `ai --version` |

---

## Running Automated Tests

```bash
make test
```

Test coverage:
- Self-contained SHA-256 and Base64 cryptographic routines
- AES/ChaCha stream cipher key encryption & decryption with machine-binding
- Encrypted configuration persistence & transparent decryption
- Bare flag parsing (`ai -p`, `ai -m`, `ai -p google -m`)
- Remote model listing JSON decoding across Google Gemini, OpenAI, and Anthropic
- Multi-turn chat session history tracking
- Gemini, OpenAI, and Anthropic request payload construction & response extraction
- Real-time Server-Sent Events (SSE) streaming token chunk decoders

---

## License
MIT License
