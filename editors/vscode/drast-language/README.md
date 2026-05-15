# Drast Language Support

World-class VS Code support for Drast, a modern systems language that keeps C++ interop close while giving developers a cleaner, indentation-sensitive surface to work in.

## Features

- Precise TextMate highlighting for Drast declarations, generics, pointer and reference syntax, enums, data enum payloads, protocols, impl blocks, match arms, comments, strings, runtime helpers, word operators, and imported C++ names.
- Rich hover cards for Drast functions, variables, parameters, structs, fields, enum variants, protocols, methods, runtime helpers, and C++ symbols imported through `use`.
- Context-aware completions for keywords, snippets, local variables, parameters, workspace symbols, struct fields, impl methods, enum variants, shorthand variants, runtime helpers, C++ headers, and common standard-library/Arduino imports.
- Document and range formatting for indentation, operator spacing, declaration spacing, match arms, top-level blank lines, and Drast line continuations.
- Clean diagnostics for malformed `use` statements, unclosed strings/comments/brackets, unsupported word operators, removed `fn` syntax, suspicious declarations, and empty block structure.
- Marketplace-ready snippets for functions, structs, enums, impl blocks, protocols, conformances, match expressions, try/catch, comptime blocks, operators, and new project entry points.

## Drast Intelligence

The bundled language server indexes every `.drast` file in the workspace and updates live as files open, change, and save. It understands the declaration forms documented in `SYNTAX.md`: comma-return function signatures, semicolon parameter/type separators, `maybe` returns, `impl Type as Protocol`, nested enum names such as `Dog.Breed`, data enum constructors, and the backtick operator used for pointers, dereference, and generic arguments.

When a Drast file imports a C++ header with `use file native.hpp`, `use 'native.hpp'`, or `use Arduino.h`, the server surfaces exported header symbols in completion and hover cards. Local headers are scanned directly; common system and embedded headers get curated signatures so interop feels immediate.

## Install From Source

```sh
npm install
npm run compile
npm install -g @vscode/vsce
npm run package
```

In VS Code, run `Extensions: Install from VSIX...` and select the generated package.

## Development

Use `npm run watch` while editing TypeScript. Run `Drast: Restart Language Server` from the command palette after changing server behavior.

The extension ships as a self-contained language experience: TextMate grammar, language configuration, snippets, icon assets, marketplace metadata, and a TypeScript LSP server. It does not require the Drast compiler to be on `PATH` for editor intelligence.
