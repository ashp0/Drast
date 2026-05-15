# Drast Bootstrap Chain

This directory is the recovery map for self-hosting. The binaries here are not convenience artifacts; they are the chain of custody that keeps Drast rebuildable if a local compiler is lost.

## Chain

1. `bootstrap/legacy/darwin-arm64/drastc`
   - Version: legacy pre-`package.txt` compiler.
   - Platform: macOS arm64.
   - SHA-256: `f147f2bebe62f7a0226d721a88c04c781c300e7325fcda8bcb87d20deaed534c`.
   - Invocation style: `drastc src/main.drast -o build/bootstrap/stage0/drast.cpp`.

2. `releases/v0.2.0/darwin-arm64/drast`
   - Version: first Drast-native package build.
   - Platform: macOS arm64.
   - SHA-256: `b8a9179d888216bbe7dee7fee04c6298967c94b723e3796dec081efed2a71b9b`.
   - Invocation style: `drast build drast`.

3. Future releases
   - Add each release under `releases/<version>/<platform>/drast`.
   - Add a sibling `.sha256` file.
   - Extend this chain before deleting or rotating any local build artifacts.

## Recovery

From a clean checkout on macOS arm64:

```sh
./bootstrap.sh --check
```

The script verifies the legacy seed hash, builds a stage-0 compiler from `src/main.drast`, uses that compiler to run `drast build drast`, and then asks the rebuilt compiler to build itself again.

To start from a newer known-good Drast binary instead of the legacy seed:

```sh
./bootstrap.sh --from-new releases/v0.2.0/darwin-arm64/drast --check
```

## Policy

- Never delete `bootstrap/legacy/**`.
- Never overwrite an existing `releases/<version>/**` binary; add a new version instead.
- Publish release binaries to GitHub Releases for the matching `v*` tag.
- CI archives a fresh main-branch binary on every merge to `main`.
