#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

"$ROOT/scripts/build.sh" sanitize

export ASAN_OPTIONS="halt_on_error=1:abort_on_error=1:symbolize=1:detect_leaks=1"
export UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1"

ctest --test-dir build-sanitize --output-on-failure
