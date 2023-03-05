{ pkgs }: {
	deps = [
		pkgs.gcc
		pkgs.cmake
		pkgs.gdb
    pkgs.curl
    pkgs.libgit2
	];
}