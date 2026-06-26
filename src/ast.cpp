#include "tinycfg/ast.hpp"

#include <sstream>

namespace tinycfg {

namespace {

std::string indent_string(int indent) {
    return std::string(static_cast<std::size_t>(indent) * 2, ' ');
}

}  // namespace

bool structural_equal(const Value& lhs, const Value& rhs) {
    if (lhs.data.index() != rhs.data.index()) {
        return false;
    }
    return std::visit(
        [&rhs](const auto& left) -> bool {
            using T = std::decay_t<decltype(left)>;
            const auto& right = std::get<T>(rhs.data);
            if constexpr (std::is_same_v<T, NullValue>) {
                return true;
            } else if constexpr (std::is_same_v<T, BoolValue> || std::is_same_v<T, IntValue> ||
                                 std::is_same_v<T, FloatValue>) {
                return left.value == right.value;
            } else if constexpr (std::is_same_v<T, StringValue>) {
                return left.value == right.value;
            } else if constexpr (std::is_same_v<T, ArrayValue>) {
                if (left.elements.size() != right.elements.size()) {
                    return false;
                }
                for (std::size_t i = 0; i < left.elements.size(); ++i) {
                    if (!structural_equal(left.elements[i], right.elements[i])) {
                        return false;
                    }
                }
                return true;
            } else if constexpr (std::is_same_v<T, ObjectValue>) {
                return structural_equal(left.properties, right.properties);
            }
            return false;
        },
        lhs.data);
}

bool structural_equal(const Object& lhs, const Object& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (const auto& [key, value] : lhs) {
        const auto it = rhs.find(key);
        if (it == rhs.end() || !structural_equal(value, it->second)) {
            return false;
        }
    }
    return true;
}

bool structural_equal(const Document& lhs, const Document& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (const auto& [name, section] : lhs) {
        const auto it = rhs.find(name);
        if (it == rhs.end() || !structural_equal(section, it->second)) {
            return false;
        }
    }
    return true;
}

std::string format_value(const Value& value, int indent) {
    std::ostringstream out;
    const std::string pad = indent_string(indent);

    if (value.is_null()) {
        out << "null";
    } else if (value.is_bool()) {
        out << (value.as_bool() ? "true" : "false");
    } else if (value.is_int()) {
        out << value.as_int();
    } else if (value.is_float()) {
        out << value.as_float();
    } else if (value.is_string()) {
        out << '"' << value.as_string() << '"';
    } else if (value.is_array()) {
        out << "[\n";
        const auto& elements = value.as_array();
        for (std::size_t i = 0; i < elements.size(); ++i) {
            out << indent_string(indent + 1) << format_value(elements[i], indent + 1);
            if (i + 1 < elements.size()) {
                out << ',';
            }
            out << '\n';
        }
        out << pad << ']';
    } else if (value.is_object()) {
        out << "{\n";
        bool first = true;
        for (const auto& [key, child] : value.as_object()) {
            if (!first) {
                out << '\n';
            }
            first = false;
            out << indent_string(indent + 1) << key << " = " << format_value(child, indent + 1)
                << ';';
        }
        out << '\n' << pad << '}';
    }
    return out.str();
}

std::string format_ast(const Document& document) {
    std::ostringstream out;
    bool first_section = true;
    for (const auto& [name, section] : document) {
        if (!first_section) {
            out << '\n';
        }
        first_section = false;
        out << name << " {\n";
        bool first = true;
        for (const auto& [key, value] : section) {
            if (!first) {
                out << '\n';
            }
            first = false;
            out << "  " << key << " = " << format_value(value, 1) << ';';
        }
        out << "\n}\n";
    }
    return out.str();
}

}  // namespace tinycfg
