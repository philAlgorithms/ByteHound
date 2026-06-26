#pragma once

#include "tinycfg/ast.hpp"
#include "tinycfg/error.hpp"
#include "tinycfg/lexer.hpp"
#include "tinycfg/limits.hpp"
#include "tinycfg/parser.hpp"
#include "tinycfg/serializer.hpp"

namespace tinycfg {

[[nodiscard]] inline ParseResult parse(std::string_view input, Document& document) {
    if (input.size() > Limits::kMaxInputBytes) {
        return ParseResult::fail("input exceeds maximum size", SourceLocation{1, 1, 0});
    }
    Parser parser(input);
    return parser.parse(document);
}

[[nodiscard]] inline ParseResult tokenize(std::string_view input, std::vector<Token>& tokens) {
    if (input.size() > Limits::kMaxInputBytes) {
        return ParseResult::fail("input exceeds maximum size", SourceLocation{1, 1, 0});
    }
    Lexer lexer(input);
    return lexer.tokenize(tokens);
}

[[nodiscard]] inline ParseResult serialize(const Document& document, std::string& output) {
    return Serializer::serialize(document, output);
}

}  // namespace tinycfg
