{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    cmake
    wxGTK32
    # Add other dependencies your project needs
    udev
    xorg.libX11
  ];
  
  shellHook = ''
    export CMAKE_PREFIX_PATH="${pkgs.wxGTK32}:$CMAKE_PREFIX_PATH"
  '';
}
