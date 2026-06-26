#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

pick_generator() {
    if command -v ninja >/dev/null 2>&1; then
        echo "Ninja"
    else
        echo "Unix Makefiles"
    fi
}

build_dir_for_mode() {
    case "$1" in
        normal) echo "build" ;;
        sanitize) echo "build-sanitize" ;;
        fuzz) echo "build-fuzz" ;;
        legacy) echo "build-legacy" ;;
        *) echo "unknown build mode: $1" >&2; exit 1 ;;
    esac
}

configure() {
    local mode="$1"
    local build_dir
    build_dir="$(build_dir_for_mode "$mode")"
    local generator
    generator="$(pick_generator)"

    local -a extra=()
    case "$mode" in
        normal)
            extra+=(-DTINYCFG_BUILD_TESTS=ON -DTINYCFG_BUILD_CLI=ON -DTINYCFG_BUILD_FUZZERS=OFF)
            ;;
        sanitize)
            extra+=(-DTINYCFG_BUILD_TESTS=ON -DTINYCFG_BUILD_CLI=ON -DTINYCFG_BUILD_FUZZERS=OFF -DTINYCFG_ENABLE_SANITIZERS=ON)
            ;;
        fuzz)
            extra+=(-DCMAKE_BUILD_TYPE=RelWithDebInfo -DTINYCFG_BUILD_TESTS=OFF -DTINYCFG_BUILD_CLI=OFF -DTINYCFG_BUILD_FUZZERS=ON -DTINYCFG_ENABLE_SANITIZERS=ON)
            ;;
        legacy)
            extra+=(-DCMAKE_BUILD_TYPE=RelWithDebInfo -DTINYCFG_BUILD_TESTS=OFF -DTINYCFG_BUILD_CLI=OFF -DTINYCFG_BUILD_FUZZERS=OFF -DTINYCFG_ENABLE_LEGACY_TRAINING_FIXTURE=ON -DTINYCFG_ENABLE_SANITIZERS=ON)
            ;;
    esac

    cmake -S . -B "$build_dir" -G "$generator" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=clang++ "${extra[@]}"
    cmake --build "$build_dir"
}

MODE="${1:-normal}"
configure "$MODE"
