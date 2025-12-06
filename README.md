# Transparent App Launcher

A lightweight, transparent, fullscreen application launcher written in C using Raylib. It scans for `.desktop` files in your system and displays them in a grid.

## Features

*   **Transparent Window**: Overlays your desktop with a semi-transparent background.
*   **Grid Layout**: Displays applications in a 4x5 grid per page.
*   **Pagination**: Supports multiple pages of applications with navigation buttons.
*   **Icon Support**:
    *   Supports standard PNG icons.
    *   **SVG Support**: Automatically converts SVG icons to PNG using ImageMagick's `convert` tool.
    *   **Caching**: Caches converted icons in `/tmp/` for fast startup on subsequent runs.
*   **Touch/Click Friendly**: Launch apps with a single click or tap.

## Dependencies

To build and run this launcher, you need the following:

*   **GCC** (or Clang)
*   **Raylib**: The library files (`libraylib.a` or `libraylib.so`) and headers (`raylib.h`) should be in the project directory or installed system-wide.
*   **ImageMagick**: Required for SVG icon conversion.
    *   Ensure `convert` is in your system PATH.
    *   Install via your package manager (e.g., `sudo apt install imagemagick` on Debian/Ubuntu, or `sudo pacman -S imagemagick` on Arch).

## Building

1.  Ensure you have the dependencies installed.
2.  Run `make` in the project directory:

```bash
make
```

This will produce an executable named `launcher`.

## Usage

Run the launcher:

```bash
./launcher
```

*   **Navigate**: Click the `<` and `>` buttons at the bottom to change pages.
*   **Launch**: Click an app icon to launch it. The launcher will close automatically.
*   **Exit**: Press `ESC` to close the launcher without launching anything.

## Troubleshooting

*   **Missing Icons**: If an icon is missing, it might be in a format not supported or in a non-standard path. The launcher searches common paths like `/usr/share/icons` and `~/.local/share/icons`.
*   **SVG Issues**: If SVG icons aren't loading, verify that `convert` is accessible by running `convert --version` in your terminal.
