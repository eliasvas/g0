# I don't know what this is (yet)

## Deps
```terminal
sudo dnf install git clang SDL3 SDL3-devel glew glew-devel -y
git lfs install
```
## Cloning
This repo uses git LFS for assets and a submodule for stb libraries, you can:
```bash
git clone --recursive https://github.com/eliasvas/g0
```
## Building
### Linux
```bash
./build.sh
```
### Web (From Linux)
first install and activate [emscripten](https://emscripten.org/docs/getting_started/downloads.html)
```bash
./build_web.sh
cd build
python -m http.server
->localhost:8000 in your browser
```
