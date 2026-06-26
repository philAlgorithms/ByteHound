#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

DURATION="${1:-15}"
BUILD_DIR="build-fuzz"
ARTIFACTS="$ROOT/artifacts/fuzz-smoke"

if [[ ! -x "$BUILD_DIR/fuzz_lexer" ]]; then
    "$ROOT/scripts/build.sh" fuzz
fi

mkdir -p "$ARTIFACTS"

declare -a TARGETS=(
    "fuzz_lexer:fuzz/seed_corpus/lexer"
    "fuzz_parser:fuzz/seed_corpus/parser"
    "fuzz_roundtrip:fuzz/seed_corpus/roundtrip"
    "fuzz_mutation_sequence:fuzz/seed_corpus/mutation_sequence"
)

status=0
for entry in "${TARGETS[@]}"; do
    target="${entry%%:*}"
    corpus="${entry##*:}"
    prefix="$ARTIFACTS/${target}"
    corpus_work="$ARTIFACTS/corpus-${target}"
    rm -rf "$corpus_work"
    mkdir -p "$corpus_work"
    if compgen -G "$ROOT/$corpus/"* >/dev/null; then
        cp -r "$ROOT/$corpus/"* "$corpus_work/"
    fi
    echo "==> smoke fuzzing $target for ${DURATION}s"
    if ! "$BUILD_DIR/$target" \
        -max_total_time="$DURATION" \
        -timeout=5 \
        -rss_limit_mb=512 \
        -max_len=65536 \
        -dict="$ROOT/fuzz/tinycfg.dict" \
        -artifact_prefix="$prefix-" \
        "$corpus_work"; then
        status=1
    fi
done

exit "$status"
