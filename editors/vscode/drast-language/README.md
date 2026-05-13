# Drast Language Support

VS Code language support for Drast.

## Features

- Syntax highlighting for Drast keywords, comments, strings, chars, literals, operators, declarations, types, runtime helpers, and punctuation.
- Language configuration for comments, brackets, auto-closing pairs, indentation, and word selection.
- Fallback keyword and snippet completions in the extension host.
- Language Server Protocol support for keyword/snippet completions, workspace symbol completions, hover text, go to definition, and basic diagnostics.

## Snippets

- `if` expands to an indented conditional block.
- `while` expands to a loop block.
- `for` expands to a foreach loop block.
- `nothing` inserts the planned empty-block keyword.

## Install From Source

```sh
npm install
npm run compile
npm install -g @vscode/vsce
vsce package
```

In VS Code, run `Extensions: Install from VSIX...` and select the generated `drast-language-0.1.0.vsix`.

## Development

Use `npm run watch` while editing TypeScript. Run `Drast: Restart Language Server` from the command palette after changing server behavior.

The LSP symbol scanner reads all workspace `.drast` files and updates its in-memory table when documents open, change, or save. It extracts Drast-style top-level functions and globals, methods in `impl`/`protocol` blocks, structs, enums, protocols, and compatibility `fn name` declarations.
