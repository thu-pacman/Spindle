{ lib
, llvmPackages
, fetchFromGitHub
, meson
, ninja
, pkg-config
, bison
, flex
, talloc
, libtins
, nghttp2
, curl
, libyaml
, libgcrypt
, libmicrohttpd
, libbson
, mongoc
, libidn
, openssl
, gnutls
, lksctp-tools
, coreutils
, Spindle
, nixosTest
}:

llvmPackages.stdenv.mkDerivation rec {
  pname = "open5gs";
  version = "2.4.10";

  src = fetchFromGitHub {
    owner = pname;
    repo = pname;
    rev = "v${version}";
    hash = "sha256-hRlZxz9qPUfJjLA2+1GYnYUd7gJ/UEtMci2CNFnn7oM=";
  };

  patches = [
    ./open5gs.patch
  ];

  freeDiameter = fetchFromGitHub {
    owner = pname;
    repo = "freeDiameter";
    rev = "r1.5.0";
    hash = "sha256-dqdBy/kFZW9EvwX4zdwpb3ZGYcSjfH9FqvSHDaasdR4=";
  };

  postPatch = ''
    ln -s $freeDiameter subprojects/freeDiameter
  '';

  hardeningDisable = [
    "all"
  ];

  NIX_CFLAGS_COMPILE = [
    "-Wno-compound-token-split-by-macro"
    "-Wno-format"
    "-Wno-unused-command-line-argument"
    "-fpass-plugin=${Spindle}/lib/SpindlePass.so"
    "-lstracer"
  ];

  mesonFlags = [
    "-Dlocalstatedir=/var"
  ];

  nativeBuildInputs = [
    meson
    ninja
    pkg-config
    bison
    flex
  ];

  buildInputs = [
    talloc
    libtins
    nghttp2
    curl
    libyaml
    libgcrypt
    libmicrohttpd
    libbson
    mongoc
    libidn
    openssl
    gnutls
    lksctp-tools
    Spindle
  ];

  postInstall = ''
    for file in configs/systemd/*.service; do
      substituteInPlace $file --replace /bin/kill ${coreutils}/bin/kill
      install -Dm644 $file $out/lib/systemd/system/$(basename $file)
    done
	install -Dm644 strace.log "$strace/strace.log"
  '';

  outputs = [ "out" "strace" ];

  passthru.tests =
    let
      services = [
        "open5gs-amfd"
        "open5gs-ausfd"
        "open5gs-bsfd"
        "open5gs-hssd"
        "open5gs-mmed"
        "open5gs-nrfd"
        "open5gs-nssfd"
        "open5gs-pcfd"
        "open5gs-pcrfd"
        "open5gs-sgwcd"
        "open5gs-sgwud"
        "open5gs-smfd"
        "open5gs-udmd"
        "open5gs-udrd"
        "open5gs-upfd"
      ];
    in
    {
      basic = nixosTest {
        name = pname;
        nodes.machine = { config, pkgs, lib, ... }: {
          services.mongodb.enable = true;
          users.users.open5gs = {
            isSystemUser = true;
            group = "open5gs";
          };
          users.groups.open5gs = { };
          systemd.tmpfiles.rules = [
            "d /var/log/open5gs 0755 open5gs open5gs - -"
          ];
          systemd.packages = [ pkgs.open5gs ];
          systemd.services = lib.genAttrs services (_: {
            wantedBy = [ "multi-user.target" ];
            after = [ "mongodb.service" ];
            serviceConfig.AmbientCapabilities = [ "CAP_NET_ADMIN" ];
          });
        };
        testScript = lib.flip lib.concatMapStrings services (service: ''
          machine.wait_for_unit("${service}.service")
        '');
      };
    };
}
