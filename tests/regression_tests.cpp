#include "test_harness.hpp"

#include "tinycfg/tinycfg.hpp"

#include <fstream>
#include <sstream>

namespace {

std::string read_data_file(const char* relative) {
    std::string path = std::string(TINYCFG_SOURCE_DIR) + "/" + relative;
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error(std::string("missing fixture: ") + relative);
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

}  // namespace

TEST(regression_trailing_comma_in_array) {
    tinycfg::Document doc;
    const auto result = tinycfg::parse("s { a = [1,]; }", doc);
    EXPECT_TRUE(result.failed());
}

TEST(regression_oversized_string_limit) {
    std::string big;
    big.push_back('"');
    big.append(tinycfg::Limits::kMaxStringBytes + 1, 'a');
    big.push_back('"');
    std::vector<tinycfg::Token> tokens;
    const auto result = tinycfg::tokenize(big, tokens);
    EXPECT_TRUE(result.failed());
}

TEST(regression_sample_valid_config) {
    const std::string input = read_data_file("tests/data/valid_server.cfg");
    tinycfg::Document doc;
    EXPECT_TRUE(tinycfg::parse(input, doc).ok());
    std::string serialized;
    EXPECT_TRUE(tinycfg::serialize(doc, serialized).ok());
    tinycfg::Document roundtrip;
    EXPECT_TRUE(tinycfg::parse(serialized, roundtrip).ok());
    EXPECT_TRUE(tinycfg::structural_equal(doc, roundtrip));
}

TEST(regression_invalid_missing_brace) {
    const std::string input = read_data_file("tests/data/invalid_missing_brace.cfg");
    tinycfg::Document doc;
    EXPECT_TRUE(tinycfg::parse(input, doc).failed());
}
