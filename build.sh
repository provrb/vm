#!/bin/bash

SRC_DIR="./src"
API_DIR="./api"
OUT_DIR="./out"

OUT="$OUT_DIR/main"
SRC=$(find $SRC_DIR $API_DIR -name "*.c")

clear
rm -f "$OUT"
find . -name "*.c" -o -name "*.h" | xargs clang-format -i
gcc $SRC -o "$OUT" -I $SRC_DIR -I $API_DIR
chmod +x "$OUT"
./"$OUT"
