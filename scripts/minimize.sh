#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

if [[ $# -ne 3 ]]; then
    echo "Usage: $0 <fuzzer-name> <artifact-path> <output-path>" >&2
    exit 1
fi

FUZZER="$1"
ARTIFACT="$2"
OUTPUT="$3"
BUILD_DIR="build-fuzz"
if [[ "$FUZZER" == "fuzz_legacy_parser" ]]; then
    BUILD_DIR="build-legacy"
fi

if [[ ! -f "$ARTIFACT" ]]; then
    echo "artifact not found: $ARTIFACT" >&2
    exit 1
fi

if [[ ! -x "$BUILD_DIR/$FUZZER" ]]; then
    if [[ "$FUZZER" == "fuzz_legacy_parser" ]]; then
        "$ROOT/scripts/build.sh" legacy
    else
        "$ROOT/scripts/build.sh" fuzz
    fi
fi

export ASAN_OPTIONS="halt_on_error=1:abort_on_error=1:symbolize=1"
export UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1"

"$BUILD_DIR/$FUZZER" -minimize_crash=1 -exact_artifact_path="$OUTPUT" "$ARTIFACT"

"$ROOT/scripts/reproduce.sh" "$FUZZER" "$OUTPUT"
