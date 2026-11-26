{
    description = "Blocky Blocks - a Minecraft clone in C11 with OpenGL";

    inputs.nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";

    outputs = { self, nixpkgs }: let
        systems = [ "x86_64-linux" "aarch64-linux" ];
        forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f system);
    in {
        devShells = forAllSystems (system: let
            pkgs = nixpkgs.legacyPackages.${system};
        in {
            default = pkgs.mkShell.override {
                stdenv = pkgs.clangStdenv;
            } {
                name = "blockyblocks";

                packages = with pkgs; [
                    clang-tools
                    cmake
                    ninja
                    mold
                    ccache
                    sdl3
                    gdb
                    valgrind

                    (writeShellScriptBin "debug" ''
                        cmake -G Ninja -B build -DCMAKE_BUILD_TYPE=Debug \
                            -DCMAKE_C_COMPILER=clang \
                            -DCMAKE_C_COMPILER_LAUNCHER=ccache \
                            -DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=mold \
                        && cmake --build build \
                        && build/blockyblocks
                    '')
                    (writeShellScriptBin "release" ''
                        cmake -G Ninja -B build/release -DCMAKE_BUILD_TYPE=Release \
                            -DCMAKE_C_COMPILER=clang \
                            -DCMAKE_C_COMPILER_LAUNCHER=ccache \
                            -DCMAKE_EXE_LINKER_FLAGS=-fuse-ld=mold \
                        && cmake --build build/release \
                        && build/release/blockyblocks
                    '')
                    (writeShellScriptBin "san" ''
                        cmake -G Ninja -B build/san -DCMAKE_BUILD_TYPE=Debug \
                            -DCMAKE_C_COMPILER=clang \
                            -DCMAKE_C_COMPILER_LAUNCHER=ccache \
                            -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=mold -fsanitize=address,undefined" \
                            -DCMAKE_C_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer" \
                        && cmake --build build/san \
                        && build/san/blockyblocks
                    '')
                ];

                LD_LIBRARY_PATH = with pkgs; lib.makeLibraryPath [
                    libGL
                ];
            };
        });
    };
}
