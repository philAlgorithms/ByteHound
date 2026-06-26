#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// EDUCATIONAL FIXTURE ONLY — intentionally vulnerable legacy binary record parser.
// Do not use in production. Enabled only with TINYCFG_ENABLE_LEGACY_TRAINING_FIXTURE.

namespace legacy {

struct Record {
    std::uint8_t tag{};
    std::vector<std::uint8_t> payload;
};

struct ParseOutcome {
    bool ok{false};
    std::string error;
    std::vector<Record> records;
};

// Fixed implementation used by regression tests.
ParseOutcome parse_records_fixed(const std::uint8_t* data, std::size_t size);

// Vulnerable implementation retained for case-study review and fuzzing demonstrations.
ParseOutcome parse_records_vulnerable(const std::uint8_t* data, std::size_t size);

}  // namespace legacy
