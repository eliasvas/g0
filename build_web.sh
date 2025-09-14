# good reference for emscripten building https://github.com/ocornut/imgui/blob/master/examples/example_sdl3_opengl3/Makefile.emscripten
rm -rf build
mkdir -p build
EMCC_DEBUG=1 emcc -v -o build/index.html $CFLAGS -Iext -O1 -std=gnu23  src/platform_all.c src/profiler.c src/game.c  -s USE_SDL=3 -s DISABLE_EXCEPTION_CATCHING=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=0 -s ASSERTIONS=1 -s FULL_ES3=1 --preload-file data



