{ lib
, llvmPackages
, writeText
, fetchFromGitHub
, Spindle
, benchmark ? "MG"
, class ? "C"
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
    BINDIR     = ../bin
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
  makeFlags = [
    benchmark
    "CLASS=${class}"
  ];
  installPhase = let name = lib.strings.toLower benchmark; in ''
    install -Dm755 ./bin/${name}.${class}    $out/bin/${name}.${class}
    install -Dm644 ./${benchmark}/strace.log $strace/strace.log
  '';
  NIX_CFLAGS_COMPILE = [
  ];
}
