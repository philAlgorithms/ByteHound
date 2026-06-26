#include "test_harness.hpp"

#include "tinycfg/limits.hpp"
#include "tinycfg/tinycfg.hpp"

#include <limits>

TEST(parser_minimal_section) {
    tinycfg::Document doc;
    const auto result = tinycfg::parse("server { host = \"localhost\"; }", doc);
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(doc.size(), 1U);
    EXPECT_EQ(doc.at("server").at("host").as_string(), "localhost");
}

TEST(parser_nested_object_and_array) {
    const char* input = R"(cfg {
        tags = ["api", "production"];
        limits = { requests = 5000; timeout_ms = 2500; };
    })";
    tinycfg::Document doc;
    const auto result = tinycfg::parse(input, doc);
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(doc.at("cfg").at("tags").as_array().size(), 2U);
    EXPECT_EQ(doc.at("cfg").at("limits").as_object().at("requests").as_int(), 5000);
}

TEST(parser_duplicate_keys_last_wins) {
    tinycfg::Document doc;
    const auto result = tinycfg::parse("s { x = 1; x = 2; }", doc);
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(doc.at("s").at("x").as_int(), 2);
}

TEST(parser_integer_boundaries) {
    tinycfg::Document doc;
    const auto min_result =
        tinycfg::parse("s { v = -9223372036854775808; }", doc);
    EXPECT_TRUE(min_result.ok());
    EXPECT_EQ(doc.at("s").at("v").as_int(), std::numeric_limits<std::int64_t>::min());

    const auto max_result =
        tinycfg::parse("s { v = 9223372036854775807; }", doc);
    EXPECT_TRUE(max_result.ok());
    EXPECT_EQ(doc.at("s").at("v").as_int(), std::numeric_limits<std::int64_t>::max());
}

TEST(parser_integer_overflow_is_error) {
    tinycfg::Document doc;
    const auto result = tinycfg::parse("s { v = 9223372036854775808; }", doc);
    EXPECT_TRUE(result.failed());
}

TEST(parser_missing_semicolon_is_error) {
    tinycfg::Document doc;
    const auto result = tinycfg::parse("s { x = 1 }", doc);
    EXPECT_TRUE(result.failed());
}

TEST(parser_nested_depth_propagates_to_inner_assignments) {
    std::string input = "s { outer = ";
    for (int i = 0; i <= tinycfg::Limits::kMaxNestingDepth; ++i) {
        input += "{ k = ";
    }
    input += "1";
    for (int i = 0; i <= tinycfg::Limits::kMaxNestingDepth; ++i) {
        input += "; }";
    }
    input += "; }";
    tinycfg::Document doc;
    const auto result = tinycfg::parse(input, doc);
    EXPECT_TRUE(result.failed());
}

TEST(parser_empty_file_is_valid) {
    tinycfg::Document doc;
    const auto result = tinycfg::parse("", doc);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(doc.empty());
}

TEST(parser_whitespace_only_is_valid) {
    tinycfg::Document doc;
    const auto result = tinycfg::parse("  \n\t  # comment\n", doc);
    EXPECT_TRUE(result.ok());
}

TEST(parser_null_and_boolean_literals) {
    tinycfg::Document doc;
    const auto result = tinycfg::parse("s { a = true; b = false; c = null; }", doc);
    EXPECT_TRUE(result.ok());
    EXPECT_TRUE(doc.at("s").at("a").as_bool());
    EXPECT_FALSE(doc.at("s").at("b").as_bool());
    EXPECT_TRUE(doc.at("s").at("c").is_null());
}

TEST(parser_float_values) {
    tinycfg::Document doc;
    const auto result = tinycfg::parse("s { pi = 3.14; exp = 1e3; }", doc);
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(doc.at("s").at("pi").as_float(), 3.14);
    EXPECT_EQ(doc.at("s").at("exp").as_float(), 1000.0);
}

TEST(parser_error_includes_location) {
    tinycfg::Document doc;
    const auto result = tinycfg::parse("s {\n  x = ;\n}", doc);
    EXPECT_TRUE(result.failed());
    EXPECT_EQ(result.error().location.line, 2U);
}
