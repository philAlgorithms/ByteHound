#include "tinycfg/serializer.hpp"

#include <array>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace tinycfg {

namespace {

void append_indent(std::string& output, int indent) {
    output.append(static_cast<std::size_t>(indent) * 2, ' ');
}

void append_escaped_string(std::string& output, std::string_view value) {
    output.push_back('"');
    for (const char ch : value) {
        switch (ch) {
            case '"':
                output.append("\\\"");
                break;
            case '\\':
                output.append("\\\\");
                break;
            case '\n':
                output.append("\\n");
                break;
            case '\t':
                output.append("\\t");
                break;
            case '\r':
                output.append("\\r");
                break;
            case '\0':
                output.append("\\0");
                break;
            default:
                output.push_back(ch);
                break;
        }
    }
    output.push_back('"');
}

[[nodiscard]] ParseResult append_int(std::string& output, std::int64_t value) {
    std::array<char, 32> buffer{};
    const auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value);
    if (result.ec != std::errc{}) {
        return ParseResult::fail("failed to serialize integer", SourceLocation{});
    }
    output.append(buffer.data(), static_cast<std::size_t>(result.ptr - buffer.data()));
    return ParseResult::success();
}

[[nodiscard]] ParseResult append_float(std::string& output, double value) {
    if (!std::isfinite(value)) {
        return ParseResult::fail("non-finite float cannot be serialized", SourceLocation{});
    }
    std::ostringstream stream;
    stream << std::setprecision(17) << value;
    const std::string text = stream.str();
    if (text.find('e') == std::string::npos && text.find('E') == std::string::npos &&
        text.find('.') == std::string::npos) {
        output.append(text);
        output.append(".0");
    } else {
        output.append(text);
    }
    return ParseResult::success();
}

[[nodiscard]] ParseResult serialize_object_body(const Object& object, std::string& output,
                                                int indent);

[[nodiscard]] ParseResult serialize_value_impl(const Value& value, std::string& output,
                                               int indent) {
    if (value.is_null()) {
        output.append("null");
        return ParseResult::success();
    }
    if (value.is_bool()) {
        output.append(value.as_bool() ? "true" : "false");
        return ParseResult::success();
    }
    if (value.is_int()) {
        return append_int(output, value.as_int());
    }
    if (value.is_float()) {
        return append_float(output, value.as_float());
    }
    if (value.is_string()) {
        append_escaped_string(output, value.as_string());
        return ParseResult::success();
    }
    if (value.is_array()) {
        output.push_back('[');
        const auto& elements = value.as_array();
        for (std::size_t i = 0; i < elements.size(); ++i) {
            if (i > 0) {
                output.append(", ");
            }
            if (auto result = serialize_value_impl(elements[i], output, indent); result.failed()) {
                return result;
            }
        }
        output.push_back(']');
        return ParseResult::success();
    }
    if (value.is_object()) {
        output.push_back('{');
        output.push_back('\n');
        if (auto result = serialize_object_body(value.as_object(), output, indent + 1);
            result.failed()) {
            return result;
        }
        append_indent(output, indent);
        output.push_back('}');
        return ParseResult::success();
    }
    return ParseResult::fail("unsupported value type", SourceLocation{});
}

[[nodiscard]] ParseResult serialize_object_body(const Object& object, std::string& output,
                                                int indent) {
    bool first = true;
    for (const auto& [key, child] : object) {
        if (!first) {
            output.push_back('\n');
        }
        first = false;
        append_indent(output, indent);
        output.append(key);
        output.append(" = ");
        if (auto result = serialize_value_impl(child, output, indent); result.failed()) {
            return result;
        }
        output.push_back(';');
    }
    if (!first) {
        output.push_back('\n');
    }
    return ParseResult::success();
}

}  // namespace

ParseResult Serializer::serialize_value(const Value& value, std::string& output, int indent) {
    return serialize_value_impl(value, output, indent);
}

ParseResult Serializer::serialize(const Document& document, std::string& output) {
    output.clear();
    bool first_section = true;
    for (const auto& [name, section] : document) {
        if (!first_section) {
            output.push_back('\n');
        }
        first_section = false;
        output.append(name);
        output.append(" {\n");
        if (auto result = serialize_object_body(section, output, 1); result.failed()) {
            return result;
        }
        output.append("}\n");
    }
    return ParseResult::success();
}

}  // namespace tinycfg
