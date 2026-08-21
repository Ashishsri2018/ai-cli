#include "test_runner.hpp"
#include "ai/config.hpp"

using namespace ai;

AI_TEST(ConfigKeyManagement) {
    std::string temp_cfg = "/tmp/ai_test_config.json";
    ConfigManager cm(temp_cfg);

    cm.set_api_key("openai", "sk-test-key-12345");
    cm.set_api_key("google", "AIzaSyTestKey");
    cm.set_default_provider("openai");
    cm.set_default_model("openai", "gpt-4o");
    ASSERT_TRUE(cm.save());

    ConfigManager cm2(temp_cfg);
    ASSERT_STREQ(cm2.get_default_provider().c_str(), "openai");
    ASSERT_STREQ(cm2.get_default_model("openai").c_str(), "gpt-4o");

    auto key1 = cm2.get_api_key("openai");
    ASSERT_TRUE(key1.has_value());
    ASSERT_STREQ(key1->c_str(), "sk-test-key-12345");

    auto key2 = cm2.get_api_key("google");
    ASSERT_TRUE(key2.has_value());
    ASSERT_STREQ(key2->c_str(), "AIzaSyTestKey");

    ASSERT_TRUE(cm2.delete_api_key("openai"));
    ASSERT_TRUE(cm2.save());

    ConfigManager cm3(temp_cfg);
    // Since environment variable might not be set in test, verify config key deletion
    ASSERT_TRUE(cm3.get_config().api_keys.find("openai") == cm3.get_config().api_keys.end());
}
