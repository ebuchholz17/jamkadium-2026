mkdir -p ../../build 
mkdir -p ../../build/linux

pushd ../../build/linux

compilerFlags="-O0 -g -DPLATFORM_LINUX"
clang $compilerFlags -shared -fPIC -Wl,--no-undefined -o game.so.temp ../../code/game/gng_game.c
mv game.so.temp game.so
clang $compilerFlags -o gng ../../code/linux/linux_platform.c -lSDL3 -lGLEW -lGLU -lGL

popd
