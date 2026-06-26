#include "legacy_parser.hpp"

#include <cstring>

namespace legacy {

namespace {

constexpr std::uint8_t kMagic0Val = 0x42;
constexpr std::uint8_t kMagic1Val = 0x48;

}  // namespace

ParseOutcome parse_records_vulnerable(const std::uint8_t* data, std::size_t size) {
    ParseOutcome outcome;
    if (size < 4) {
        outcome.error = "input too short";
        return outcome;
    }
    if (data[0] != kMagic0Val || data[1] != kMagic1Val) {
        outcome.error = "bad magic";
        return outcome;
    }

    const std::uint16_t record_count = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(data[2]) | (static_cast<std::uint16_t>(data[3]) << 8));
    std::size_t offset = 4;

    for (std::uint16_t i = 0; i < record_count; ++i) {
        if (offset + 3 > size) {
            outcome.error = "truncated record header";
            return outcome;
        }
        Record record;
        record.tag = data[offset++];
        const std::uint16_t length = static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(data[offset]) |
            (static_cast<std::uint16_t>(data[offset + 1]) << 8));
        offset += 2;

        // BUG: trusts length without verifying payload fits in remaining input.
        record.payload.resize(length);
        std::memcpy(record.payload.data(), data + offset, length);
        offset += length;
        outcome.records.push_back(std::move(record));
    }

    outcome.ok = true;
    return outcome;
}

ParseOutcome parse_records_fixed(const std::uint8_t* data, std::size_t size) {
    ParseOutcome outcome;
    if (size < 4) {
        outcome.error = "input too short";
        return outcome;
    }
    if (data[0] != kMagic0Val || data[1] != kMagic1Val) {
        outcome.error = "bad magic";
        return outcome;
    }

    const std::uint16_t record_count = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(data[2]) | (static_cast<std::uint16_t>(data[3]) << 8));
    std::size_t offset = 4;

    for (std::uint16_t i = 0; i < record_count; ++i) {
        if (offset + 3 > size) {
            outcome.error = "truncated record header";
            return outcome;
        }
        Record record;
        record.tag = data[offset++];
        const std::uint16_t length = static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(data[offset]) |
            (static_cast<std::uint16_t>(data[offset + 1]) << 8));
        offset += 2;

        if (offset + length > size) {
            outcome.error = "record payload exceeds input bounds";
            return outcome;
        }

        record.payload.assign(data + offset, data + offset + length);
        offset += length;
        outcome.records.push_back(std::move(record));
    }

    outcome.ok = true;
    return outcome;
}

}  // namespace legacy
