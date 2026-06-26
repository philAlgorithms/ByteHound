# Crash Triage

## 1. Preserve the artifact

Fuzzers write artifacts under `artifacts/` when using `scripts/fuzz-smoke.sh` (`-artifact_prefix`).

## 2. Reproduce

```bash
scripts/reproduce.sh <fuzzer-name> <artifact-path>
```

Example:

```bash
scripts/reproduce.sh fuzz_parser artifacts/fuzz-smoke/fuzz_parser-deadly
```

## 3. Record toolchain

```bash
clang++ --version
echo "$ASAN_OPTIONS"
echo "$UBSAN_OPTIONS"
```

## 4. Minimize

```bash
scripts/minimize.sh <fuzzer-name> <artifact-path> artifacts/minimized-input
```

## 5. Symbolize

Ensure `ASAN_OPTIONS` includes `symbolize=1`. Use `llvm-symbolizer` from the same Clang version if needed.

## 6. Identify first project frame

Look for the first frame under `src/`, `include/tinycfg/`, or `training/legacy_parser/`.

## 7. Fix root cause

Patch library logic. Do not silence the crash in the harness unless the input violates an explicit contract documented as out of scope.

## 8. Regression fixture

Add minimized input under `tests/data/` or `training/legacy_parser/data/` and a named test in `tests/regression_tests.cpp` or `training/legacy_parser/legacy_parser_tests.cpp`.

## 9. Validate

```bash
scripts/test.sh
scripts/fuzz-smoke.sh 10
```

## 10. Document

Update `docs/results.md` and, for training findings, `training/legacy_parser/CASE_STUDY.md`.
