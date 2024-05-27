{
  description = "meta24";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.11";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem
      (system:
        let
          pkgs = import nixpkgs {
            inherit system;
          };
        in
        {
          devShells.default =
            pkgs.mkShell {
              buildInputs = with pkgs; [
                bazel
                clang
                git
              ];

              shellHook = ''
                export PS1='[\[\033[1;32m\]meta24\[\033[0m\]:\[\033[1;34m\]\w\[\033[0m\]]\$ '
                # Sync meta libs
                git submodule update --init --recursive
              '';
            };
        }
      );
}
