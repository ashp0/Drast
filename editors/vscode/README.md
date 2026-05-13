# Drast VS Code Extension

The publishable VS Code extension lives in `drast-language/`. It provides TextMate syntax highlighting, editor language configuration, fallback keyword/snippet completions, and a TypeScript Language Server with keyword, snippet, symbol, hover, definition, and basic diagnostic support.

## Install From Source

```sh
cd editors/vscode/drast-language
npm install
npm run compile
npm install -g @vscode/vsce
vsce package
```

Then install the generated `.vsix` in VS Code:

1. Open the Extensions view.
2. Select the `...` menu.
3. Choose `Install from VSIX...`.
4. Pick the generated `drast-language-0.1.0.vsix`.

For development, open `editors/vscode/drast-language` in VS Code and run the extension host launch configuration you create locally, or use `npm run watch` while debugging. The `Drast: Restart Language Server` command restarts the bundled language server without reloading VS Code.

## Notes

- Files ending in `.drast` activate the extension.
- Syntax grammar is based on `SYNTAX.md` and `src/lexer.drast`, with the planned `nothing` keyword included.
- The language server scans workspace `.drast` files with a regex-level pass. It recognizes current Drast top-level function syntax and also accepts a compatibility `fn name` pattern for symbol extraction.
