#include "tinycfg/tinycfg.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr int kExitSuccess = 0;
constexpr int kExitUsage = 1;
constexpr int kExitIo = 2;
constexpr int kExitParse = 3;

void print_usage() {
    std::cerr << "Usage:\n"
              << "  tinycfg-cli parse <file>    Print readable AST\n"
              << "  tinycfg-cli validate <file> Exit 0 for valid input\n"
              << "  tinycfg-cli format <file>   Print canonical TinyCFG\n"
              << "  tinycfg-cli tokens <file>   Print token stream\n"
              << "\nExit codes:\n"
              << "  0 success\n"
              << "  1 usage error\n"
              << "  2 I/O error\n"
              << "  3 parse/validation error\n";
}

[[nodiscard]] bool read_file(const char* path, std::string& contents) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        std::cerr << "failed to open file: " << path << '\n';
        return false;
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    contents = buffer.str();
    return true;
}

const char* token_type_name(tinycfg::TokenType type) {
    using tinycfg::TokenType;
    switch (type) {
        case TokenType::kIdentifier:
            return "identifier";
        case TokenType::kString:
            return "string";
        case TokenType::kInteger:
            return "integer";
        case TokenType::kFloat:
            return "float";
        case TokenType::kTrue:
            return "true";
        case TokenType::kFalse:
            return "false";
        case TokenType::kNull:
            return "null";
        case TokenType::kLeftBrace:
            return "{";
        case TokenType::kRightBrace:
            return "}";
        case TokenType::kLeftBracket:
            return "[";
        case TokenType::kRightBracket:
            return "]";
        case TokenType::kEqual:
            return "=";
        case TokenType::kSemicolon:
            return ";";
        case TokenType::kComma:
            return ",";
        case TokenType::kEndOfFile:
            return "eof";
    }
    return "unknown";
}

int cmd_tokens(const std::string& contents) {
    std::vector<tinycfg::Token> tokens;
    const auto result = tinycfg::tokenize(contents, tokens);
    if (result.failed()) {
        const auto& error = result.error();
        std::cerr << error.message << " at line " << error.location.line << ", column "
                  << error.location.column << '\n';
        return kExitParse;
    }
    for (const auto& token : tokens) {
        std::cout << token_type_name(token.type) << '\t' << token.lexeme << '\t' << token.location.line
                  << '\t' << token.location.column << '\n';
    }
    return kExitSuccess;
}

int cmd_parse(const std::string& contents) {
    tinycfg::Document document;
    const auto result = tinycfg::parse(contents, document);
    if (result.failed()) {
        const auto& error = result.error();
        std::cerr << error.message << " at line " << error.location.line << ", column "
                  << error.location.column << '\n';
        return kExitParse;
    }
    std::cout << tinycfg::format_ast(document);
    return kExitSuccess;
}

int cmd_validate(const std::string& contents) {
    tinycfg::Document document;
    const auto result = tinycfg::parse(contents, document);
    if (result.failed()) {
        const auto& error = result.error();
        std::cerr << error.message << " at line " << error.location.line << ", column "
                  << error.location.column << '\n';
        return kExitParse;
    }
    return kExitSuccess;
}

int cmd_format(const std::string& contents) {
    tinycfg::Document document;
    const auto parse_result = tinycfg::parse(contents, document);
    if (parse_result.failed()) {
        const auto& error = parse_result.error();
        std::cerr << error.message << " at line " << error.location.line << ", column "
                  << error.location.column << '\n';
        return kExitParse;
    }
    std::string output;
    const auto serialize_result = tinycfg::serialize(document, output);
    if (serialize_result.failed()) {
        std::cerr << serialize_result.error().message << '\n';
        return kExitParse;
    }
    std::cout << output;
    return kExitSuccess;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 3) {
        print_usage();
        return kExitUsage;
    }

    const std::string_view command = argv[1];
    const char* path = argv[2];

    std::string contents;
    if (!read_file(path, contents)) {
        return kExitIo;
    }

    if (command == "parse") {
        return cmd_parse(contents);
    }
    if (command == "validate") {
        return cmd_validate(contents);
    }
    if (command == "format") {
        return cmd_format(contents);
    }
    if (command == "tokens") {
        return cmd_tokens(contents);
    }

    print_usage();
    return kExitUsage;
}
