#pragma once
/// @file test_main.h
/// @brief Minimal zero-dependency test framework for libew.

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string_view>

namespace test {
    inline int g_pass = 0;
    inline int g_fail = 0;

    inline void pass(std::string_view expr) {
        ++g_pass;
        std::cout << "  PASS  " << expr << "\n";
    }
    inline void fail(std::string_view expr, std::string_view file, int line, std::string_view detail = "") {
        ++g_fail;
        std::cout << "  FAIL  " << expr << "  [" << file << ":" << line << "]";
        if (!detail.empty()) std::cout << "  (" << detail << ")";
        std::cout << "\n";
    }
    inline int summary() {
        std::cout << "\n" << g_pass << " passed, " << g_fail << " failed\n";
        return g_fail > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
    }
} // namespace test

#define ASSERT_TRUE(expr) \
    do { \
        if (expr) test::pass(#expr); \
        else      test::fail(#expr, __FILE__, __LINE__); \
    } while(false)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))

#define ASSERT_NEAR(actual, expected, tol) \
    do { \
        const double _a = static_cast<double>(actual); \
        const double _e = static_cast<double>(expected); \
        const double _t = static_cast<double>(tol); \
        const double _d = std::abs(_a - _e); \
        if (_d <= _t) { \
            test::pass(#actual " ~= " #expected); \
        } else { \
            char _buf[128]; \
            std::snprintf(_buf, sizeof(_buf), "actual=%.6g expected=%.6g diff=%.6g tol=%.6g", _a, _e, _d, _t); \
            test::fail(#actual " ~= " #expected, __FILE__, __LINE__, _buf); \
        } \
    } while(false)

#define ASSERT_EQ(actual, expected) ASSERT_NEAR(actual, expected, 1e-9)

#define RUN_TEST(fn) \
    do { std::cout << "\n[" #fn "]\n"; fn(); } while(false)

#define TEST_MAIN() \
    int main()      \
    { std::cout << "=== " __FILE__ " ===\n";
// Usage: TEST_MAIN() { RUN_TEST(my_test); return test::summary(); }
