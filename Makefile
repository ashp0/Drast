CXX      := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wno-unused-parameter -g -O0 -MMD -MP
SRC_DIR  := src
BUILD    := build
SRCS     := $(SRC_DIR)/Token.cpp $(SRC_DIR)/Lexer.cpp $(SRC_DIR)/Parser.cpp \
            $(SRC_DIR)/Codegen.cpp $(SRC_DIR)/main.cpp
OBJS     := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD)/%.o,$(SRCS))
DEPS     := $(OBJS:.o=.d)
BIN      := $(BUILD)/drastc

.PHONY: all clean test

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILD)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

-include $(DEPS)

clean:
	rm -rf $(BUILD) drast_v2.cpp drast_v2

.PHONY: run-examples
run-examples: $(BIN)
	@mkdir -p $(BUILD)/runtime
	@cp runtime/drast_runtime.hpp $(BUILD)/runtime/
	@$(BIN) src/drast/main.drast -o $(BUILD)/drast_v2.cpp
	@$(CXX) $(CXXFLAGS) $(BUILD)/drast_v2.cpp -o $(BUILD)/drast_v2
	@find Examples -name '*.drast' | sort | while IFS= read -r example; do \
		echo "=== $$example ==="; \
		./$(BUILD)/drast_v2 "$$example"; \
	done

test: $(BIN)
	@$(BIN) Examples/Fundamentals/helloWorld.drast
