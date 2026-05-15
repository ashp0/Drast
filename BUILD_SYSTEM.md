# Drast Native Build System

## Overview

Every Drast package is configured by `package.txt` at the project root. The Drast CLI reads that manifest, transpiles Drast sources to C++, marks generated C++ read-only, generates an internal xmake project, and invokes xmake as the C++ backend.

The user-facing build surface is:

```sh
drast init [ProjectName]
drast build [target]
drast run [target]
drast <target>
drast help
```

## Manifest Format

`package.txt` is line-oriented and indentation-friendly:

```txt
package Drast
version 0.2.0
default drast

target drast
	kind binary
	entry src/main.drast
	output .drast/build/bin/drast
	generated .drast/build/generated/drast
	include .
	cxx c++17

target test
	kind command
	depends drast
	command tests/run_tests.sh {output:drast} {root}
```

Supported target fields:

- `kind`: `binary` or `command`; `library` and `embed` are reserved and produce clear diagnostics.
- `entry`: Drast source entry point for binary targets.
- `output`: binary output path.
- `generated`: generated C++ directory.
- `depends`: one or more target names.
- `include`, `cxxfile`, `link`, `linkdir`, `define`, `cxxflag`, `ldflag`: passed through to the xmake backend.
- `prebuild`, `postbuild`, `command`: shell commands with placeholders.

Command placeholders:

- `{root}`: package root.
- `{package}`: package name.
- `{target}`: current target name.
- `{output}`: current target output path.
- `{output:name}` and `{target:name}`: named target output/name.

## Architecture

- `src/main.drast` is a thin entry point.
- `src/cli.drast` owns command parsing and project initialization.
- `src/package.drast` owns manifest structs and parsing.
- `src/build_system.drast` owns graph resolution, dependency ordering, transpilation, generated-file permissions, and command target execution.
- `src/xmake_backend.drast` owns internal xmake project generation/invocation.
- `src/platform.drast` wraps runtime filesystem/process helpers so future direct C++ interop can replace `drast_runtime.hpp` usage in one place.

Future extension points are already represented in the manifest model for external C++ linking, multiple targets, and embedding-oriented target kinds.

## Bootstrap

The self-hosting chain is documented in [bootstrap/CHAIN.md](bootstrap/CHAIN.md). The recovery path is:

```sh
./bootstrap.sh --check
```

That verifies the pinned legacy seed, builds a stage-0 compiler, builds the package-era compiler, and asks the rebuilt compiler to build itself again.
