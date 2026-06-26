#pragma once

#include "tinycfg/ast.hpp"
#include "tinycfg/error.hpp"

#include <string>
#include <string_view>

namespace tinycfg {

class Serializer {
public:
    [[nodiscard]] static ParseResult serialize(const Document& document, std::string& output);
    [[nodiscard]] static ParseResult serialize_value(const Value& value, std::string& output,
                                                     int indent = 0);
};

}  // namespace tinycfg
