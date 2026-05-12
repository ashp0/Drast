# ══════════════════════════════════════════════════════════════════════════════
#  Drast Compiler — Makefile
#  Stages: prebuilt (dist/drastc-darwin-arm64) → drast_v2 → drast_v3 (self-hosted) → release/dist
# ══════════════════════════════════════════════════════════════════════════════

CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wno-unused-parameter -g -O0 -MMD -MP -I. -Iruntime
OPTFLAGS := -O3 -DNDEBUG -flto

PREBUILT_COMPILER := dist/drastc-darwin-arm64
SRC_DIR       := src
BUILD         := build
RELEASE_DIR   := $(BUILD)/release
DIST_DIR      := dist

# Override with: make self-run FILE=Examples/foo.drast
FILE ?= $(SRC_DIR)/main.drast

# Cross-compilation via zig (brew install zig)
ZIG := zig
ZIGFLAGS := -std=c++17 -O3 -I. -Iruntime

DRAST_MAIN := $(SRC_DIR)/main.drast

# ── Phony targets ─────────────────────────────────────────────────────────────

.PHONY: all \
        run \
        self-build self-run self-release self-examples \
        dist \
        release run-examples test clean help

# ── Default ───────────────────────────────────────────────────────────────────

all: help

# ══════════════════════════════════════════════════════════════════════════════
#  SELF-HOSTED PIPELINE  (two-stage bootstrap from prebuilt)
# ══════════════════════════════════════════════════════════════════════════════

BOOTSTRAP_STAMP := $(BUILD)/.bootstrap

# Stage 1 — prebuilt compiler → drast_v2 (bootstrap only once after clean)
$(BOOTSTRAP_STAMP): $(PREBUILT_COMPILER) $(DRAST_MAIN)
	@mkdir -p $(BUILD)/runtime
	@cp runtime/drast_runtime.hpp $(BUILD)/runtime/
	@$(PREBUILT_COMPILER) "$(DRAST_MAIN)" -o $(BUILD)/drast_v2.cpp
	@$(CXX) $(CXXFLAGS) $(BUILD)/drast_v2.cpp -o $(BUILD)/drast_v2
	@touch $(BOOTSTRAP_STAMP)
	@echo "Bootstrapped self-hosted compiler → $(BUILD)/drast_v2"

# Stage 2 — drast_v2 compiles drast_v3 (first self-hosted stage)
$(BUILD)/drast_v3: $(BOOTSTRAP_STAMP)
	@./$(BUILD)/drast_v2 "$(DRAST_MAIN)" -o $(BUILD)/drast_v3.cpp
	@$(CXX) $(CXXFLAGS) $(BUILD)/drast_v3.cpp -o $(BUILD)/drast_v3
	@echo "Built $(BUILD)/drast_v3"

## self-build  — run the full self-hosting pipeline (prebuilt → v2 → v3)
self-build: $(BUILD)/drast_v3
	@echo "Self-hosted compiler ready → $(BUILD)/drast_v3"

## run  — compile FILE with the prebuilt compiler and execute it
##        (default FILE=$(SRC_DIR)/main.drast, override: make run FILE=path/to/file.drast)
run: $(PREBUILT_COMPILER)
	@mkdir -p $(BUILD)/run
	@echo "── Compiling $(FILE) with prebuilt compiler ──"
	@$(PREBUILT_COMPILER) "$(FILE)" -o $(BUILD)/run/out.cpp
	@$(CXX) $(CXXFLAGS) $(BUILD)/run/out.cpp -o $(BUILD)/run/out
	@echo "── Running ──"
	@$(BUILD)/run/out

## self-run  — compile FILE with the self-hosted compiler and execute it
##             (override: make self-run FILE=path/to/file.drast)
self-run: $(BUILD)/drast_v3
	@mkdir -p $(BUILD)/self-run
	@echo "── Compiling $(FILE) with self-hosted compiler ──"
	@./$(BUILD)/drast_v3 "$(FILE)" -o $(BUILD)/self-run/out.cpp
	@$(CXX) $(CXXFLAGS) $(BUILD)/self-run/out.cpp -o $(BUILD)/self-run/out
	@echo "── Running ──"
	@$(BUILD)/self-run/out

## self-release  — highly optimized native release binary (darwin-arm64)
self-release: $(BUILD)/drast_v3
	@mkdir -p $(RELEASE_DIR)/runtime
	@cp runtime/drast_runtime.hpp $(RELEASE_DIR)/runtime/
	@echo "Building optimized release..."
	@./$(BUILD)/drast_v3 "$(DRAST_MAIN)" -o $(RELEASE_DIR)/drastc.cpp
	@$(CXX) $(CXXFLAGS) $(OPTFLAGS) $(RELEASE_DIR)/drastc.cpp -o $(RELEASE_DIR)/drastc
	@echo "Release binary → $(RELEASE_DIR)/drastc"

## self-examples  — compile and run all Examples/**/*.drast with the self-hosted compiler
self-examples: $(BUILD)/drast_v3
	@find Examples -name '*.drast' | sort | while IFS= read -r example; do \
		echo "=== $$example ==="; \
		./$(BUILD)/drast_v3 "$$example"; \
	done

# ══════════════════════════════════════════════════════════════════════════════
#  CROSS-PLATFORM DISTRIBUTION  (requires: brew install zig)
#
#  Transpiles main.drast → drastc.cpp once, then cross-compiles for 5 targets:
#    darwin-arm64   (your M1, native)
#    darwin-amd64   (Intel Mac, via -arch x86_64)
#    linux-amd64    (zig cc cross-compile)
#    linux-arm64    (zig cc cross-compile)
#    windows-amd64  (zig cc cross-compile → .exe)
# ══════════════════════════════════════════════════════════════════════════════

## dist  — build release binaries for all platforms into dist/
dist: $(BUILD)/drast_v3
	@mkdir -p $(DIST_DIR)
	@echo "Transpiling $(DRAST_MAIN) → $(DIST_DIR)/drastc.cpp"
	@./$(BUILD)/drast_v3 "$(DRAST_MAIN)" -o $(DIST_DIR)/drastc.cpp

	@echo "[1/5] darwin-arm64  (native clang)"
	@$(CXX) $(CXXFLAGS) $(OPTFLAGS) \
	    $(DIST_DIR)/drastc.cpp -o $(DIST_DIR)/drastc-darwin-arm64

	@echo "[2/5] darwin-amd64  (clang -arch x86_64)"
	@$(CXX) $(CXXFLAGS) $(OPTFLAGS) -arch x86_64 \
	    $(DIST_DIR)/drastc.cpp -o $(DIST_DIR)/drastc-darwin-amd64

	@echo "[3/5] linux-amd64   (zig c++ cross)"
	@$(ZIG) c++ $(ZIGFLAGS) -target x86_64-linux-gnu \
	    $(DIST_DIR)/drastc.cpp -o $(DIST_DIR)/drastc-linux-amd64

	@echo "[4/5] linux-arm64   (zig c++ cross)"
	@$(ZIG) c++ $(ZIGFLAGS) -target aarch64-linux-gnu \
	    $(DIST_DIR)/drastc.cpp -o $(DIST_DIR)/drastc-linux-arm64

	@echo "[5/5] windows-amd64 (zig c++ cross)"
	@$(ZIG) c++ $(ZIGFLAGS) -target x86_64-windows-gnu \
	    $(DIST_DIR)/drastc.cpp -o $(DIST_DIR)/drastc-windows-amd64.exe

	@echo ""
	@echo "Distribution binaries:"
	@ls -lh $(DIST_DIR)/drastc-* 2>/dev/null

# ══════════════════════════════════════════════════════════════════════════════
#  COMPAT ALIASES  (keep old targets working)
# ══════════════════════════════════════════════════════════════════════════════

release: self-release
run-examples: self-examples

# ══════════════════════════════════════════════════════════════════════════════
#  MISC
# ══════════════════════════════════════════════════════════════════════════════

## test  — quick smoke test using the prebuilt compiler
test: $(PREBUILT_COMPILER)
	@$(PREBUILT_COMPILER) Examples/Fundamentals/helloWorld.drast

## clean  — remove all build artifacts
clean:
	rm -rf $(BUILD)

## help  — list available targets
help:
	@echo ""
	@echo "  Drast Compiler — available targets"
	@echo "  ────────────────────────────────────────────────────────────────"
	@echo "  Prebuilt compiler"
	@echo "    make run [FILE=...]       compile+run FILE with prebuilt (default: main.drast)"
	@echo ""
	@echo "  Self-hosted pipeline"
	@echo "    make self-build           prebuilt → drast_v2 → drast_v3"
	@echo "    make self-run [FILE=...]  compile+run FILE with self-hosted compiler"
	@echo "    make self-release         optimized native release binary"
	@echo "    make self-examples        compile+run all Examples/ with self-hosted"
	@echo ""
	@echo "  Distribution  (requires: brew install zig)"
	@echo "    make dist                 cross-compile for all platforms → dist/"
	@echo ""
	@echo "  Misc"
	@echo "    make test                 smoke test (prebuilt + helloWorld.drast)"
	@echo "    make clean                remove build/"
	@echo "  ────────────────────────────────────────────────────────────────"
	@echo ""
