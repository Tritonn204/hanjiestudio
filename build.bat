cd "src"
g++ *.cpp *.c -o "../build/nonogrammer.exe" -Iinclude -Llib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image -Wno-narrowing
cd "../build"
start nonogrammer.exe
