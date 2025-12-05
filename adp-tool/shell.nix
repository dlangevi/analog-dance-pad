{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    vcpkg
    cmake
    ninja
    pkg-config-unwrapped
    gcc
    udev

    avrdude

    xorg.libX11
    xorg.libXrandr
    xorg.libXinerama
    xorg.libXcursor
    xorg.libXi
    xorg.libXext
    libGL
    libGLU

    mesa
  ];
  
  shellHook = ''
    export CC=${pkgs.gcc}/bin/gcc
    export CXX=${pkgs.gcc}/bin/g++
    export CMAKE_MAKE_PROGRAM=${pkgs.ninja}/bin/ninja
    export VCPKG_ROOT=${pkgs.vcpkg}/share/vcpkg
    # export VCPKG_FORCE_SYSTEM_BINARIES=1
    export PKG_CONFIG=${pkgs.pkg-config-unwrapped}/bin/pkg-config
  '';
}
