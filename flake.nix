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
      rec {
        devShells = rec {
          default = develop;
          develop = pkgs.mkShell {
            nativeBuildInputs = with pkgs; [
              autoreconfHook
              gdb
              indent
              clang-tools
            ];
          };
          use = pkgs.mkShell {
            nativeBuildInputs = [ packages.diredit ];
          };
        };

        packages = rec {
            default = diredit;
            diredit = pkgs.stdenv.mkDerivation {
                pname = "diredit";
                version = "1.0";
                src = ./.;
                nativeBuildInputs = with pkgs; [ autoreconfHook ];
              	preConfigure = ''
              		autoreconf --install
              	'';
              };
          };
      }
    );
}
