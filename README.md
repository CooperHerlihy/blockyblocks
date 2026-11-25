# Blocky Blocks

A Minecraft clone written in C11 with OpenGL.

## Dependencies

To build:

- A C11 compiler (Clang or GCC)
- CMake 3.20 or newer
- Ninja (or Make)
- SDL3 development package

To run:

- An OpenGL 3.3 capable driver (normally already installed on a desktop system)

Optional: `ccache` and the `mold` linker for faster builds, `gdb` or `valgrind`
for debugging.

GLAD is vendored in `vendor/glad`.

## Building

Install the build dependencies with your package manager. For example:

```sh
# Debian/Ubuntu
sudo apt install clang cmake ninja-build libsdl3-dev

# Fedora
sudo dnf install clang cmake ninja-build SDL3-devel

# Arch
sudo pacman -S clang cmake ninja sdl3
```

Then configure and build:

```sh
cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/blockyblocks
```

For a release build, use `-DCMAKE_BUILD_TYPE=Release`.

## NixOS

A flake is provided with all dependencies and build scripts. Enter the
development shell, then use `debug` or `release`:

```sh
nix develop
debug
```

## License

MIT, see [LICENSE.md](LICENSE.md).
