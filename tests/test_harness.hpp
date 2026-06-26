#pragma once

#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

struct TestCase {
    std::string name;
    std::function<void()> body;
};

inline std::vector<TestCase>& test_registry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct TestRegistrar {
    TestRegistrar(std::string name, std::function<void()> body) {
        test_registry().push_back(TestCase{std::move(name), std::move(body)});
    }
};

#define TEST(name)                                         \
    void test_##name();                                    \
    static TestRegistrar registrar_##name(#name, test_##name); \
    void test_##name()

#define EXPECT_TRUE(expr)                                                     \
    do {                                                                      \
        if (!(expr)) {                                                        \
            throw std::runtime_error(std::string("EXPECT_TRUE failed: ") + #expr); \
        }                                                                     \
    } while (false)

#define EXPECT_FALSE(expr) EXPECT_TRUE(!(expr))

#define EXPECT_EQ(lhs, rhs)                                                   \
    do {                                                                      \
        if ((lhs) != (rhs)) {                                                 \
            throw std::runtime_error(std::string("EXPECT_EQ failed: ") + #lhs " vs " #rhs); \
        }                                                                     \
    } while (false)

#define EXPECT_NE(lhs, rhs)                                                   \
    do {                                                                      \
        if ((lhs) == (rhs)) {                                                 \
            throw std::runtime_error(std::string("EXPECT_NE failed: ") + #lhs " vs " #rhs); \
        }                                                                     \
    } while (false)

inline int run_all_tests() {
    int failures = 0;
    for (const auto& test : test_registry()) {
        try {
            test.body();
            std::cout << "[ PASS ] " << test.name << '\n';
        } catch (const std::exception& ex) {
            std::cerr << "[ FAIL ] " << test.name << ": " << ex.what() << '\n';
            failures++;
        }
    }
    const auto passed = test_registry().size() - static_cast<std::size_t>(failures);
    std::cout << failures << " failed, " << passed << " passed\n";
    return failures == 0 ? 0 : 1;
}
