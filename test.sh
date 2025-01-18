# dir names
TEST_DIR="tests"
SRC_DIR="src"
OUT_DIR="out"
API_DIR="$SRC_DIR/api"

# input and output files
OUT="$OUT_DIR/tests.exe"
SRC="$TEST_DIR/*.c $API_DIR/*.c"

clear # clear output
rm -f "$OUT"
find . -name "*.c" -o -name "*.h" | xargs clang-format -i
gcc $SRC -o "$OUT" -Wno-implicit-function-declaration
chmod +x "$OUT"
./"$OUT"