#include "tinycfg/tinycfg.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>

namespace {

enum class Op : std::uint8_t {
    kAddScalar = 0,
    kAddArray = 1,
    kAppendArrayElement = 2,
    kAddNestedObject = 3,
    kRemoveProperty = 4,
    kReplaceValue = 5,
    kSerialize = 6,
    kParseSerialized = 7,
    kCompareRoundtrip = 8,
};

constexpr std::size_t kMaxOps = 64;
constexpr std::size_t kMaxKeyLen = 16;

std::string make_key(const std::uint8_t* data, std::size_t size) {
    if (size == 0) {
        return "k";
    }
    return std::string(reinterpret_cast<const char*>(data), std::min(size, kMaxKeyLen));
}

tinycfg::Value scalar_from_byte(std::uint8_t byte) {
    switch (byte % 5) {
        case 0:
            return tinycfg::Value{tinycfg::BoolValue{true}};
        case 1:
            return tinycfg::Value{tinycfg::IntValue{static_cast<std::int64_t>(byte)}};
        case 2:
            return tinycfg::Value{tinycfg::FloatValue{static_cast<double>(byte) / 10.0}};
        case 3:
            return tinycfg::Value{tinycfg::NullValue{}};
        default: {
            const char ch = static_cast<char>('a' + (byte % 26));
            return tinycfg::Value{tinycfg::StringValue{std::string(1, ch)}};
        }
    }
}

tinycfg::Object::iterator pick_entry(tinycfg::Object& object, std::uint8_t selector) {
    if (object.empty()) {
        return object.end();
    }
    const std::size_t pick = selector % object.size();
    auto it = object.begin();
    for (std::size_t step = 0; step < pick; ++step) {
        ++it;
    }
    return it;
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    if (size > tinycfg::Limits::kMaxInputBytes || size < 2) {
        return 0;
    }

    tinycfg::Document document;
    bool has_root = false;
    std::string last_serialized;

    const std::size_t op_count = std::min<std::size_t>(data[0] % (kMaxOps + 1), kMaxOps);
    std::size_t offset = 1;

    for (std::size_t i = 0; i < op_count && offset < size; ++i) {
        const auto op = static_cast<Op>(data[offset++] % 9);
        if (offset >= size) {
            break;
        }

        if (!has_root) {
            document.emplace("root", tinycfg::Object{});
            has_root = true;
        }
        tinycfg::Object& root = document.at("root");

        switch (op) {
            case Op::kAddScalar: {
                const std::size_t limit_byte = data[offset++];
                const std::size_t key_size =
                    std::min<std::size_t>(limit_byte % (kMaxKeyLen + 1), size - offset);
                const auto key = make_key(data + offset, key_size);
                offset = std::min(offset + key_size, size);
                if (offset >= size) {
                    break;
                }
                root[key] = scalar_from_byte(data[offset++]);
                break;
            }
            case Op::kAddArray: {
                const std::size_t limit_byte = data[offset++];
                const std::size_t key_size =
                    std::min<std::size_t>(limit_byte % (kMaxKeyLen + 1), size - offset);
                const auto key = make_key(data + offset, key_size);
                offset = std::min(offset + key_size, size);
                root[key] = tinycfg::Value{tinycfg::ArrayValue{}};
                break;
            }
            case Op::kAppendArrayElement: {
                const auto it = pick_entry(root, data[offset]);
                if (it != root.end() && it->second.is_array() &&
                    it->second.as_array().size() < tinycfg::Limits::kMaxContainerEntries) {
                    it->second.as_array_mut().push_back(scalar_from_byte(data[offset]));
                }
                ++offset;
                break;
            }
            case Op::kAddNestedObject: {
                const std::size_t limit_byte = data[offset++];
                const std::size_t key_size =
                    std::min<std::size_t>(limit_byte % (kMaxKeyLen + 1), size - offset);
                const auto key = make_key(data + offset, key_size);
                offset = std::min(offset + key_size, size);
                tinycfg::Object nested;
                nested["child"] = scalar_from_byte(data[offset % size]);
                root[key] = tinycfg::Value{tinycfg::ObjectValue{std::move(nested)}};
                break;
            }
            case Op::kRemoveProperty: {
                const auto it = pick_entry(root, data[offset]);
                if (it != root.end()) {
                    root.erase(it);
                }
                ++offset;
                break;
            }
            case Op::kReplaceValue: {
                const auto it = pick_entry(root, data[offset]);
                if (it != root.end()) {
                    it->second = scalar_from_byte(data[(offset + 1) % size]);
                }
                offset += 2;
                break;
            }
            case Op::kSerialize: {
                last_serialized.clear();
                tinycfg::Document single;
                single.emplace("root", root);
                (void)tinycfg::serialize(single, last_serialized);
                break;
            }
            case Op::kParseSerialized: {
                if (last_serialized.empty()) {
                    break;
                }
                tinycfg::Document parsed;
                (void)tinycfg::parse(last_serialized, parsed);
                break;
            }
            case Op::kCompareRoundtrip: {
                if (last_serialized.empty()) {
                    break;
                }
                tinycfg::Document parsed;
                if (tinycfg::parse(last_serialized, parsed).failed()) {
                    break;
                }
                std::string reserialized;
                if (tinycfg::serialize(parsed, reserialized).failed()) {
                    break;
                }
                tinycfg::Document again;
                if (tinycfg::parse(reserialized, again).failed()) {
                    __builtin_trap();
                }
                if (!tinycfg::structural_equal(parsed, again)) {
                    __builtin_trap();
                }
                break;
            }
        }
    }

    return 0;
}
