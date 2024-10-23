# Compiler and flags
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Og -g3 -Isrc
LDFLAGS = -mwindows -mconsole # Link with the Win32 subsystem

# Folder paths
SRC_DIR = src
BUILD_DIR = build
BIN_DIR = bin

# Project name
TARGET = $(BIN_DIR)/sand.exe

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
# Object files
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

# Default target
all: $(TARGET)

# Rule to create the target binary
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Rule to compile each source file into an object file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 
run: $(TARGET)
	$(TARGET)

# Clean up build and bin directories
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

# Phony targets to avoid conflicts with files named 'all' or 'clean'
.PHONY: all clean