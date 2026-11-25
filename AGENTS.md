# Blocky Blocks

A minecraft clone, written in C99 with OpenGL

Uses GLAD to load OpenGL
Uses SDL3 for platform independence

Build:
```
Debug:
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_C_COMPILER_LAUNCHER=ccache \
    -DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=mold \
cmake --build build
build/blockyblocks

Release:
cmake -G Ninja -B build/release -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_C_COMPILER_LAUNCHER=ccache \
    -DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=mold \
cmake --build build/release
build/release/blockyblocks
```

