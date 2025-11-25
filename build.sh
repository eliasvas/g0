PKGS="sdl3 glew"
CFLAGS="-Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wswitch-enum \
  -pedantic -fno-exceptions -fstack-protector -g -fsanitize=address"
CC="clang"

rm -rf build
mkdir -p build

export ASAN_OPTIONS=detect_stack_use_after_return=1
export LSAN_OPTIONS=suppressions=lsan_ignore.txt

# Build the game dynamic library
$CC $CFLAGS -O0 -std=gnu23 -Iext -Isrc -fPIC -shared \
src/core/input.c src/game/*.c src/gui/*.c -o build/game.so

if [ $? -eq 0 ]; then
    echo "✅ Game Build succeeded."
else
    echo "❌ Game Build failed."
fi

# Build the core (engine) executable
$CC $CFLAGS -O0 -std=gnu23 `pkg-config --cflags $PKGS` -Lbuild \
-Iext -Isrc src/base/*.c src/core/*.c -o build/game0 \
`pkg-config --libs $PKGS` -lm 

if [ $? -eq 0 ]; then
    echo "✅ Core Build succeeded."
else
    echo "❌ Core Build failed."
fi
