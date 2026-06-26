# Verified Results

All entries below were produced on the development machine used to build this repository.

## Environment

| Item | Value |
|------|-------|
| Date | 2026-06-26 |
| Platform | WSL2 (Linux) on Windows 10 |
| Git | 2.43.0 |
| Clang | Ubuntu clang 18.1.3 (1ubuntu1) |
| CMake | 3.28.3 |
| Ninja | 1.11.1 |
| Python | 3.12.3 |
| Docker | Not available in WSL (`docker: command not found`) |

## Sanitizer configuration

Non-fuzzer builds:

```
-fsanitize=address,undefined -fno-omit-frame-pointer -g
ASAN_OPTIONS=halt_on_error=1:abort_on_error=1:symbolize=1:detect_leaks=1
UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1
```

Fuzzer builds:

```
-fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer -g
```

## Build and test commands

```bash
./scripts/build.sh normal
ctest --test-dir build --output-on-failure

./scripts/test.sh

./scripts/build.sh fuzz
./scripts/fuzz-smoke.sh 10
```

### Unit test outcome

24 tests passed (0 failed) in `tinycfg_unit_tests` after final fixes.

## Fuzz smoke (10 seconds per target)

| Target | Outcome | Notes |
|--------|---------|-------|
| `fuzz_lexer` | Passed | ~39k exec/s window, no crash |
| `fuzz_parser` | Passed | ~42k exec/s window, no crash |
| `fuzz_roundtrip` | Passed | ~20k exec/s window, no crash |
| `fuzz_mutation_sequence` | Passed after harness fix | Initial run hit ASan heap-use-after-free due to invalid map iterator dereference in harness (not TinyCFG library) |

## Organic TinyCFG parser findings

No AddressSanitizer or UndefinedBehaviorSanitizer crashes were attributed to `include/tinycfg/` or `src/` after fixes during this session.

### Fixes applied during development

1. **Parser nesting depth propagation** — inner object assignments always used depth 0; fixed by passing depth through `parse_assignment`. Regression: `parser_nested_depth_propagates_to_inner_assignments`.
2. **Mutation fuzz harness iterator safety** — `fuzz_mutation_sequence` could dereference `end()` iterators; fixed with bounded `pick_entry` helper.
3. **Mutation fuzz harness buffer read** — `make_key` read one byte past input when key length was zero; fixed empty-length handling.

## Educational legacy fixture (intentional)

| Step | Result |
|------|--------|
| Build with `TINYCFG_ENABLE_LEGACY_TRAINING_FIXTURE=ON` | Success (`build-legacy/`) |
| Fuzz `fuzz_legacy_parser` | ASan heap-buffer-overflow reproduced with 9-byte input |
| Minimized reproducer | `training/legacy_parser/data/minimized_heap_buffer_overflow.bin` |
| Fixed parser regression tests | `./build-legacy/legacy_parser_tests` — 2 passed |

See `training/legacy_parser/CASE_STUDY.md`.

## ClusterFuzzLite

Files created under `.clusterfuzzlite/` and `.github/workflows/` following official `google/clusterfuzzlite/actions/*@v1` examples.

Docker integration was **not** validated locally because Docker is unavailable in this WSL environment.

## Remaining limitations

- Fuzz durations were short (10–20 seconds per target); coverage is not exhaustive.
- No continuous storage repo configured for corpus merging in CI.
- Docker-based ClusterFuzzLite build unverified locally.
