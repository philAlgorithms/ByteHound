#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace tinycfg {

struct SourceLocation {
    std::size_t line{1};
    std::size_t column{1};
    std::size_t byte_offset{0};
};

struct ParseError {
    std::string message;
    SourceLocation location{};
};

class ParseResult {
public:
    static ParseResult success() { return ParseResult{}; }

    static ParseResult fail(std::string message, SourceLocation location) {
        ParseResult result;
        result.error_ = ParseError{std::move(message), location};
        return result;
    }

    [[nodiscard]] bool ok() const { return !error_.has_value(); }
    [[nodiscard]] bool failed() const { return error_.has_value(); }
    [[nodiscard]] const ParseError& error() const { return *error_; }

private:
    std::optional<ParseError> error_;
};

}  // namespace tinycfg
