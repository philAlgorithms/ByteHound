#include "tinycfg/tinycfg.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    if (size > tinycfg::Limits::kMaxInputBytes) {
        return 0;
    }
    const std::string_view input(reinterpret_cast<const char*>(data), size);
    tinycfg::Document first;
    const auto parse_result = tinycfg::parse(input, first);
    if (parse_result.failed()) {
        return 0;
    }

    std::string serialized;
    const auto serialize_result = tinycfg::serialize(first, serialized);
    if (serialize_result.failed()) {
        return 0;
    }

    tinycfg::Document second;
    const auto reparsed = tinycfg::parse(serialized, second);
    if (reparsed.failed()) {
        __builtin_trap();
    }
    if (!tinycfg::structural_equal(first, second)) {
        __builtin_trap();
    }

    std::string reserialized;
    if (tinycfg::serialize(second, reserialized).failed()) {
        __builtin_trap();
    }
    if (serialized != reserialized) {
        __builtin_trap();
    }
    return 0;
}
