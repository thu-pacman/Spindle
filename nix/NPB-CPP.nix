{ lib
, llvmPackages
, writeText
, fetchFromGitHub
, Spindle
}:
let
  config = writeText "config" ''
    CC         = clang++
    CLINK      = $(CC)
    C_LIB      = -lm -lstracer
    C_INC      = -I../common
    CFLAGS     = -O3 -mcmodel=medium -fpass-plugin=${Spindle}/lib/SpindlePass.so -fno-unroll-loops -fno-vectorize
    CLINKFLAGS = -O3 -mcmodel=medium
    UCC        = $(CC)
    BINDIR     = $(out)/bin
    RAND       = randdp
    WTIME      = wtime.cpp
  '';
in
llvmPackages.stdenv.mkDerivation rec {
  pname = "NPB-CPP";
  version = "1.0";

  src = fetchFromGitHub {
    owner = "GMAP";
    repo = pname;
    rev = "v${version}";
    hash = "sha256-ALkD0Qr4meoIQZd1jtEirm9QsXZGi3YTEkbzGdqVFKI=";
  };

  sourceRoot = "source/NPB-SER";

  outputs = [ "out" "strace" ];

  buildInputs = [ Spindle ];

  postConfigure = ''
    ln -sf ${config} config/make.def
  '';

  buildPhase = ''
    mkdir -p "$out/bin" "$strace"
    for workload in BT CG EP FT IS LU MG SP; do
      make "$workload" CLASS=C 2> "$strace/$workload.log"
      install -Dm644 "$workload/strace.log" "$strace/$workload.strace"
    done
  '';

  dontInstall = true;
}
