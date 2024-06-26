{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = inputs@{ self, ... }:
    inputs.flake-utils.lib.eachDefaultSystem (
      system:
        let
          pkgs = import inputs.nixpkgs {inherit system;};
        in
        {
          devShells.default = pkgs.mkShell {
            nativeBuildInputs = with pkgs; [
              gdb
              indent
              clang-tools
            ];
          };

          packages.default = pkgs.stdenv.mkDerivation {
            pname = "diredit";
            version = "1.0";
            src = ./.;
            installPhase = ''
              PREFIX=$out make install
            '';
          };
        }
      );
}
