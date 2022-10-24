{ lib
, llvmPackages
, fetchFromGitHub
, cmake
, lksctp-tools
}:

llvmPackages.stdenv.mkDerivation rec {
  pname = "UERANSIM";
  version = "3.2.6";

  src = fetchFromGitHub {
    owner = "aligungr";
    repo = pname;
    rev = "v${version}";
    hash = "sha256-wl5AiLxMgANV9El/it0Ihegv0+rzIntRATqDVTH8SUA=";
  };

  nativeBuildInputs = [
    cmake
  ];

  buildInputs = [
    lksctp-tools
  ];

  installPhase = ''
    install -Dm755 nr-ue        $out/bin/nr-ue
    install -Dm755 nr-gnb       $out/bin/nr-gnb
    install -Dm755 nr-cli       $out/bin/nr-cli
    install -Dm755 libdevbnd.so $out/lib/libdevbnd.so
  '';
}
