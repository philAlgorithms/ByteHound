# Legacy Training Fixture Case Study (Intentional Defect)

> **Classification:** Educational / intentionally planted — NOT an organic TinyCFG parser vulnerability.

## Finding title

Heap buffer overflow from unchecked record length in legacy binary parser

## Affected component

`training/legacy_parser/legacy_parser.cpp` — `parse_records_vulnerable`

## Discovery method

AddressSanitizer report while executing `fuzz_legacy_parser` with a crafted 9-byte binary input (also found manually during fixture validation).

## Sanitizer

AddressSanitizer (`-fsanitize=fuzzer,address,undefined`)

## Reproducer

```bash
scripts/reproduce.sh fuzz_legacy_parser training/legacy_parser/data/minimized_heap_buffer_overflow.bin
```

## Minimized input

`training/legacy_parser/data/minimized_heap_buffer_overflow.bin` (9 bytes)

Hex: `42 48 01 00 01 00 01 aa bb`

Layout:

| Offset | Meaning |
|--------|---------|
| 0-1 | Magic `BH` |
| 2-3 | Record count = 1 |
| 4 | Tag = 1 |
| 5-6 | Declared payload length = 256 |
| 7-8 | Only 2 payload bytes available |

## Observed failure

```
ERROR: AddressSanitizer: heap-buffer-overflow
READ of size 256 at ...
legacy::parse_records_vulnerable(...) legacy_parser.cpp:43
```

Root cause line:

```cpp
record.payload.resize(length);
std::memcpy(record.payload.data(), data + offset, length);  // reads past input end
```

## Root cause

The vulnerable parser trusts the 16-bit length field without verifying `offset + length <= size`.

## Exploitability discussion

Out-of-bounds read into heap-allocated storage. Impact depends on caller; this fixture is not linked into production builds. No evidence of control-flow hijack was evaluated.

## Patch

`parse_records_fixed` validates bounds before copying:

```cpp
if (offset + length > size) {
    outcome.error = "record payload exceeds input bounds";
    return outcome;
}
```

## Regression test

`legacy_fixed_rejects_oversized_length` and `legacy_fixed_accepts_minimized_reproducer` in `training/legacy_parser/legacy_parser_tests.cpp`

## Validation commands

```bash
./scripts/build.sh legacy
./build-legacy/legacy_parser_tests
scripts/reproduce.sh fuzz_legacy_parser training/legacy_parser/data/minimized_heap_buffer_overflow.bin
# Fixed parser returns error instead of crashing when invoked via unit tests.
```

## Limitations

The vulnerable function remains in-tree for training review; default builds exclude the fixture.
