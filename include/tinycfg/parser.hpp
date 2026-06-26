#pragma once

#include "tinycfg/ast.hpp"
#include "tinycfg/error.hpp"
#include "tinycfg/lexer.hpp"

#include <string_view>
#include <vector>

namespace tinycfg {

class Parser {
public:
    explicit Parser(std::string_view input);

    [[nodiscard]] ParseResult parse(Document& document);

private:
    [[nodiscard]] ParseResult parse_document(Document& document);
    [[nodiscard]] ParseResult parse_section(Object& section);
    [[nodiscard]] ParseResult parse_assignment(Object& object, int depth);
    [[nodiscard]] ParseResult parse_value(Value& value, int depth);
    [[nodiscard]] ParseResult parse_object(Object& object, int depth);
    [[nodiscard]] ParseResult parse_array(Array& array, int depth);

    [[nodiscard]] const Token& current() const;
    [[nodiscard]] const Token& previous() const;
    [[nodiscard]] bool check(TokenType type) const;
    [[nodiscard]] bool at_end() const;
    [[nodiscard]] ParseResult advance();
    [[nodiscard]] ParseResult consume(TokenType type, std::string_view message);
    [[nodiscard]] ParseResult make_error(std::string message) const;
    [[nodiscard]] ParseResult make_error_at(const Token& token, std::string message) const;

    std::vector<Token> tokens_;
    std::size_t current_{0};
    std::string_view input_;
};

}  // namespace tinycfg
