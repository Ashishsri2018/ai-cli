#include "test_runner.hpp"
#include "ai/utils.hpp"
#include <sys/stat.h>

using namespace ai;

AI_TEST(UtilsStringOperations) {
    ASSERT_STREQ(utils::trim("  hello world \t\n").c_str(), "hello world");
    ASSERT_STREQ(utils::to_lower("HeLLo WoRLD").c_str(), "hello world");
    ASSERT_TRUE(utils::starts_with("ai --set api", "ai --set"));
    ASSERT_FALSE(utils::starts_with("ai --set api", "openai"));
    ASSERT_TRUE(utils::ends_with("main.cpp", ".cpp"));
    ASSERT_FALSE(utils::ends_with("main.cpp", ".hpp"));

    auto parts = utils::split("apple,banana,cherry", ',');
    ASSERT_EQ(parts.size(), 3);
    ASSERT_STREQ(parts[0].c_str(), "apple");
    ASSERT_STREQ(parts[1].c_str(), "banana");
    ASSERT_STREQ(parts[2].c_str(), "cherry");

    std::string rep = utils::replace_all("foo bar foo", "foo", "baz");
    ASSERT_STREQ(rep.c_str(), "baz bar baz");
}

AI_TEST(UtilsFileReadWrite) {
    std::string test_path = "/tmp/ai_test_file.txt";
    std::string test_data = "sample config data 123";
    ASSERT_TRUE(utils::write_file(test_path, test_data, true));

    std::string read_back;
    ASSERT_TRUE(utils::read_file(test_path, read_back));
    ASSERT_STREQ(read_back.c_str(), test_data.c_str());
}

AI_TEST(UtilsSecureFilePermissions) {
    std::string test_path = "/tmp/ai_test_secure_perms.txt";
    ASSERT_TRUE(utils::write_file(test_path, "secret data", true));

    struct stat st;
    ASSERT_EQ(stat(test_path.c_str(), &st), 0);
    // File should be owner-only read/write (0600)
    mode_t perms = st.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO);
    ASSERT_EQ(perms, static_cast<mode_t>(S_IRUSR | S_IWUSR));
}
