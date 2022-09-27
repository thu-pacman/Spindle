{
  inputs = {
    nixpkgs.url = "github:NickCao/nixpkgs/nixos-unstable-small";
  };
  outputs = { self, nixpkgs, ... }: with nixpkgs.legacyPackages.x86_64-linux; {
    devShells.x86_64-linux.default = mkShell {
      nativeBuildInputs = [ cmake ninja llvmPackages_14.clang gdb ];
      buildInputs = [ llvmPackages_14.llvm ];
    };
    packages.x86_64-linux.open5gs = callPackage ./nix/open5gs.nix {
      llvmPackages = llvmPackages_14;
    };
  };
}
