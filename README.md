# CheapChatContext

A small command-line tool for feeding chatbots context about your project.

Run it from any project directory (or add its folder to `PATH` and run it
from anywhere inside a project) and it will:

1. Scan the current directory, honoring every `.gitignore` it finds
   (root-level and nested).
2. Render that scan as a simple ASCII directory tree.
3. Read `AGENTS.md` from the project root, if present.
4. Read `ccc.config.json` from the project root, if present, and pull in
   whatever extra files/folders it lists.
5. Assemble all of that into one Markdown document and copy it straight to
   your clipboard, ready to paste into a chat.

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

## Usage

```
ccc [options]

Options:
  -o, --output <file>   Also write the generated context to <file>
      --no-clipboard    Don't touch the clipboard (useful with --output)
  -h, --help             Show usage
```

Just running `ccc` with no arguments is the normal case: it scans the
current directory and copies the result to your clipboard.

### `ccc.config.json`

Lets you pull specific extra files (or whole folders) into the generated
context, beyond what's already shown in the project structure tree.
Useful for things like a `package.json`, a design doc, or a folder of
schema files that you always want a chatbot to see in full.

```jsonc
{
  "include": [
    "docs", // a folder - every file inside is included, recursively
    "package.json", // a single file
  ],
  "exclude": [
    "docs/internal-notes.md", // skip just this one file from the "docs" include
  ],
}
```

Notes:

- Paths are relative to the project root (the directory you run `ccc`
  from), using forward slashes.
- `exclude` entries can point at a single file or at an entire
  sub-folder; either way, everything under that path is skipped from the
  expanded results.
- Folder includes are an explicit request from you, so they deliberately
  bypass `.gitignore` - if you list a folder, you get everything in it
  (minus anything you've excluded).
- Files that look binary (contain a NUL byte) are noted as omitted rather
  than dumped into the output, since that would just be garbage text and
  bloat the clipboard contents.

### `AGENTS.md`

If a file named `AGENTS.md` exists in the project root, its contents are
included verbatim (inside a fenced code block) under its own heading.

## Output format

```markdown
# Context:

## Project structure:

\`\`\`
(ASCII tree)
\`\`\`

## AGENTS.md

\`\`\`
(contents of AGENTS.md)
\`\`\`

## Other important files

### path/to/file

\`\`\`
(file contents)
\`\`\`
```

Sections with nothing to show (no `AGENTS.md`, no `ccc.config.json`
entries) are omitted entirely. If a file's own contents contain a run of
backticks, the surrounding fence is automatically made one backtick
longer than the longest run found inside it - the standard Markdown way
of nesting code fences safely, rather than mangling the file's contents.

## How `.gitignore` matching works

`ccc` implements its own `.gitignore`-pattern matcher covering the rules
people actually rely on day to day: `*`, `?`, `[...]` character classes,
`**` for matching across directories, `/`-anchored vs. any-depth
patterns, directory-only (`pattern/`) patterns, and `!` negation -
including negating a file inside a directory excluded by a _less_
specific `.gitignore` higher up the tree. It is not a byte-for-byte
reimplementation of git's own matcher (a handful of obscure edge cases
around escaped characters aren't handled), but it covers the patterns
found in the overwhelming majority of real-world `.gitignore` files. The
`.git` directory itself is always skipped, regardless of `.gitignore`
content.

## Project layout

```
ccc/
|-- CMakeLists.txt
|-- src/
|   |-- main.cpp              entry point / CLI handling
|   |-- GitIgnore.{h,cpp}      .gitignore parsing + matching engine
|   |-- DirectoryScanner.{h,cpp}  builds the in-memory file tree
|   |-- TreeRenderer.{h,cpp}   renders the tree as ASCII art
|   |-- Config.{h,cpp}        parses ccc.config.json
|   |-- MarkdownBuilder.{h,cpp}  assembles the final context string
|   |-- FileUtils.{h,cpp}      file read/write + binary detection helpers
|   |-- Clipboard.h            platform-agnostic clipboard interface
|   |-- Clipboard_Windows.cpp  Win32 clipboard implementation
|   `-- Clipboard_Linux.cpp    shells out to wl-copy/xclip/xsel
`-- third_party/
    `-- nlohmann/json.hpp     vendored JSON library (MIT licensed)
```

Each piece is independent and unit-testable on its own; only `main.cpp`
wires them together, and only the two `Clipboard_*.cpp` files are
platform-specific.
