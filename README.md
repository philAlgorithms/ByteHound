# ByteHound

Portfolio-grade C++20 project demonstrating parser construction, libFuzzer harness design, sanitizer-driven triage, and ClusterFuzzLite CI integration.

## Why parser fuzzing matters

Configuration parsers accept untrusted input at process boundaries. Small lexing or bounds-check omissions become memory safety bugs. ByteHound shows how to combine strict parsing limits, deterministic serialization, fuzzing, and sanitizer instrumentation on a realistic but compact language.

## Architecture overview

TinyCFG uses a separate lexer and recursive-descent parser, a value-semantics AST, and a canonical serializer. See [docs/architecture.md](docs/architecture.md).

## TinyCFG syntax (summary)

```cfg
server {
    host = "localhost";
    port = 8080;
    tls = true;
    retries = 3;
    tags = ["api", "production"];
    limits = {
        requests = 5000;
        timeout_ms = 2500;
    };
}
```

Features: identifiers, quoted strings with escapes, int64/float/bool/null, arrays, nested objects, `#` and `//` comments, semicolon-terminated assignments. Duplicate object keys use **last-wins** semantics.

## Quick start (Linux / WSL2)

```bash
chmod +x scripts/*.sh
./scripts/build.sh normal
ctest --test-dir build --output-on-failure
./build/tinycfg-cli validate tests/data/valid_server.cfg
```

Requires **Clang**, **CMake**, and optionally **Ninja**.

## Builds

| Mode | Command |
|------|---------|
| Normal | `./scripts/build.sh normal` |
| Sanitizers | `./scripts/build.sh sanitize` |
| Fuzzers | `./scripts/build.sh fuzz` |
| Legacy fixture | `./scripts/build.sh legacy` |

Manual CMake example:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DTINYCFG_BUILD_TESTS=ON
cmake --build build
```

## Running fuzzers

```bash
./scripts/build.sh fuzz
./build-fuzz/fuzz_parser fuzz/seed_corpus/parser fuzz/tinycfg.dict
./scripts/fuzz-smoke.sh 15
```

## Reproducing and minimizing crashes

```bash
scripts/reproduce.sh fuzz_parser path/to/crash-input
scripts/minimize.sh fuzz_parser path/to/crash-input artifacts/minimized
```

## Tests

```bash
./scripts/test.sh
```

## ClusterFuzzLite / Docker

- Configuration: `.clusterfuzzlite/`
- Workflows: `.github/workflows/cflite_pr.yml`, `cflite_cron.yml`

Build the Docker image locally (when Docker is available):

```bash
docker build -t bytehound-cflite -f .clusterfuzzlite/Dockerfile .
```

## Repository layout

See project tree in `docs/architecture.md`. Key directories: `include/tinycfg/`, `src/`, `fuzz/`, `tests/`, `training/legacy_parser/`.

## Verified results

See [docs/results.md](docs/results.md) for command lines, toolchain versions, and findings recorded from actual runs in this repository.

## Limitations

- Fuzzing does **not** prove the parser is secure.
- Default builds exclude the intentionally vulnerable legacy training code.
- CI fuzz jobs use bounded durations suitable for PR feedback, not exhaustive campaigns.

## Future work

- Expand seed corpora from CI corpus pruning
- Add structured fuzzer for numeric edge cases
- Publish coverage reports via a ClusterFuzzLite storage repo
