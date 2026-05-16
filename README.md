# Drast

Drast is a small self-hosted programming language toolchain. The compiler is written in Drast, transpiles to C++, and now builds through a Drast-native package workflow.

## Build

Drast projects are configured with `package.txt`. From this repository root:

```sh
./bootstrap.sh --check
./build/bin/drast build
./build/bin/drast build test
```

The CLI invokes xmake internally for C++ compilation; users should not call xmake directly for normal Drast builds.

## CLI

```sh
drast init [ProjectName]
drast build [target]
drast run [target]
drast <target>
drast help
```

Generated C++ is written under `build/generated/<target>/` and marked read-only so hand edits are not lost silently on the next build. Package-managed binaries, generated sources, backend project files, objects, and caches all live under `build/`.

## Package Example

```txt
package Hello
version 0.1.0
default hello

target hello
	kind binary
	entry main.drast
	output build/bin/hello
	generated build/generated/hello
	include .
	cxx c++17
```

## Bootstrap And Releases

Self-hosting recovery is documented in [bootstrap/CHAIN.md](bootstrap/CHAIN.md). The pinned legacy seed lives in `bootstrap/legacy/darwin-arm64/` and release binaries live in `releases/<version>/<platform>/`.

CI rebuilds Drast on every main-branch merge and archives the resulting binary. Tagged `v*` builds publish release assets to GitHub Releases.

## Repository Map

- `src/` contains the compiler, CLI, package parser, and build system.
- `runtime/` is legacy; generated C++ now emits requested headers and inline support directly.
- `tests/` contains compiler fixtures plus CLI/package integration checks.
- `Examples/` contains sample Drast programs.
- `SYNTAX.md` documents the implemented language syntax.
