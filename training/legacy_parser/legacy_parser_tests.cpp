#include "legacy_parser.hpp"

#include "test_harness.hpp"

#include <fstream>
#include <iterator>
#include <vector>

namespace {

std::vector<std::uint8_t> load_minimized_reproducer() {
    std::ifstream input(std::string(TINYCFG_SOURCE_DIR) +
                            "/training/legacy_parser/data/minimized_heap_buffer_overflow.bin",
                        std::ios::binary);
    if (!input) {
        throw std::runtime_error("missing minimized reproducer fixture");
    }
    return {std::istreambuf_iterator<char>(input), {}};
}

}  // namespace

TEST(legacy_fixed_rejects_oversized_length) {
    const std::uint8_t input[] = {0x42, 0x48, 0x01, 0x00, 0x01, 0x00, 0x01, 0xAA, 0xBB};
    const auto outcome = legacy::parse_records_fixed(input, sizeof(input));
    EXPECT_FALSE(outcome.ok);
    EXPECT_NE(outcome.error.find("bounds"), std::string::npos);
}

TEST(legacy_fixed_accepts_minimized_reproducer) {
    const auto bytes = load_minimized_reproducer();
    EXPECT_TRUE(bytes.size() >= 7);
    const auto outcome = legacy::parse_records_fixed(bytes.data(), bytes.size());
    EXPECT_FALSE(outcome.ok);
}
