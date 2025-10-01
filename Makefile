CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Wpedantic -pthread -Iinclude -DSPDLOG_LEVEL=\"debug\"
LDFLAGS :=
LDLIBS := -lboost_system -lboost_program_options -lpthread

# Output directory for all build artifacts
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj

SRC := $(shell find . -type f -name '*.cpp' -not -path './build/*' -printf '%P\n')
VPATH := .:src

# Place all object files into a flat obj directory under build
OBJ := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(notdir $(SRC)))

# Final binary placed in the build directory
BIN := $(BUILD_DIR)/uzsBkk320NetworkManager

all: $(BIN)

$(BIN): $(OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)


$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean run

run: $(BIN)
	./$(BIN) -p 30720

clean:
	rm -rf $(BUILD_DIR)


