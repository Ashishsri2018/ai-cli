#include "ai/terminal.hpp"
#include <unistd.h>

namespace ai::term {

namespace {
bool g_color_enabled = true;
}

void set_color_enabled(bool enabled) {
    g_color_enabled = enabled;
}

bool is_color_enabled() {
    return g_color_enabled && is_stdout_tty();
}

bool is_stdout_tty() {
    return isatty(fileno(stdout));
}

bool is_stdin_tty() {
    return isatty(fileno(stdin));
}

std::string colorize(const std::string& text, Color color) {
    if (!is_color_enabled()) return text;
    std::string code;
    switch (color) {
        case Color::Bold:    code = "\033[1m"; break;
        case Color::Dim:     code = "\033[2m"; break;
        case Color::Red:     code = "\033[31m"; break;
        case Color::Green:   code = "\033[32m"; break;
        case Color::Yellow:  code = "\033[33m"; break;
        case Color::Blue:    code = "\033[34m"; break;
        case Color::Magenta: code = "\033[35m"; break;
        case Color::Cyan:    code = "\033[36m"; break;
        case Color::White:   code = "\033[37m"; break;
        default:             return text;
    }
    return code + text + "\033[0m";
}

void print_banner(const std::string& provider, const std::string& model) {
    std::cout << colorize("🤖 AI Terminal Assistant", Color::Cyan)
              << " (" << colorize("Provider: " + provider, Color::Dim)
              << " | " << colorize("Model: " + model, Color::Dim) << ")\n"
              << colorize("Type your message. Commands: /clear, /model <name>, /provider <name>, /system <prompt>, /help, quit.", Color::Dim)
              << "\n\n";
}

void print_error(const std::string& msg) {
    std::cerr << colorize("Error: ", Color::Red) << msg << "\n";
}

void print_success(const std::string& msg) {
    std::cout << colorize("✓ ", Color::Green) << msg << "\n";
}

void print_info(const std::string& msg) {
    std::cout << colorize("ℹ ", Color::Cyan) << msg << "\n";
}

void print_help() {
    std::cout << colorize("AI Terminal Client (C++ Edition)", Color::Bold) << "\n\n"
              << colorize("USAGE:", Color::Yellow) << "\n"
              << "  ai [FLAGS] [QUERY]\n"
              << "  ai                     Start continuous interactive chat\n"
              << "  ai \"capital of france\" Direct terminal question\n"
              << "  cat file | ai \"query\"  Pipe stdin to AI prompt\n\n"
              << colorize("API KEY MANAGEMENT:", Color::Yellow) << "\n"
              << "  ai --set api <key> <provider>   Set API key (e.g. ai --set api \"sk-...\" \"openai\")\n"
              << "  ai --del api <provider>         Delete API key for provider\n"
              << "  ai --list                       List configured providers and keys\n\n"
              << colorize("FLAGS:", Color::Yellow) << "\n"
              << "  -p, --provider <name>   Select LLM provider (google, openai, anthropic, groq, deepseek, ollama)\n"
              << "  -m, --model <name>      Select model (e.g. gemini-2.5-flash, gpt-4o, claude-3-5-haiku-20241022)\n"
              << "  -s, --system <prompt>   Set custom system prompt\n"
              << "  -t, --temperature <val> Set temperature (default: 0.7)\n"
              << "  -c, --chat              Force interactive chat mode\n"
              << "  --no-stream             Disable real-time streaming\n"
              << "  --raw, --no-color       Disable ANSI color codes\n"
              << "  -h, --help              Show this help message\n"
              << "  -v, --version           Show version information\n";
}

void print_version() {
    std::cout << "ai version 1.0.0 (C++17, dependency-free)\n";
}

} // namespace ai::term
