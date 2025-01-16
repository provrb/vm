# dir names
TEST_DIR="tests"
SRC_DIR="src"
OUT_DIR="out"
API_DIR="$SRC_DIR/api"

# input and output files
OUT="$OUT_DIR/tests.exe"
SRC="$TEST_DIR/tests.c $API_DIR/*.c"

clear # clear output
rm -f "$OUT"
gcc $SRC -o "$OUT" -Wall -Wextra -Werror
chmod +x "$OUT"
./"$OUT"