#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace tinycfg {

struct Value;

using Object = std::map<std::string, Value>;
using Array = std::vector<Value>;
using Document = std::map<std::string, Object>;

struct NullValue {};
struct BoolValue {
    bool value{};
};
struct IntValue {
    std::int64_t value{};
};
struct FloatValue {
    double value{};
};
struct StringValue {
    std::string value;
};
struct ArrayValue {
    Array elements;
};
struct ObjectValue {
    Object properties;
};

struct Value {
    std::variant<NullValue, BoolValue, IntValue, FloatValue, StringValue, ArrayValue,
                 ObjectValue>
        data{NullValue{}};

    [[nodiscard]] bool is_null() const { return std::holds_alternative<NullValue>(data); }
    [[nodiscard]] bool is_bool() const { return std::holds_alternative<BoolValue>(data); }
    [[nodiscard]] bool is_int() const { return std::holds_alternative<IntValue>(data); }
    [[nodiscard]] bool is_float() const { return std::holds_alternative<FloatValue>(data); }
    [[nodiscard]] bool is_string() const { return std::holds_alternative<StringValue>(data); }
    [[nodiscard]] bool is_array() const { return std::holds_alternative<ArrayValue>(data); }
    [[nodiscard]] bool is_object() const { return std::holds_alternative<ObjectValue>(data); }

    [[nodiscard]] bool as_bool() const { return std::get<BoolValue>(data).value; }
    [[nodiscard]] std::int64_t as_int() const { return std::get<IntValue>(data).value; }
    [[nodiscard]] double as_float() const { return std::get<FloatValue>(data).value; }
    [[nodiscard]] const std::string& as_string() const {
        return std::get<StringValue>(data).value;
    }
    [[nodiscard]] const Array& as_array() const { return std::get<ArrayValue>(data).elements; }
    [[nodiscard]] const Object& as_object() const {
        return std::get<ObjectValue>(data).properties;
    }

    [[nodiscard]] Array& as_array_mut() { return std::get<ArrayValue>(data).elements; }
    [[nodiscard]] Object& as_object_mut() { return std::get<ObjectValue>(data).properties; }
};

[[nodiscard]] bool structural_equal(const Value& lhs, const Value& rhs);
[[nodiscard]] bool structural_equal(const Object& lhs, const Object& rhs);
[[nodiscard]] bool structural_equal(const Document& lhs, const Document& rhs);

[[nodiscard]] std::string format_ast(const Document& document);
[[nodiscard]] std::string format_value(const Value& value, int indent = 0);

}  // namespace tinycfg
