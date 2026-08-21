#include "test_runner.hpp"
#include <iostream>

int main() {
    auto& tests = ai::testing::get_tests();
    std::cout << "Running " << tests.size() << " tests...\n";
    int passed = 0;
    int failed = 0;

    for (const auto& t : tests) {
        std::cout << "  [TEST] " << t.name << " ... ";
        try {
            t.func();
            std::cout << "PASSED\n";
            passed++;
        } catch (const std::exception& e) {
            std::cout << "FAILED: " << e.what() << "\n";
            failed++;
        } catch (...) {
            std::cout << "FAILED (unknown exception)\n";
            failed++;
        }
    }

    std::cout << "\nTest Results: " << passed << " passed, " << failed << " failed.\n";
    return failed == 0 ? 0 : 1;
}
