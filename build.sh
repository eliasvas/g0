#TODO: Look into emscripten building
#TODO: Look into building with ASAN
PKGS="sdl3 glew"
CFLAGS="-Wall -Wextra -Wno-unused-parameter -Wswitch-enum -pedantic -fno-exceptions -ggdb"
CC="clang"
rm -rf build
mkdir -p build
$CC $CFLAGS -O3 -std=gnu23  `pkg-config --cflags $PKGS` -o build/g0 src/platform_all.c src/profiler.c src/game.c `pkg-config --libs $PKGS`
