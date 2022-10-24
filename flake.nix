{
  inputs = {
    nixpkgs.url = "github:NickCao/nixpkgs/nixos-unstable-small";
  };
  outputs = { self, nixpkgs, ... }:
    let
      pkgs = import nixpkgs {
        system = "x86_64-linux";
        overlays = [ self.overlays.default ];
      };
    in
    {
      overlays.default = super: self: {
        open5gs = super.callPackage ./nix/open5gs.nix {
          llvmPackages = super.llvmPackages_14;
        };
        UERANSIM = super.callPackage ./nix/UERANSIM.nix {
          llvmPackages = super.llvmPackages_14;
        };
        Spindle = with super; llvmPackages_14.stdenv.mkDerivation {
          name = "Spindle";
          src = ./.;
          nativeBuildInputs = [ cmake ];
          buildInputs = [ llvmPackages_14.llvm ];
        };
      };
      packages.x86_64-linux = {
        inherit (pkgs) Spindle open5gs UERANSIM;
      };
      devShells.x86_64-linux.default = with pkgs; mkShell {
        nativeBuildInputs = [ cmake ninja llvmPackages_14.clang gdb ];
        buildInputs = [ llvmPackages_14.llvm ];
      };
    };
}
