# Fuzzing Methodology

## Coverage-guided fuzzing

libFuzzer mutates inputs to maximize code coverage. ByteHound uses four independent harnesses so lexer, parser, serializer, and AST lifecycle bugs surface independently.

## Seed corpora

Seeds under `fuzz/seed_corpus/` bootstrap exploration with valid configs, nested structures, comments, numeric boundaries, and incomplete syntax. Seeds are small and hand-written.

## Dictionaries

`fuzz/tinycfg.dict` supplies tokens and literals that help the mutator compose syntactically plausible inputs quickly.

## Sanitizer instrumentation

Non-fuzzer builds may enable ASan+UBSan via `TINYCFG_ENABLE_SANITIZERS`. Fuzzer binaries always link libFuzzer with ASan and UBSan.

Sanitizers turn memory and undefined-behavior bugs into deterministic crashes with stack traces.

## Reproducibility

Every crash must replay with a single command:

```bash
scripts/reproduce.sh fuzz_parser artifacts/crash-...
```

## Why harness filtering is discouraged

Ignoring inputs that trigger sanitizer failures hides real bugs. Prefer root-cause fixes in library code.

## Unit tests vs fuzzing

Unit tests assert known examples and regression fixtures. Fuzzing searches an enormous input space for unexpected interactions. Both are required; neither replaces the other.

## Main parser vs training fixture

| | Main TinyCFG parser | Legacy training fixture |
|--|---------------------|-------------------------|
| Purpose | Production-quality reference | Deliberate vulnerability lab |
| Default build | Enabled | Disabled |
| Findings | Organic only if observed | Intentionally planted |

Never present training fixture results as organic TinyCFG vulnerabilities.

## Limitations

Fuzzing does not prove security. It increases confidence by finding crashes and UB during bounded exploration.
