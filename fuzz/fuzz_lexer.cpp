#include "tinycfg/tinycfg.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    if (size > tinycfg::Limits::kMaxInputBytes) {
        return 0;
    }
    const std::string_view input(reinterpret_cast<const char*>(data), size);
    std::vector<tinycfg::Token> tokens;
    (void)tinycfg::tokenize(input, tokens);
    return 0;
}
