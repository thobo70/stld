# Makefile for STLD/STAR project
# C99 compliant build system

# Project configuration
PROJECT_NAME := stld-star
VERSION := 1.0.0

# Directories
SRC_DIR := src
BUILD_DIR := build
TEST_DIR := tests
DOC_DIR := docs

# Compiler settings
CC := gcc
CFLAGS := -std=c99 -Wall -Wextra -Wpedantic -O2
CFLAGS_DEBUG := -std=c99 -Wall -Wextra -Wpedantic -g -DDEBUG
CFLAGS_COVERAGE := $(CFLAGS_DEBUG) --coverage

# Standard compliance
C_STANDARD := c99
POSIX_FLAGS := -D_POSIX_C_SOURCE=200112L -D_GNU_SOURCE

# Include directories
INCLUDES := -Isrc/common/include -Isrc/stld/include -Isrc/star/include

# Libraries
LIBS := -lm

# Source files (will be populated when actual source is created)
COMMON_SRCS := $(wildcard src/common/*.c)
STLD_SRCS := $(wildcard src/stld/*.c)
STAR_SRCS := $(wildcard src/star/*.c)
TEST_SRCS := $(wildcard tests/*.c)

# Object files
COMMON_OBJS := $(COMMON_SRCS:src/%.c=$(BUILD_DIR)/%.o)
STLD_OBJS := $(STLD_SRCS:src/%.c=$(BUILD_DIR)/%.o)
STAR_OBJS := $(STAR_SRCS:src/%.c=$(BUILD_DIR)/%.o)
TEST_OBJS := $(TEST_SRCS:tests/%.c=$(BUILD_DIR)/tests/%.o)

# Binaries
STLD_BIN := $(BUILD_DIR)/stld
STAR_BIN := $(BUILD_DIR)/star
TEST_BIN := $(BUILD_DIR)/test_runner

# Default target
.PHONY: all
all: directories $(STLD_BIN) $(STAR_BIN)

# Create build directories
.PHONY: directories
directories:
	@mkdir -p $(BUILD_DIR)/src/common
	@mkdir -p $(BUILD_DIR)/src/stld
	@mkdir -p $(BUILD_DIR)/src/star
	@mkdir -p $(BUILD_DIR)/tests

# STLD linker binary
$(STLD_BIN): $(COMMON_OBJS) $(STLD_OBJS)
	@echo "Linking STLD..."
	@$(CC) $(CFLAGS) $(POSIX_FLAGS) $^ -o $@ $(LIBS)

# STAR archiver binary
$(STAR_BIN): $(COMMON_OBJS) $(STAR_OBJS)
	@echo "Linking STAR..."
	@$(CC) $(CFLAGS) $(POSIX_FLAGS) $^ -o $@ $(LIBS)

# Test runner
$(TEST_BIN): $(COMMON_OBJS) $(TEST_OBJS)
	@echo "Linking test runner..."
	@$(CC) $(CFLAGS_DEBUG) $(POSIX_FLAGS) $^ -o $@ $(LIBS)

# Compile source files
$(BUILD_DIR)/src/%.o: src/%.c
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) $(POSIX_FLAGS) $(INCLUDES) -c $< -o $@

# Compile test files
$(BUILD_DIR)/tests/%.o: tests/%.c
	@echo "Compiling test $<..."
	@$(CC) $(CFLAGS_DEBUG) $(POSIX_FLAGS) $(INCLUDES) -c $< -o $@

# Development targets
.PHONY: debug
debug: CFLAGS = $(CFLAGS_DEBUG)
debug: all

.PHONY: release
release: CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -O3 -DNDEBUG
release: all

# Testing targets
.PHONY: tests
tests: $(TEST_BIN)
	@echo "Running tests..."
	@./$(TEST_BIN)

.PHONY: check
check: tests

# Code quality targets
.PHONY: check-c99
check-c99:
	@echo "Checking C99 compliance..."
	@if [ -n "$(COMMON_SRCS)$(STLD_SRCS)$(STAR_SRCS)" ]; then \
		for file in $(COMMON_SRCS) $(STLD_SRCS) $(STAR_SRCS); do \
			echo "Checking $$file..."; \
			$(CC) -std=c99 -fsyntax-only -Wall -Wextra -Wpedantic $(POSIX_FLAGS) $(INCLUDES) "$$file" || exit 1; \
		done; \
		echo "✓ All source files are C99 compliant"; \
	else \
		echo "⚠ No source files found to check"; \
	fi

.PHONY: static-analysis
static-analysis:
	@echo "Running static analysis..."
	@if command -v cppcheck >/dev/null 2>&1; then \
		if [ -n "$(COMMON_SRCS)$(STLD_SRCS)$(STAR_SRCS)" ]; then \
			cppcheck --std=c99 --enable=all --inconclusive --suppress=missingIncludeSystem $(INCLUDES) $(COMMON_SRCS) $(STLD_SRCS) $(STAR_SRCS); \
		else \
			echo "⚠ No source files found for static analysis"; \
		fi \
	else \
		echo "⚠ cppcheck not found, skipping static analysis"; \
	fi

.PHONY: format
format:
	@echo "Formatting code..."
	@if command -v clang-format >/dev/null 2>&1; then \
		find src tests -name "*.c" -o -name "*.h" | xargs clang-format -i -style="{BasedOnStyle: LLVM, IndentWidth: 4, ColumnLimit: 80}"; \
		echo "✓ Code formatted"; \
	else \
		echo "⚠ clang-format not found, skipping formatting"; \
	fi

# Coverage targets
.PHONY: coverage
coverage: CFLAGS = $(CFLAGS_COVERAGE)
coverage: clean tests
	@echo "Generating coverage report..."
	@if command -v lcov >/dev/null 2>&1; then \
		lcov --capture --directory $(BUILD_DIR) --output-file coverage.info; \
		lcov --remove coverage.info '/usr/*' --output-file coverage.info; \
		lcov --remove coverage.info '*/tests/*' --output-file coverage.info; \
		genhtml coverage.info --output-directory coverage; \
		echo "✓ Coverage report generated in coverage/index.html"; \
	else \
		echo "⚠ lcov not found, skipping coverage report"; \
	fi

# Documentation targets
.PHONY: docs
docs:
	@echo "Generating documentation..."
	@if command -v doxygen >/dev/null 2>&1; then \
		doxygen Doxyfile 2>/dev/null || echo "⚠ Doxyfile not found, skipping documentation"; \
	else \
		echo "⚠ doxygen not found, skipping documentation"; \
	fi

# Install targets
.PHONY: install
install: $(STLD_BIN) $(STAR_BIN)
	@echo "Installing binaries..."
	@install -d $(DESTDIR)/usr/local/bin
	@install -m 755 $(STLD_BIN) $(DESTDIR)/usr/local/bin/
	@install -m 755 $(STAR_BIN) $(DESTDIR)/usr/local/bin/
	@echo "✓ Installed to $(DESTDIR)/usr/local/bin/"

.PHONY: uninstall
uninstall:
	@echo "Uninstalling binaries..."
	@rm -f $(DESTDIR)/usr/local/bin/stld
	@rm -f $(DESTDIR)/usr/local/bin/star
	@echo "✓ Uninstalled"

# Clean targets
.PHONY: clean
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR)
	@rm -f coverage.info
	@rm -rf coverage
	@echo "✓ Build directory cleaned"

.PHONY: distclean
distclean: clean
	@echo "Cleaning all generated files..."
	@rm -rf docs/html
	@echo "✓ All generated files cleaned"

# Help target
.PHONY: help
help:
	@echo "STLD/STAR Build System"
	@echo ""
	@echo "Available targets:"
	@echo "  all           - Build STLD and STAR binaries (default)"
	@echo "  debug         - Build with debug symbols"
	@echo "  release       - Build optimized release version"
	@echo "  tests         - Build and run tests"
	@echo "  check         - Alias for tests"
	@echo "  check-c99     - Check C99 compliance"
	@echo "  static-analysis - Run static analysis with cppcheck"
	@echo "  format        - Format code with clang-format"
	@echo "  coverage      - Generate code coverage report"
	@echo "  docs          - Generate documentation with doxygen"
	@echo "  install       - Install binaries to system"
	@echo "  uninstall     - Remove installed binaries"
	@echo "  clean         - Clean build files"
	@echo "  distclean     - Clean all generated files"
	@echo "  help          - Show this help message"
	@echo ""
	@echo "Variables:"
	@echo "  CC            - Compiler (default: gcc)"
	@echo "  DESTDIR       - Installation prefix"
	@echo ""

# Dependency generation
-include $(BUILD_DIR)/src/common/*.d
-include $(BUILD_DIR)/src/stld/*.d
-include $(BUILD_DIR)/src/star/*.d
-include $(BUILD_DIR)/tests/*.d

# Generate dependency files
$(BUILD_DIR)/%.d: %.c
	@$(CC) $(CFLAGS) $(POSIX_FLAGS) $(INCLUDES) -MM -MT $(@:.d=.o) $< > $@
