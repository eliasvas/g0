PKGS="sdl3 glew"
CFLAGS="-Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wswitch-enum \
  -pedantic -fno-exceptions -g -fsanitize=address"
CC="clang"

rm -rf build
mkdir -p build

export ASAN_OPTIONS=detect_stack_use_after_return=1
export LSAN_OPTIONS=suppressions=lsan_ignore.txt

$CC $CFLAGS -O0 -std=gnu23 `pkg-config --cflags $PKGS` -Iext -Isrc \
src/base/*.c src/core/*.c src/game/*.c src/gui/*.c -o build/g0 `pkg-config --libs $PKGS` -lm

if [ $? -eq 0 ]; then
    echo "✅ Build succeeded."
else
    echo "❌ Build failed."
fi
