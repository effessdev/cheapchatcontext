# CheapChatContext developer documentation

## Building

Requires a C++17 compiler and CMake 3.16+.

### Windows (Visual Studio)

```bat
cmake -S . -B build -A x64
cmake --build build --config Release
```

The executable will be at `build\Release\ccc.exe`. Put its folder on your
`PATH` (System Properties > Environment Variables) so you can call `ccc`
from any project directory.

To create a Windows installer, install Inno Setup, open `ccc-installer.iss`
in the Inno Setup Compiler, and press `F9` (Compile). The generated
installer will be written to the `output/` directory.

### Windows (MinGW) / Linux / macOS

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The executable will be at `build/ccc` (or `build/ccc.exe` on Windows).

On Linux, copying to the clipboard shells out to whichever of these is
installed: `wl-copy` (Wayland), `xclip`, or `xsel` (X11). Install one of
them, e.g.:

```bash
sudo apt install xclip
```
