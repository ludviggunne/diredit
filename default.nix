{ pkgs ? import <nixpkgs> {}, ... }:

pkgs.stdenv.mkDerivation {
	pname = "direnv";
	version = "1.0";
	src = ./.;
	nativeBuildInputs = with pkgs; [ autoreconfHook ];
	preConfigure = ''
		autoreconf --install
	'';
}
