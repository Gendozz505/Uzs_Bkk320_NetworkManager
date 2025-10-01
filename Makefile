CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic -pthread -Iinclude -DSPDLOG_LEVEL=\"debug\"
LDFLAGS :=
LDLIBS := -lboost_system -lpthread

# Output directory for all build artifacts
BUILD_DIR := build

SRC := $(shell find . -type f -name '*.cpp' -not -path './build/*' -printf '%P\n')

# Place object files in the build directory, mirroring source tree
OBJ := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRC))

# Final binary placed in the build directory
BIN := $(BUILD_DIR)/network_manager

all: $(BIN)

$(BIN): $(OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean run

run: $(BIN)
	./$(BIN) 30720

clean:
	rm -rf $(BUILD_DIR)


