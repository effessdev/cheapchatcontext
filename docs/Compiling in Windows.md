# Compiling in Windows (by Gemini 3.1 Flash Lite)

To compile a CMake project in VS Code, follow these steps for the most efficient setup:

## Prerequisites

Install the "CMake Tools" extension by Microsoft (this automatically installs the necessary "C/C++" extension).

## The Workflow

1. **Open Folder:** Open your project root in VS Code.
2. **Configure:** If a CMakeLists.txt is detected, the extension will prompt you to select a Kit (your compiler, e.g., "Visual Studio Community 2022 Release - amd64").
3. **Build:** Click the Build button in the bottom status bar, or press F7.

## Note

Ensure you have the Visual Studio Build Tools installed on your system, as VS Code is just an editor and requires the underlying MSVC compiler to perform the actual compilation.
