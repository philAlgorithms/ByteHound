#include "test_harness.hpp"

#include "tinycfg/tinycfg.hpp"

TEST(lexer_empty_input_produces_eof) {
    std::vector<tinycfg::Token> tokens;
    const auto result = tinycfg::tokenize("", tokens);
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(tokens.size(), 1U);
    EXPECT_EQ(tokens[0].type, tinycfg::TokenType::kEndOfFile);
}

TEST(lexer_comments_are_skipped) {
    std::vector<tinycfg::Token> tokens;
    const auto result = tinycfg::tokenize("# comment\n// line\nhost", tokens);
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(tokens[0].type, tinycfg::TokenType::kIdentifier);
    EXPECT_EQ(tokens[0].lexeme, "host");
}

TEST(lexer_string_escape_sequences) {
    std::vector<tinycfg::Token> tokens;
    const auto result = tinycfg::tokenize(R"("a\n\t\\")", tokens);
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(tokens[0].type, tinycfg::TokenType::kString);
    EXPECT_EQ(tokens[0].lexeme, "a\n\t\\");
}

TEST(lexer_invalid_escape_is_error) {
    std::vector<tinycfg::Token> tokens;
    const auto result = tinycfg::tokenize(R"("\q")", tokens);
    EXPECT_TRUE(result.failed());
}

TEST(lexer_embedded_nul_bytes) {
    std::vector<tinycfg::Token> tokens;
    const std::string input = std::string("\"\x00\"", 3);
    const auto result = tinycfg::tokenize(input, tokens);
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(tokens[0].lexeme.size(), 1U);
    EXPECT_EQ(tokens[0].lexeme[0], '\0');
}
