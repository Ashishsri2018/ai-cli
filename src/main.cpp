#include "ai/cli.hpp"
#include "ai/config.hpp"
#include "ai/http_client.hpp"
#include "ai/terminal.hpp"
#include "ai/repl.hpp"
#include "ai/query_runner.hpp"

int main(int argc, char* argv[]) {
    using namespace ai;
    CliArgs args = CliParser::parse(argc, argv);
    term::set_color_enabled(args.color);

    if (args.mode == CliMode::Help) { term::print_help(); return 0; }
    if (args.mode == CliMode::Version) { term::print_version(); return 0; }

    ConfigManager cm;
    CurlHttpClient http_client;

    if (args.mode == CliMode::ListConfig || args.mode == CliMode::SetConfig || args.mode == CliMode::DelConfig) {
        QueryRunner::handle_config_command(cm, args);
        return 0;
    }

    if (args.mode == CliMode::ListProviders) {
        QueryRunner::list_supported_providers(cm);
        return 0;
    }

    if (args.mode == CliMode::ListModels) {
        QueryRunner::list_provider_models(cm, http_client, args.provider);
        return 0;
    }

    if (args.mode == CliMode::Chat) {
        RequestOptions opt;
        opt.model = args.model;
        opt.system_prompt = args.system_prompt;
        opt.temperature = args.temperature;
        opt.stream = args.stream;
        ReplSession repl(cm, http_client, opt);
        repl.run();
        return 0;
    }

    QueryRunner::run_single_query(cm, http_client, args);
    return 0;
}
