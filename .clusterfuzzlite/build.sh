#!/bin/bash -eu

PROJECT=byte-hound
BUILD_DIR="$WORK/build"

export CXX="${CXX:-clang++}"
export CC="${CC:-clang}"

cmake -S "$SRC/$PROJECT" -B "$BUILD_DIR" -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_C_COMPILER="$CC" \
  -DCMAKE_CXX_COMPILER="$CXX" \
  -DTINYCFG_BUILD_CLI=OFF \
  -DTINYCFG_BUILD_TESTS=OFF \
  -DTINYCFG_BUILD_FUZZERS=ON \
  -DTINYCFG_ENABLE_SANITIZERS=ON \
  -DCMAKE_C_FLAGS="$CFLAGS" \
  -DCMAKE_CXX_FLAGS="$CXXFLAGS"

cmake --build "$BUILD_DIR" --parallel "$(nproc)"

declare -a FUZZERS=(
  fuzz_lexer
  fuzz_parser
  fuzz_roundtrip
  fuzz_mutation_sequence
)

for fuzzer in "${FUZZERS[@]}"; do
  if [[ ! -f "$BUILD_DIR/$fuzzer" ]]; then
    echo "missing fuzzer binary: $fuzzer" >&2
    exit 1
  fi
  cp "$BUILD_DIR/$fuzzer" "$OUT/"
done

cp "$SRC/$PROJECT/fuzz/tinycfg.dict" "$OUT/fuzz_parser.dict"

for corpus_dir in lexer parser roundtrip mutation_sequence; do
  target="fuzz_${corpus_dir}"
  if [[ "$corpus_dir" == "mutation_sequence" ]]; then
    target="fuzz_mutation_sequence"
  fi
  zip -j "$OUT/${target}_seed_corpus.zip" "$SRC/$PROJECT/fuzz/seed_corpus/${corpus_dir}/"*
done
