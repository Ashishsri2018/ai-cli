#include "test_runner.hpp"
#include "ai/cli.hpp"

using namespace ai;

AI_TEST(CliParserQuery) {
    const char* argv[] = {"ai", "-p", "openai", "-m", "gpt-4o", "capital", "of", "france"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    CliArgs args = CliParser::parse(argc, const_cast<char**>(argv));
    ASSERT_EQ(static_cast<int>(args.mode), static_cast<int>(CliMode::Query));
    ASSERT_STREQ(args.provider.c_str(), "openai");
    ASSERT_STREQ(args.model.c_str(), "gpt-4o");
    ASSERT_STREQ(args.query.c_str(), "capital of france");
}

AI_TEST(CliParserSetConfig) {
    const char* argv[] = {"ai", "--set", "api", "sk-test-key", "openai"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    CliArgs args = CliParser::parse(argc, const_cast<char**>(argv));
    ASSERT_EQ(static_cast<int>(args.mode), static_cast<int>(CliMode::SetConfig));
    ASSERT_STREQ(args.config_subcommand.c_str(), "api");
    ASSERT_EQ(args.config_args.size(), 2);
    ASSERT_STREQ(args.config_args[0].c_str(), "sk-test-key");
    ASSERT_STREQ(args.config_args[1].c_str(), "openai");
}

AI_TEST(CliParserDelConfig) {
    const char* argv[] = {"ai", "--del", "api", "openai"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    CliArgs args = CliParser::parse(argc, const_cast<char**>(argv));
    ASSERT_EQ(static_cast<int>(args.mode), static_cast<int>(CliMode::DelConfig));
    ASSERT_STREQ(args.config_subcommand.c_str(), "api");
    ASSERT_EQ(args.config_args.size(), 1);
    ASSERT_STREQ(args.config_args[0].c_str(), "openai");
}

AI_TEST(CliParserListConfig) {
    const char* argv[] = {"ai", "--list"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    CliArgs args = CliParser::parse(argc, const_cast<char**>(argv));
    ASSERT_EQ(static_cast<int>(args.mode), static_cast<int>(CliMode::ListConfig));
}

AI_TEST(CliParserListModels) {
    const char* argv[] = {"ai", "--models", "google"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    CliArgs args = CliParser::parse(argc, const_cast<char**>(argv));
    ASSERT_EQ(static_cast<int>(args.mode), static_cast<int>(CliMode::ListModels));
    ASSERT_STREQ(args.provider.c_str(), "google");
}

AI_TEST(CliParserBareFlags) {
    const char* argv_p[] = {"ai", "-p"};
    CliArgs args_p = CliParser::parse(2, const_cast<char**>(argv_p));
    ASSERT_EQ(static_cast<int>(args_p.mode), static_cast<int>(CliMode::ListProviders));

    const char* argv_m[] = {"ai", "-m"};
    CliArgs args_m = CliParser::parse(2, const_cast<char**>(argv_m));
    ASSERT_EQ(static_cast<int>(args_m.mode), static_cast<int>(CliMode::ListModels));

    const char* argv_pm[] = {"ai", "-p", "google", "-m"};
    CliArgs args_pm = CliParser::parse(4, const_cast<char**>(argv_pm));
    ASSERT_EQ(static_cast<int>(args_pm.mode), static_cast<int>(CliMode::ListModels));
    ASSERT_STREQ(args_pm.provider.c_str(), "google");
}
