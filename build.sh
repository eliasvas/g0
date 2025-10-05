#TODO: Look into building with ASAN
PKGS="sdl3 glew"
CFLAGS="-Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wswitch-enum -pedantic -fno-exceptions -ggdb"
CC="clang"

rm -rf build
mkdir -p build

$CC $CFLAGS -O0 -std=gnu23 `pkg-config --cflags $PKGS` -Iext src/*.c -o build/g0 `pkg-config --libs $PKGS` -lm

