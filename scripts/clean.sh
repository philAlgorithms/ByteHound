#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

rm -rf build build-sanitize build-fuzz build-legacy artifacts corpus-out coverage
find . -name 'CMakeCache.txt' -delete 2>/dev/null || true

echo "Clean complete. Committed seed corpora under fuzz/seed_corpus were preserved."
