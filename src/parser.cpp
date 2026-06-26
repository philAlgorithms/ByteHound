#include "tinycfg/parser.hpp"

#include "tinycfg/limits.hpp"

#include <charconv>
#include <cmath>
#include <limits>

namespace tinycfg {

namespace {

[[nodiscard]] ParseResult parse_int64(std::string_view text, std::int64_t& out) {
    if (text.empty()) {
        return ParseResult::fail("invalid integer", SourceLocation{});
    }
    std::int64_t value = 0;
    const auto* begin = text.data();
    const auto* end = begin + text.size();
    const auto result = std::from_chars(begin, end, value);
    if (result.ec != std::errc{} || result.ptr != end) {
        return ParseResult::fail("invalid integer", SourceLocation{});
    }
    out = value;
    return ParseResult::success();
}

[[nodiscard]] ParseResult parse_double(std::string_view text, double& out) {
    if (text.empty()) {
        return ParseResult::fail("invalid floating-point number", SourceLocation{});
    }
    try {
        std::size_t consumed = 0;
        const std::string str(text);
        out = std::stod(str, &consumed);
        if (consumed != text.size()) {
            return ParseResult::fail("invalid floating-point number", SourceLocation{});
        }
        if (!std::isfinite(out)) {
            return ParseResult::fail("floating-point number out of range", SourceLocation{});
        }
    } catch (const std::exception&) {
        return ParseResult::fail("invalid floating-point number", SourceLocation{});
    }
    return ParseResult::success();
}

}  // namespace

Parser::Parser(std::string_view input) : input_(input) {}

ParseResult Parser::parse(Document& document) {
    Lexer lexer(input_);
    if (auto result = lexer.tokenize(tokens_); result.failed()) {
        return result;
    }
    current_ = 0;
    return parse_document(document);
}

const Token& Parser::current() const { return tokens_.at(current_); }

const Token& Parser::previous() const { return tokens_.at(current_ - 1); }

bool Parser::check(TokenType type) const { return current().type == type; }

bool Parser::at_end() const { return check(TokenType::kEndOfFile); }

ParseResult Parser::advance() {
    if (!at_end()) {
        current_++;
    }
    return ParseResult::success();
}

ParseResult Parser::consume(TokenType type, std::string_view message) {
    if (check(type)) {
        return advance();
    }
    return make_error_at(current(), std::string(message));
}

ParseResult Parser::make_error(std::string message) const {
    return make_error_at(current(), std::move(message));
}

ParseResult Parser::make_error_at(const Token& token, std::string message) const {
    return ParseResult::fail(std::move(message), token.location);
}

ParseResult Parser::parse_document(Document& document) {
    document.clear();
    while (!at_end()) {
        if (!check(TokenType::kIdentifier)) {
            return make_error("expected section name");
        }
        const std::string section_name = current().lexeme;
        if (auto result = advance(); result.failed()) {
            return result;
        }
        if (auto result = consume(TokenType::kLeftBrace, "expected '{' after section name");
            result.failed()) {
            return result;
        }

        Object section;
        if (auto result = parse_section(section); result.failed()) {
            return result;
        }
        document[section_name] = std::move(section);

        if (auto result = consume(TokenType::kRightBrace, "expected '}' to close section");
            result.failed()) {
            return result;
        }
    }
    return ParseResult::success();
}

ParseResult Parser::parse_section(Object& section) {
    section.clear();
    while (!check(TokenType::kRightBrace) && !at_end()) {
        if (auto result = parse_assignment(section, 0); result.failed()) {
            return result;
        }
    }
    return ParseResult::success();
}

ParseResult Parser::parse_assignment(Object& object, int depth) {
    if (!check(TokenType::kIdentifier)) {
        return make_error("expected property name");
    }
    const std::string key = current().lexeme;
    if (auto result = advance(); result.failed()) {
        return result;
    }
    if (auto result = consume(TokenType::kEqual, "expected '=' after property name");
        result.failed()) {
        return result;
    }

    Value value;
    if (auto result = parse_value(value, depth); result.failed()) {
        return result;
    }
    // Duplicate keys: last assignment wins.
    object[key] = std::move(value);

    if (auto result = consume(TokenType::kSemicolon, "expected ';' after value"); result.failed()) {
        return result;
    }
    return ParseResult::success();
}

ParseResult Parser::parse_value(Value& value, int depth) {
    if (depth > Limits::kMaxNestingDepth) {
        return make_error("nesting depth exceeds maximum");
    }

    switch (current().type) {
        case TokenType::kTrue:
            value = Value{BoolValue{true}};
            return advance();
        case TokenType::kFalse:
            value = Value{BoolValue{false}};
            return advance();
        case TokenType::kNull:
            value = Value{NullValue{}};
            return advance();
        case TokenType::kString:
            value = Value{StringValue{current().lexeme}};
            return advance();
        case TokenType::kInteger: {
            std::int64_t parsed = 0;
            if (auto result = parse_int64(current().lexeme, parsed); result.failed()) {
                return make_error_at(current(), result.error().message);
            }
            value = Value{IntValue{parsed}};
            return advance();
        }
        case TokenType::kFloat: {
            double parsed = 0.0;
            if (auto result = parse_double(current().lexeme, parsed); result.failed()) {
                return make_error_at(current(), result.error().message);
            }
            value = Value{FloatValue{parsed}};
            return advance();
        }
        case TokenType::kLeftBracket: {
            Array array;
            if (auto result = parse_array(array, depth + 1); result.failed()) {
                return result;
            }
            value = Value{ArrayValue{std::move(array)}};
            return ParseResult::success();
        }
        case TokenType::kLeftBrace: {
            Object object;
            if (auto result = parse_object(object, depth + 1); result.failed()) {
                return result;
            }
            value = Value{ObjectValue{std::move(object)}};
            return ParseResult::success();
        }
        default:
            return make_error("expected value");
    }
}

ParseResult Parser::parse_array(Array& array, int depth) {
    if (auto result = consume(TokenType::kLeftBracket, "expected '['"); result.failed()) {
        return result;
    }
    array.clear();
    if (check(TokenType::kRightBracket)) {
        return advance();
    }

    while (!check(TokenType::kRightBracket) && !at_end()) {
        if (array.size() >= Limits::kMaxContainerEntries) {
            return make_error("array exceeds maximum entry count");
        }
        Value element;
        if (auto result = parse_value(element, depth); result.failed()) {
            return result;
        }
        array.push_back(std::move(element));

        if (check(TokenType::kComma)) {
            if (auto result = advance(); result.failed()) {
                return result;
            }
            if (check(TokenType::kRightBracket)) {
                return make_error("trailing comma in array");
            }
            continue;
        }
        break;
    }

    return consume(TokenType::kRightBracket, "expected ']' to close array");
}

ParseResult Parser::parse_object(Object& object, int depth) {
    if (depth > Limits::kMaxNestingDepth) {
        return make_error("nesting depth exceeds maximum");
    }
    if (auto result = consume(TokenType::kLeftBrace, "expected '{'"); result.failed()) {
        return result;
    }
    object.clear();
    if (check(TokenType::kRightBrace)) {
        return advance();
    }

    while (!check(TokenType::kRightBrace) && !at_end()) {
        if (object.size() >= Limits::kMaxContainerEntries) {
            return make_error("object exceeds maximum entry count");
        }
        if (auto result = parse_assignment(object, depth); result.failed()) {
            return result;
        }
    }

    return consume(TokenType::kRightBrace, "expected '}' to close object");
}

}  // namespace tinycfg
