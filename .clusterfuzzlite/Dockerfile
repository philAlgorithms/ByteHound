FROM gcr.io/oss-fuzz-base/base-builder:latest

RUN apt-get update && apt-get install -y cmake ninja-build && rm -rf /var/lib/apt/lists/*

COPY . $SRC/byte-hound
WORKDIR $SRC/byte-hound

COPY .clusterfuzzlite/build.sh $SRC/build.sh
