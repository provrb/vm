# dir names
SRC_DIR="src"
OUT_DIR="out"
API_DIR="$SRC_DIR/api"

# input and output files
OUT="$OUT_DIR/main.exe"
SRC="$SRC_DIR/*.c $API_DIR/*.c"

rm -f "$OUT"
gcc $SRC -o "$OUT"
./"$OUT"
