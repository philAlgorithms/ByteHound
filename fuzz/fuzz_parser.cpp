#include "tinycfg/tinycfg.hpp"

#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    if (size > tinycfg::Limits::kMaxInputBytes) {
        return 0;
    }
    const std::string_view input(reinterpret_cast<const char*>(data), size);
    tinycfg::Document document;
    (void)tinycfg::parse(input, document);
    return 0;
}
