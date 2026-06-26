#include "tinycfg/lexer.hpp"

#include "tinycfg/limits.hpp"

#include <cctype>

namespace tinycfg {

Lexer::Lexer(std::string_view input) : input_(input) {}

bool Lexer::at_end() const { return index_ >= input_.size(); }

char Lexer::peek() const {
    if (at_end()) {
        return '\0';
    }
    return input_[index_];
}

char Lexer::advance() {
    if (at_end()) {
        return '\0';
    }
    const char ch = input_[index_++];
    location_.byte_offset = index_;
    if (ch == '\n') {
        location_.line++;
        location_.column = 1;
    } else {
        location_.column++;
    }
    return ch;
}

void Lexer::skip_whitespace_and_comments() {
    while (!at_end()) {
        const char ch = peek();
        if (std::isspace(static_cast<unsigned char>(ch)) != 0) {
            advance();
            continue;
        }
        if (ch == '#') {
            while (!at_end() && peek() != '\n') {
                advance();
            }
            continue;
        }
        if (ch == '/' && index_ + 1 < input_.size() && input_[index_ + 1] == '/') {
            advance();
            advance();
            while (!at_end() && peek() != '\n') {
                advance();
            }
            continue;
        }
        break;
    }
}

ParseResult Lexer::make_error(std::string message) const {
    return ParseResult::fail(std::move(message), location_);
}

ParseResult Lexer::add_token(TokenType type, std::string lexeme, Token& token) {
    if (token_count_ >= Limits::kMaxTokenCount) {
        return make_error("token count exceeds maximum");
    }
    token.type = type;
    token.lexeme = std::move(lexeme);
    token.location = location_;
    token_count_++;
    return ParseResult::success();
}

ParseResult Lexer::scan_identifier_or_keyword(Token& token) {
    const SourceLocation start = location_;
    std::string lexeme;
    while (!at_end()) {
        const char ch = peek();
        if (std::isalnum(static_cast<unsigned char>(ch)) != 0 || ch == '_') {
            lexeme.push_back(advance());
        } else {
            break;
        }
    }
    if (lexeme.size() > Limits::kMaxIdentifierBytes) {
        return ParseResult::fail("identifier exceeds maximum length", start);
    }

    if (lexeme == "true") {
        return add_token(TokenType::kTrue, lexeme, token);
    }
    if (lexeme == "false") {
        return add_token(TokenType::kFalse, lexeme, token);
    }
    if (lexeme == "null") {
        return add_token(TokenType::kNull, lexeme, token);
    }
    return add_token(TokenType::kIdentifier, std::move(lexeme), token);
}

ParseResult Lexer::scan_string(Token& token) {
    const SourceLocation start = location_;
    advance();  // opening quote
    std::string value;
    while (!at_end()) {
        const char ch = peek();
        if (ch == '"') {
            advance();
            if (value.size() > Limits::kMaxStringBytes) {
                return ParseResult::fail("string exceeds maximum length", start);
            }
            return add_token(TokenType::kString, std::move(value), token);
        }
        if (ch == '\n') {
            return ParseResult::fail("unterminated string", start);
        }
        if (ch == '\\') {
            advance();
            if (at_end()) {
                return ParseResult::fail("unterminated escape sequence", start);
            }
            const char esc = advance();
            switch (esc) {
                case '"':
                    value.push_back('"');
                    break;
                case '\\':
                    value.push_back('\\');
                    break;
                case 'n':
                    value.push_back('\n');
                    break;
                case 't':
                    value.push_back('\t');
                    break;
                case 'r':
                    value.push_back('\r');
                    break;
                case '0':
                    value.push_back('\0');
                    break;
                default:
                    return ParseResult::fail("invalid escape sequence", start);
            }
            continue;
        }
        value.push_back(advance());
    }
    return ParseResult::fail("unterminated string", start);
}

ParseResult Lexer::scan_number(Token& token) {
    const SourceLocation start = location_;
    std::string lexeme;
    bool is_float = false;

    if (peek() == '-') {
        lexeme.push_back(advance());
    }
    if (at_end()) {
        return ParseResult::fail("invalid number", start);
    }
    if (peek() == '0') {
        lexeme.push_back(advance());
    } else {
        if (std::isdigit(static_cast<unsigned char>(peek())) == 0) {
            return ParseResult::fail("invalid number", start);
        }
        while (!at_end() && std::isdigit(static_cast<unsigned char>(peek())) != 0) {
            lexeme.push_back(advance());
        }
    }

    if (!at_end() && peek() == '.') {
        is_float = true;
        lexeme.push_back(advance());
        if (at_end() || std::isdigit(static_cast<unsigned char>(peek())) == 0) {
            return ParseResult::fail("invalid floating-point number", start);
        }
        while (!at_end() && std::isdigit(static_cast<unsigned char>(peek())) != 0) {
            lexeme.push_back(advance());
        }
    }

    if (!at_end() && (peek() == 'e' || peek() == 'E')) {
        is_float = true;
        lexeme.push_back(advance());
        if (!at_end() && (peek() == '+' || peek() == '-')) {
            lexeme.push_back(advance());
        }
        if (at_end() || std::isdigit(static_cast<unsigned char>(peek())) == 0) {
            return ParseResult::fail("invalid floating-point exponent", start);
        }
        while (!at_end() && std::isdigit(static_cast<unsigned char>(peek())) != 0) {
            lexeme.push_back(advance());
        }
    }

    if (is_float) {
        return add_token(TokenType::kFloat, std::move(lexeme), token);
    }
    return add_token(TokenType::kInteger, std::move(lexeme), token);
}

ParseResult Lexer::tokenize(std::vector<Token>& tokens) {
    tokens.clear();
    while (!at_end()) {
        skip_whitespace_and_comments();
        if (at_end()) {
            break;
        }

        Token token;
        const char ch = peek();

        switch (ch) {
            case '{':
                advance();
                if (auto result = add_token(TokenType::kLeftBrace, "{", token); result.failed()) {
                    return result;
                }
                break;
            case '}':
                advance();
                if (auto result = add_token(TokenType::kRightBrace, "}", token); result.failed()) {
                    return result;
                }
                break;
            case '[':
                advance();
                if (auto result = add_token(TokenType::kLeftBracket, "[", token); result.failed()) {
                    return result;
                }
                break;
            case ']':
                advance();
                if (auto result = add_token(TokenType::kRightBracket, "]", token); result.failed()) {
                    return result;
                }
                break;
            case '=':
                advance();
                if (auto result = add_token(TokenType::kEqual, "=", token); result.failed()) {
                    return result;
                }
                break;
            case ';':
                advance();
                if (auto result = add_token(TokenType::kSemicolon, ";", token); result.failed()) {
                    return result;
                }
                break;
            case ',':
                advance();
                if (auto result = add_token(TokenType::kComma, ",", token); result.failed()) {
                    return result;
                }
                break;
            case '"':
                if (auto result = scan_string(token); result.failed()) {
                    return result;
                }
                break;
            default:
                if (std::isalpha(static_cast<unsigned char>(ch)) != 0 || ch == '_') {
                    if (auto result = scan_identifier_or_keyword(token); result.failed()) {
                        return result;
                    }
                } else if (std::isdigit(static_cast<unsigned char>(ch)) != 0 || ch == '-') {
                    if (auto result = scan_number(token); result.failed()) {
                        return result;
                    }
                } else {
                    return make_error("unexpected character");
                }
                break;
        }
        tokens.push_back(std::move(token));
    }

    Token eof_token;
    eof_token.type = TokenType::kEndOfFile;
    eof_token.location = location_;
    tokens.push_back(std::move(eof_token));
    return ParseResult::success();
}

}  // namespace tinycfg
