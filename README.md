# CheapChatContext

## Installation (Windows)

Download and run the installer to install the app.

## Usage

### Optional configuration file

Run the command `ccc config` from the root of your project to create a `ccc.config.json` file with the default configuration. This is optional and can be skipped if you only want the project structure.

### Basic usage

Run the command `ccc` from your project root. This will copy the context into your clipboard, and you can easily paste it into your chatbot.


## Advanced usage

```
ccc [options]

Options:
  -o, --output <file>   Also write the generated context to <file>
      --no-clipboard    Don't touch the clipboard (useful with --output)
  -h, --help            Show usage
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

  
## Output format

```markdown
# Context:

## Project structure:

\`\`\`
(ASCII tree)
\`\`\`

## Other important files

### path/to/file

\`\`\`
(file contents)
\`\`\`
```
