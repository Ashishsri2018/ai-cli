#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>

namespace ai::testing {

struct TestCase {
    std::string name;
    std::function<void()> func;
};

inline std::vector<TestCase>& get_tests() {
    static std::vector<TestCase> tests;
    return tests;
}

inline void register_test(const std::string& name, std::function<void()> func) {
    get_tests().push_back({name, std::move(func)});
}

#define AI_TEST(name) \
    static void test_func_##name(); \
    static struct AutoReg_##name { \
        AutoReg_##name() { ::ai::testing::register_test(#name, test_func_##name); } \
    } auto_reg_##name; \
    static void test_func_##name()

#define ASSERT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            std::cerr << "Assertion failed at " << __FILE__ << ":" << __LINE__ << ": " #cond << "\n"; \
            throw std::runtime_error("ASSERT_TRUE failed: " #cond); \
        } \
    } while(0)

#define ASSERT_FALSE(cond) ASSERT_TRUE(!(cond))

#define ASSERT_EQ(a, b) \
    do { \
        if ((a) != (b)) { \
            std::cerr << "Assertion failed at " << __FILE__ << ":" << __LINE__ << ": (" #a " == " #b ") with values: " << (a) << " vs " << (b) << "\n"; \
            throw std::runtime_error("ASSERT_EQ failed"); \
        } \
    } while(0)

#define ASSERT_STREQ(a, b) ASSERT_EQ(std::string(a), std::string(b))

} // namespace ai::testing
