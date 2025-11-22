{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    wxGTK32
    # Add other dependencies your project needs
    udev
    xorg.libX11

    vulkan-headers
    vulkan-loader

    gsettings-desktop-schemas
    glib
    gtk3
  ];

  nativeBuildInputs = with pkgs; [
    cmake
    pkg-config
  ];
  
  shellHook = ''
    export CMAKE_PREFIX_PATH="${pkgs.wxGTK32}:$CMAKE_PREFIX_PATH"
    export XDG_DATA_DIRS="$GSETTINGS_SCHEMAS_PATH:$XDG_DATA_DIRS"
    cd build
  '';
}
