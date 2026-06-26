#pragma once

#include <cstddef>
#include <cstdint>

namespace tinycfg {

struct Limits {
    static constexpr std::size_t kMaxInputBytes = 1 << 20;       // 1 MiB
    static constexpr int kMaxNestingDepth = 64;
    static constexpr std::size_t kMaxTokenCount = 100'000;
    static constexpr std::size_t kMaxStringBytes = 64 * 1024;    // 64 KiB
    static constexpr std::size_t kMaxContainerEntries = 10'000;
    static constexpr std::size_t kMaxIdentifierBytes = 256;
};

}  // namespace tinycfg
