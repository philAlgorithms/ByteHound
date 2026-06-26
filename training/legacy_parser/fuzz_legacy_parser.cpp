#include "legacy_parser.hpp"

#include <cstddef>
#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    (void)legacy::parse_records_vulnerable(data, size);
    return 0;
}
