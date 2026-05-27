{
  # insanely hacky but it does work
  description = "Pegasus Frontend - local dev flake";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { nixpkgs, ... }:
    let
      eachSystem = nixpkgs.lib.genAttrs [
        "x86_64-linux"
        "aarch64-linux"
      ];
    in
    {
      devShells = eachSystem (
        system: with nixpkgs.legacyPackages.${system}; {
          default =
            let
              buildInputs =
                (with libsForQt5; [
                  qtbase
                  qtmultimedia
                  qtsvg
                  qtgraphicaleffects
                  qtx11extras
                  qtimageformats
                ])
                ++ [
                  sqlite
                  SDL2
                ];
            in
            mkShell {
              name = "pegasus-frontend-dev";

              nativeBuildInputs = [
                cmake
                libsForQt5.qttools
                libsForQt5.wrapQtAppsHook
                (pkgs.writeShellScriptBin "build" ''
                  cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
                  cmake --build build -j$(nproc)
                '')
                (pkgs.writeShellScriptBin "run" "./build/src/app/pegasus-fe")
              ];

              inherit buildInputs;

              LD_LIBRARY_PATH = lib.makeLibraryPath buildInputs;

              shellHook = ''
                              # Nix setup hooks already populate CMAKE_PREFIX_PATH.
                              # Append explicit -dev outputs so cmake finds Qt5Config etc.
                              export Qt5_DIR="${libsForQt5.qtbase.dev}/lib/cmake/Qt5"
                              export CMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH:${libsForQt5.qtbase.dev}:${libsForQt5.qtmultimedia.dev}:${libsForQt5.qtsvg.dev}:${libsForQt5.qtgraphicaleffects.dev}:${libsForQt5.qtx11extras.dev}:${libsForQt5.qtimageformats.dev}:${libsForQt5.qtdeclarative.dev}:${sqlite.dev}:${SDL2.dev}"

                              # Apply Qt wrapper env vars (QT_PLUGIN_PATH, QML2_IMPORT_PATH, etc.)
                              # without spawning a subshell that would hang direnv/editors.
                              eval $(echo "''${qtWrapperArgs[@]}" | perl -n -e \
                                '$_ =~ s/--prefix (\w+) : ([\w\-.\/]+)/export ''${1}="''${2}:\''${''${1}}";/g; print')

                              # Write .vscode/.env so VS Code / clangd resolve include paths.
                              mkdir -p .vscode
                              cat > .vscode/.env <<EOF
                QT_DIR=${libsForQt5.qtbase.dev}
                QT_INCLUDE_PATH=${libsForQt5.qtbase.dev}/include
                QT_BIN=${libsForQt5.qtbase.bin}/bin
                Qt5_DIR=${libsForQt5.qtbase.dev}/lib/cmake/Qt5
                CMAKE_PATH=${cmake}/bin/cmake
                CMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH
                SDL2_INCLUDE=${SDL2.dev}/include/SDL2
                SQLITE_INCLUDE=${sqlite.dev}/include
                QT_MULTIMEDIA_INCLUDE=${libsForQt5.qtmultimedia.dev}/include
                QT_SVG_INCLUDE=${libsForQt5.qtsvg.dev}/include
                QT_DECLARATIVE_INCLUDE=${libsForQt5.qtdeclarative.dev}/include
                QT_X11EXTRAS_INCLUDE=${libsForQt5.qtx11extras.dev}/include
                EOF

                              # Source the .vscode/.env file into the shell environment
                              set -a
                              source .vscode/.env
                              set +a

                              echo ""
                              echo "  Pegasus Frontend dev shell ready."
                              echo "  Qt ${libsForQt5.qtbase.version} | SDL2 | SQLite"
                              echo ""
                              echo "  Build:  cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
                              echo "          cmake --build build -j\$(nproc)"
                              echo "  Run:    ./build/src/app/pegasus-fe"
                              echo ""
              '';
            };
        }
      );
    };
}
