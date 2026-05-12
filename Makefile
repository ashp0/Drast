CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wno-unused-parameter -g -O0 -MMD -MP -I.
OPTFLAGS := -O3 -DNDEBUG -flto

BOOTSTRAP_DIR := legacy/cpp_bootstrap
SRC_DIR       := src
BUILD         := build
RELEASE_DIR   := $(BUILD)/release

# Bootstrap compiler (C++ implementation)
BOOTSTRAP_SRCS := $(BOOTSTRAP_DIR)/Token.cpp $(BOOTSTRAP_DIR)/Lexer.cpp \
                  $(BOOTSTRAP_DIR)/Parser.cpp $(BOOTSTRAP_DIR)/Codegen.cpp \
                  $(BOOTSTRAP_DIR)/main.cpp
BOOTSTRAP_OBJS := $(patsubst $(BOOTSTRAP_DIR)/%.cpp,$(BUILD)/bootstrap/%.o,$(BOOTSTRAP_SRCS))
BOOTSTRAP_BIN  := $(BUILD)/drastc_bootstrap

# Drast compiler source
DRAST_MAIN := $(SRC_DIR)/main.drast

.PHONY: all clean test release run-examples

all: $(BOOTSTRAP_BIN)

# 1. Build the C++ bootstrap compiler
$(BOOTSTRAP_BIN): $(BOOTSTRAP_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD)/bootstrap/%.o: $(BOOTSTRAP_DIR)/%.cpp
	@mkdir -p $(BUILD)/bootstrap
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include $(BUILD)/bootstrap/*.d

# 2. Build drast_v2 (compiled by the C++ bootstrap)
$(BUILD)/drast_v2: $(BOOTSTRAP_BIN)
	@mkdir -p $(BUILD)/runtime
	@cp runtime/drast_runtime.hpp $(BUILD)/runtime/
	@$(BOOTSTRAP_BIN) "$(DRAST_MAIN)" -o $(BUILD)/drast_v2.cpp
	@$(CXX) $(CXXFLAGS) $(BUILD)/drast_v2.cpp -o $(BUILD)/drast_v2
	@echo "Built $(BUILD)/drast_v2"

# 3. Build drast_v3 (compiled by drast_v2 - first self-hosted stage)
$(BUILD)/drast_v3: $(BUILD)/drast_v2
	@./$(BUILD)/drast_v2 "$(DRAST_MAIN)" -o $(BUILD)/drast_v3.cpp
	@$(CXX) $(CXXFLAGS) $(BUILD)/drast_v3.cpp -o $(BUILD)/drast_v3
	@echo "Built $(BUILD)/drast_v3"

# 4. Release build (highly optimized version of drast_v3)
release: $(BUILD)/drast_v3
	@mkdir -p $(RELEASE_DIR)/runtime
	@cp runtime/drast_runtime.hpp $(RELEASE_DIR)/runtime/
	@echo "Building highly optimized release version..."
	@./$(BUILD)/drast_v3 "$(DRAST_MAIN)" -o $(RELEASE_DIR)/drastc.cpp
	@$(CXX) $(CXXFLAGS) $(OPTFLAGS) $(RELEASE_DIR)/drastc.cpp -o $(RELEASE_DIR)/drastc
	@echo "Success! Release binary is at $(RELEASE_DIR)/drastc"

run-examples: $(BUILD)/drast_v3
	@find Examples -name '*.drast' | sort | while IFS= read -r example; do \
		echo "=== $$example ==="; \
		./$(BUILD)/drast_v3 "$$example"; \
	done

test: $(BOOTSTRAP_BIN)
	@$(BOOTSTRAP_BIN) Examples/Fundamentals/helloWorld.drast

clean:
	rm -rf $(BUILD)
