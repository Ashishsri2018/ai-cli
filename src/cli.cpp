#include "ai/cli.hpp"
#include "ai/terminal.hpp"
#include "ai/utils.hpp"
#include <iostream>

namespace ai {

namespace {

void read_stdin_if_piped(CliArgs& args) {
    if (!term::is_stdin_tty()) {
        std::string line;
        while (std::getline(std::cin, line)) {
            args.stdin_content += line + "\n";
        }
    }
}

void parse_set_flag(int argc, char* argv[], int& i, CliArgs& args) {
    args.mode = CliMode::SetConfig;
    if (i + 1 < argc) {
        args.config_subcommand = argv[++i];
        while (i + 1 < argc && argv[i + 1][0] != '-') {
            args.config_args.push_back(argv[++i]);
        }
    }
}

void parse_del_flag(int argc, char* argv[], int& i, CliArgs& args) {
    args.mode = CliMode::DelConfig;
    if (i + 1 < argc) {
        args.config_subcommand = argv[++i];
        while (i + 1 < argc && argv[i + 1][0] != '-') {
            args.config_args.push_back(argv[++i]);
        }
    }
}

} // namespace

CliArgs CliParser::parse(int argc, char* argv[]) {
    CliArgs args;
    bool force_chat = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            args.mode = CliMode::Help;
            return args;
        } else if (arg == "-v" || arg == "--version") {
            args.mode = CliMode::Version;
            return args;
        } else if (arg == "-l" || arg == "--list") {
            args.mode = CliMode::ListConfig;
            return args;
        } else if (arg == "--models" || arg == "--list-models" || arg == "models") {
            args.mode = CliMode::ListModels;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                args.provider = argv[++i];
            }
            return args;
        } else if (arg == "--set") {
            parse_set_flag(argc, argv, i, args);
            return args;
        } else if (arg == "--del") {
            parse_del_flag(argc, argv, i, args);
            return args;
        } else if ((arg == "-p" || arg == "--provider")) {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                args.provider = argv[++i];
            } else {
                args.mode = CliMode::ListProviders;
            }
        } else if ((arg == "-m" || arg == "--model")) {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                args.model = argv[++i];
            } else {
                args.mode = CliMode::ListModels;
            }
        } else if (arg == "-s" || arg == "--system") {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                args.system_prompt = argv[++i];
                args.has_system_prompt = true;
            } else {
                args.mode = CliMode::ShowSystemPrompt;
            }
        } else if (arg == "-t" || arg == "--temperature") {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                args.temperature = std::stod(argv[++i]);
                args.has_temperature = true;
            } else {
                args.mode = CliMode::ShowTemperature;
            }
        } else if (arg == "-u" || arg == "--usage") {
            args.show_usage = true;
        } else if (arg == "-c" || arg == "--chat") {
            force_chat = true;
        } else if (arg == "--no-stream") {
            args.stream = false;
        } else if (arg == "--raw" || arg == "--no-color") {
            args.color = false;
        } else if (!arg.empty() && arg[0] != '-') {
            if (!args.query.empty()) args.query += " ";
            args.query += arg;
        }
    }

    if (args.mode == CliMode::ListProviders || args.mode == CliMode::ListModels ||
        args.mode == CliMode::ShowSystemPrompt || args.mode == CliMode::ShowTemperature) {
        return args;
    }

    if (!force_chat) {
        read_stdin_if_piped(args);
    }

    if (args.query.empty() && args.stdin_content.empty()) {
        if (args.has_system_prompt) {
            args.mode = CliMode::SetSystemPrompt;
        } else if (args.has_temperature) {
            args.mode = CliMode::SetTemperature;
        } else {
            args.mode = CliMode::Chat;
        }
    } else {
        args.mode = CliMode::Query;
    }
    return args;
}

} // namespace ai
