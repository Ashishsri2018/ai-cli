#pragma once

#include <string>
#include <iostream>

#include "ai/types.hpp"

namespace ai::term {

enum class Color {
    Default,
    Bold,
    Dim,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White
};

void set_color_enabled(bool enabled);
bool is_color_enabled();
bool is_stdout_tty();
bool is_stdin_tty();

std::string colorize(const std::string& text, Color color);
void print_banner(const std::string& provider, const std::string& model);
void print_error(const std::string& msg);
void print_success(const std::string& msg);
void print_info(const std::string& msg);
void print_usage(const UsageInfo& usage);
void print_help();
void print_version();

} // namespace ai::term
