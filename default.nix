{ pkgs ? import <nixpkgs> {}, ... }:

pkgs.stdenv.mkDerivation {
	pname = "diredit";
	version = "1.0";
	src = ./.;
	nativeBuildInputs = with pkgs; [ autoreconfHook ];
	preConfigure = ''
		autoreconf --install
	'';
}
