# Blocky Blocks

A minecraft clone, written in C99 with OpenGL

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

Windows:
```
IMPORTANT: Always open the Visual Studio x64 developer shell before compiling
(x64 Native Tools Command Prompt for VS 2022, or in any PowerShell:
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64).
Without it, cl.exe, cmake and ninja are not on PATH.

Debug:
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
build\blockyblocks.exe

Release:
cmake -G Ninja -B build/release -DCMAKE_BUILD_TYPE=Release
cmake --build build/release
build\release\blockyblocks.exe
```

