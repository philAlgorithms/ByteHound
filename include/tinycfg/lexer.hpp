#pragma once

#include "tinycfg/error.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace tinycfg {

enum class TokenType {
    kIdentifier,
    kString,
    kInteger,
    kFloat,
    kTrue,
    kFalse,
    kNull,
    kLeftBrace,
    kRightBrace,
    kLeftBracket,
    kRightBracket,
    kEqual,
    kSemicolon,
    kComma,
    kEndOfFile,
};

struct Token {
    TokenType type{TokenType::kEndOfFile};
    std::string lexeme;
    SourceLocation location{};
};

class Lexer {
public:
    explicit Lexer(std::string_view input);

    [[nodiscard]] ParseResult tokenize(std::vector<Token>& tokens);

private:
    [[nodiscard]] bool at_end() const;
    [[nodiscard]] char peek() const;
    char advance();
    void skip_whitespace_and_comments();
    [[nodiscard]] ParseResult scan_identifier_or_keyword(Token& token);
    [[nodiscard]] ParseResult scan_string(Token& token);
    [[nodiscard]] ParseResult scan_number(Token& token);
    [[nodiscard]] ParseResult add_token(TokenType type, std::string lexeme, Token& token);
    [[nodiscard]] ParseResult make_error(std::string message) const;

    std::string_view input_;
    std::size_t index_{0};
    SourceLocation location_{1, 1, 0};
    std::size_t token_count_{0};
};

}  // namespace tinycfg
