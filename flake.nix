{
  description = "meta24";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.11";
  };

  outputs = { self, nixpkgs, ... }:
    let system = "x86_64-linux"; in
    {
      devShells.${system}.default =
        let
          pkgs = import nixpkgs {
            inherit system;
          };
        in
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
    };
}
