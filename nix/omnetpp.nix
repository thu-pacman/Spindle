{ lib
, llvmPackages
, fetchFromGitHub
, meson
, ninja
, Spindle
}:

llvmPackages.stdenv.mkDerivation {
  name = "omnetpp";

  src = fetchFromGitHub {
    owner = "NickCao";
    repo = "520.omnetpp_r";
    rev = "e4d5b86d09b76adb1ab4661c12b6d47c2c5c1e3e";
    hash = "sha256-kApwQU7FcxRIoRi+Srw+2QX/HGeH0/PeBDy5CbUCI1Q=";
  };

  nativeBuildInputs = [
    meson
    ninja
  ];

  NIX_CFLAGS_COMPILE = [
    "-Wno-reserved-user-defined-literal"
  ];

  doCheck = true;
}
