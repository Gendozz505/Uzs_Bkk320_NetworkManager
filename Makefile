# Compiler and flags for host build
HOST_CXX := g++
HOST_CXXFLAGS := -g -O0 -std=c++17 -pthread -Iinclude -DSPDLOG_LEVEL=\"trace\"
HOST_LDLIBS := -lboost_system -lboost_program_options -lpthread

# Compiler and flags for target (ARM64) build
TARGET_CXX := arm-linux-gnueabihf-g++
TARGET_CXXFLAGS := -O2 -std=c++17 --sysroot ~/Git-projects/sysrootU20 -Wno-psabi -Wformat -Iinclude -DSPDLOG_LEVEL=\"info\"
TARGET_LDFLAGS := -static
TARGET_LDLIBS := -lboost_system -lboost_program_options -lpthread

# Output directories for build artifacts
HOST_BUILD_DIR := build/host
TARGET_BUILD_DIR := build/target
HOST_OBJ_DIR := $(HOST_BUILD_DIR)/obj
TARGET_OBJ_DIR := $(TARGET_BUILD_DIR)/obj

# Source files
SRC := $(shell find . -type f -name '*.cpp' -not -path './build/*' -printf '%P\n')
VPATH := .:src

# Object files for host and target
HOST_OBJ := $(patsubst %.cpp,$(HOST_OBJ_DIR)/%.o,$(notdir $(SRC)))
TARGET_OBJ := $(patsubst %.cpp,$(TARGET_OBJ_DIR)/%.o,$(notdir $(SRC)))

# Final binaries
HOST_BIN := $(HOST_BUILD_DIR)/uzsBkk320NetworkManager
TARGET_BIN := $(TARGET_BUILD_DIR)/uzsBkk320NetworkManager

# Default target
all: host

# Host build
host: $(HOST_BIN)
$(HOST_BIN): $(HOST_OBJ)
	@mkdir -p $(dir $@)
	$(HOST_CXX) $(HOST_CXXFLAGS) -o $@ $^ $(HOST_LDLIBS)

$(HOST_OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(HOST_CXX) $(HOST_CXXFLAGS) -c $< -o $@

# Target (ARM64) build with static linking
target: $(TARGET_BIN)
$(TARGET_BIN): $(TARGET_OBJ)
	@mkdir -p $(dir $@)
	$(TARGET_CXX) $(TARGET_CXXFLAGS) $(TARGET_LDFLAGS) -o $@ $^ $(TARGET_LDLIBS)

$(TARGET_OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(TARGET_CXX) $(TARGET_CXXFLAGS) -c $< -o $@

# Run the host binary
run: $(HOST_BIN)
	./$(HOST_BIN) -p 30720

# Clean both host and target build directories
clean:
	rm -rf $(HOST_BUILD_DIR) $(TARGET_BUILD_DIR)

.PHONY: all host target clean run