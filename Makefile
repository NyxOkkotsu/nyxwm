# ==============================================================================
# 1. COMPILER & FLAGS CONFIGURATION
# ==============================================================================
NAME     := nyxwm

SRC_DIR  := src
OBJ_DIR  := build
INC_DIR  := include
BIN_DIR  := $(OBJ_DIR)/bin
TARGET   := $(BIN_DIR)/$(NAME)

CC       := gcc
CFLAGS   := -Wall -Wextra
LDFLAGS  := -lX11 -lXi
CPPFLAGS := -I$(INC_DIR) -MMD -MP

# ==============================================================================
# 2. DYNAMIC FILE DETECTION
# ==============================================================================
SRCS     := $(wildcard $(SRC_DIR)/*.c)
OBJS     := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS     := $(OBJS:.o=.d)

# ==============================================================================
# 3. BUILD RULES
# ==============================================================================
.PHONY: all build run clean test reset purge

all: build

build: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "Linking executable: $@"
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@echo "Compiling: $< -> $@"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@ 

$(OBJ_DIR) $(BIN_DIR):
	@mkdir -p $@

run: $(TARGET)
	@echo ""
	@echo "-------------------------[test]--------------------------------"
	@echo ""
	@./$(TARGET)
	@echo ""
	@echo "---------------------------------------------------------------"
	@echo ""

clean:
	@echo "Cleaning up..."
	@rm -rf $(OBJ_DIR)

test: run clean

reset:
	@echo "Resetting the project..."
	@rm -rf $(SRC_DIR)/*
	@rm -rf $(INC_DIR)/*
	@rm -rf $(OBJ_DIR)

# For arch makepkg -si helper. 
purge: clean 
	@echo "Cleaning makepkg residues..."
	@rm -rf pkg 
	@rm -f *.tar.zst 

# ==============================================================================
# 4. HEADER DEPENDENCY INCLUSION
# ==============================================================================
-include $(DEPS)
