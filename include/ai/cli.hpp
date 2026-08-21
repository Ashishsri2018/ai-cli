#pragma once

#include <string>
#include <vector>
#include <optional>

namespace ai {

enum class CliMode {
    Query,
    Chat,
    SetConfig,
    DelConfig,
    ListConfig,
    ListModels,
    ListProviders,
    ShowSystemPrompt,
    SetSystemPrompt,
    ShowTemperature,
    SetTemperature,
    Help,
    Version
};

struct CliArgs {
    CliMode mode{CliMode::Query};
    std::string query;
    std::string stdin_content;
    std::string provider;
    std::string model;
    std::string system_prompt;
    bool has_system_prompt{false};
    double temperature{0.7};
    bool has_temperature{false};
    bool stream{true};
    bool color{true};

    std::string config_subcommand;
    std::vector<std::string> config_args;
    std::string error_message;
};

class CliParser {
public:
    static CliArgs parse(int argc, char* argv[]);
};

} // namespace ai
