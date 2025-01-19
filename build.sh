# dir names
SRC_DIR="src"
OUT_DIR="out"
API_DIR="$SRC_DIR/api"

# input and output files
OUT="$OUT_DIR/main.exe"
SRC="$SRC_DIR/*.c $API_DIR/*.c"

clear # clear output
rm -f "$OUT"
find . -name "*.c" -o -name "*.h" | xargs clang-format -i
gcc $SRC -o "$OUT"
chmod +x "$OUT"
./"$OUT"
