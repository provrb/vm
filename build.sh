#!/bin/bash

SRC_DIR="./src"
OUT_DIR="./out"
API_DIR="$SRC_DIR/api"

OUT="$OUT_DIR/main"
SRC="$SRC_DIR/*.c $API_DIR/*.c"

clear
rm -f "$OUT"
gcc $SRC -o "$OUT" -Wextra -Wall
chmod +x "$OUT"
./"$OUT"
