# Makefile for STLD/STAR Unit Tests
# Builds and runs comprehensive unit test suite using Unity framework

# Project information
PROJECT_NAME = STLD-STAR
VERSION = 1.0.0

# Directory structure
ROOT_DIR = ../..
SRC_DIR = $(ROOT_DIR)/src
TESTS_DIR = .
UNITY_DIR = $(ROOT_DIR)/external/unity
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin
OBJ_DIR = $(BUILD_DIR)/obj

# Source directories
COMMON_SRC_DIR = $(SRC_DIR)/common
STLD_SRC_DIR = $(SRC_DIR)/stld
STAR_SRC_DIR = $(SRC_DIR)/star
UNITY_SRC_DIR = $(UNITY_DIR)/src

# Test directories
COMMON_TEST_DIR = $(TESTS_DIR)/common
STLD_TEST_DIR = $(TESTS_DIR)/stld
STAR_TEST_DIR = $(TESTS_DIR)/star

# Compiler and tools
CC = gcc
AR = ar
RANLIB = ranlib

# Compiler flags
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -Werror
CFLAGS += -g -O0 -fprofile-arcs -ftest-coverage
CFLAGS += -DUNITY_INCLUDE_CONFIG_H
CFLAGS += -DTEST_BUILD

# Include paths
INCLUDES = -I$(SRC_DIR)
INCLUDES += -I$(COMMON_SRC_DIR)
INCLUDES += -I$(STLD_SRC_DIR)
INCLUDES += -I$(STAR_SRC_DIR)
INCLUDES += -I$(UNITY_SRC_DIR)
INCLUDES += -I$(TESTS_DIR)

# Library flags
LDFLAGS = -lgcov -lm

# Source files
UNITY_SOURCES = $(UNITY_SRC_DIR)/unity.c

# Common module sources
COMMON_SOURCES = $(COMMON_SRC_DIR)/memory.c \
                $(COMMON_SRC_DIR)/smof.c \
                $(COMMON_SRC_DIR)/error.c

# STLD module sources
STLD_SOURCES = $(STLD_SRC_DIR)/symbol_table.c \
              $(STLD_SRC_DIR)/section.c \
              $(STLD_SRC_DIR)/relocation.c \
              $(STLD_SRC_DIR)/output.c

# STAR module sources
STAR_SOURCES = $(STAR_SRC_DIR)/archive.c \
              $(STAR_SRC_DIR)/compress.c \
              $(STAR_SRC_DIR)/index.c

# Test source files
COMMON_TEST_SOURCES = $(COMMON_TEST_DIR)/test_memory.c \
                     $(COMMON_TEST_DIR)/test_smof.c \
                     $(COMMON_TEST_DIR)/test_error.c

STLD_TEST_SOURCES = $(STLD_TEST_DIR)/test_symbol_table.c \
                   $(STLD_TEST_DIR)/test_section.c \
                   $(STLD_TEST_DIR)/test_relocation.c \
                   $(STLD_TEST_DIR)/test_output.c

STAR_TEST_SOURCES = $(STAR_TEST_DIR)/test_archive.c \
                   $(STAR_TEST_DIR)/test_compress.c \
                   $(STAR_TEST_DIR)/test_index.c

ALL_TEST_SOURCES = $(COMMON_TEST_SOURCES) $(STLD_TEST_SOURCES) $(STAR_TEST_SOURCES)

# Object files
UNITY_OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(UNITY_SOURCES:.c=.o)))
COMMON_OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(COMMON_SOURCES:.c=.o)))
STLD_OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(STLD_SOURCES:.c=.o)))
STAR_OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(STAR_SOURCES:.c=.o)))

COMMON_TEST_OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(COMMON_TEST_SOURCES:.c=.o)))
STLD_TEST_OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(STLD_TEST_SOURCES:.c=.o)))
STAR_TEST_OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(STAR_TEST_SOURCES:.c=.o)))

ALL_OBJECTS = $(UNITY_OBJECTS) $(COMMON_OBJECTS) $(STLD_OBJECTS) $(STAR_OBJECTS)
ALL_TEST_OBJECTS = $(COMMON_TEST_OBJECTS) $(STLD_TEST_OBJECTS) $(STAR_TEST_OBJECTS)

# Test executables
TEST_RUNNER = $(BIN_DIR)/test_runner
COMMON_TESTS = $(BIN_DIR)/test_memory $(BIN_DIR)/test_smof $(BIN_DIR)/test_error
STLD_TESTS = $(BIN_DIR)/test_symbol_table $(BIN_DIR)/test_section $(BIN_DIR)/test_relocation $(BIN_DIR)/test_output
STAR_TESTS = $(BIN_DIR)/test_archive $(BIN_DIR)/test_compress $(BIN_DIR)/test_index

ALL_TESTS = $(COMMON_TESTS) $(STLD_TESTS) $(STAR_TESTS)

# Default target
.PHONY: all
all: $(TEST_RUNNER) $(ALL_TESTS)

# Create build directories
$(BUILD_DIR) $(BIN_DIR) $(OBJ_DIR):
	@mkdir -p $@

# Unity framework compilation
$(OBJ_DIR)/unity.o: $(UNITY_SRC_DIR)/unity.c | $(OBJ_DIR)
	@echo "Compiling Unity framework..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Common module compilation
$(OBJ_DIR)/memory.o: $(COMMON_SRC_DIR)/memory.c | $(OBJ_DIR)
	@echo "Compiling memory.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/smof.o: $(COMMON_SRC_DIR)/smof.c | $(OBJ_DIR)
	@echo "Compiling smof.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/error.o: $(COMMON_SRC_DIR)/error.c | $(OBJ_DIR)
	@echo "Compiling error.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# STLD module compilation
$(OBJ_DIR)/symbol_table.o: $(STLD_SRC_DIR)/symbol_table.c | $(OBJ_DIR)
	@echo "Compiling symbol_table.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/section.o: $(STLD_SRC_DIR)/section.c | $(OBJ_DIR)
	@echo "Compiling section.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/relocation.o: $(STLD_SRC_DIR)/relocation.c | $(OBJ_DIR)
	@echo "Compiling relocation.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/output.o: $(STLD_SRC_DIR)/output.c | $(OBJ_DIR)
	@echo "Compiling output.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# STAR module compilation
$(OBJ_DIR)/archive.o: $(STAR_SRC_DIR)/archive.c | $(OBJ_DIR)
	@echo "Compiling archive.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/compress.o: $(STAR_SRC_DIR)/compress.c | $(OBJ_DIR)
	@echo "Compiling compress.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/index.o: $(STAR_SRC_DIR)/index.c | $(OBJ_DIR)
	@echo "Compiling index.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Test compilation rules
$(OBJ_DIR)/test_memory.o: $(COMMON_TEST_DIR)/test_memory.c | $(OBJ_DIR)
	@echo "Compiling test_memory.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test_smof.o: $(COMMON_TEST_DIR)/test_smof.c | $(OBJ_DIR)
	@echo "Compiling test_smof.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test_error.o: $(COMMON_TEST_DIR)/test_error.c | $(OBJ_DIR)
	@echo "Compiling test_error.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test_symbol_table.o: $(STLD_TEST_DIR)/test_symbol_table.c | $(OBJ_DIR)
	@echo "Compiling test_symbol_table.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test_section.o: $(STLD_TEST_DIR)/test_section.c | $(OBJ_DIR)
	@echo "Compiling test_section.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test_relocation.o: $(STLD_TEST_DIR)/test_relocation.c | $(OBJ_DIR)
	@echo "Compiling test_relocation.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test_output.o: $(STLD_TEST_DIR)/test_output.c | $(OBJ_DIR)
	@echo "Compiling test_output.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test_archive.o: $(STAR_TEST_DIR)/test_archive.c | $(OBJ_DIR)
	@echo "Compiling test_archive.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test_compress.o: $(STAR_TEST_DIR)/test_compress.c | $(OBJ_DIR)
	@echo "Compiling test_compress.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test_index.o: $(STAR_TEST_DIR)/test_index.c | $(OBJ_DIR)
	@echo "Compiling test_index.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/test_runner.o: $(TESTS_DIR)/test_runner.c | $(OBJ_DIR)
	@echo "Compiling test_runner.c..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Individual test executables
$(BIN_DIR)/test_memory: $(OBJ_DIR)/test_memory.o $(OBJ_DIR)/memory.o $(UNITY_OBJECTS) | $(BIN_DIR)
	@echo "Linking test_memory..."
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/test_smof: $(OBJ_DIR)/test_smof.o $(OBJ_DIR)/smof.o $(OBJ_DIR)/memory.o $(OBJ_DIR)/error.o $(UNITY_OBJECTS) | $(BIN_DIR)
	@echo "Linking test_smof..."
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/test_error: $(OBJ_DIR)/test_error.o $(OBJ_DIR)/error.o $(UNITY_OBJECTS) | $(BIN_DIR)
	@echo "Linking test_error..."
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/test_symbol_table: $(OBJ_DIR)/test_symbol_table.o $(OBJ_DIR)/symbol_table.o $(OBJ_DIR)/memory.o $(OBJ_DIR)/error.o $(UNITY_OBJECTS) | $(BIN_DIR)
	@echo "Linking test_symbol_table..."
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/test_section: $(OBJ_DIR)/test_section.o $(OBJ_DIR)/section.o $(OBJ_DIR)/memory.o $(OBJ_DIR)/error.o $(UNITY_OBJECTS) | $(BIN_DIR)
	@echo "Linking test_section..."
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/test_relocation: $(OBJ_DIR)/test_relocation.o $(OBJ_DIR)/relocation.o $(OBJ_DIR)/symbol_table.o $(OBJ_DIR)/section.o $(OBJ_DIR)/memory.o $(OBJ_DIR)/error.o $(UNITY_OBJECTS) | $(BIN_DIR)
	@echo "Linking test_relocation..."
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/test_output: $(OBJ_DIR)/test_output.o $(OBJ_DIR)/output.o $(OBJ_DIR)/section.o $(OBJ_DIR)/symbol_table.o $(OBJ_DIR)/memory.o $(OBJ_DIR)/error.o $(UNITY_OBJECTS) | $(BIN_DIR)
	@echo "Linking test_output..."
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/test_archive: $(OBJ_DIR)/test_archive.o $(OBJ_DIR)/archive.o $(OBJ_DIR)/compress.o $(OBJ_DIR)/memory.o $(OBJ_DIR)/error.o $(UNITY_OBJECTS) | $(BIN_DIR)
	@echo "Linking test_archive..."
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/test_compress: $(OBJ_DIR)/test_compress.o $(OBJ_DIR)/compress.o $(OBJ_DIR)/memory.o $(OBJ_DIR)/error.o $(UNITY_OBJECTS) | $(BIN_DIR)
	@echo "Linking test_compress..."
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN_DIR)/test_index: $(OBJ_DIR)/test_index.o $(OBJ_DIR)/index.o $(OBJ_DIR)/memory.o $(OBJ_DIR)/error.o $(UNITY_OBJECTS) | $(BIN_DIR)
	@echo "Linking test_index..."
	$(CC) $^ -o $@ $(LDFLAGS)

# Main test runner
$(TEST_RUNNER): $(OBJ_DIR)/test_runner.o $(ALL_OBJECTS) $(ALL_TEST_OBJECTS) | $(BIN_DIR)
	@echo "Linking test_runner..."
	$(CC) $^ -o $@ $(LDFLAGS)

# Test execution targets
.PHONY: test
test: $(TEST_RUNNER)
	@echo "=========================================="
	@echo "Running comprehensive test suite..."
	@echo "=========================================="
	./$(TEST_RUNNER)

.PHONY: test-memory
test-memory: $(BIN_DIR)/test_memory
	@echo "Running memory tests..."
	./$(BIN_DIR)/test_memory

.PHONY: test-smof
test-smof: $(BIN_DIR)/test_smof
	@echo "Running SMOF tests..."
	./$(BIN_DIR)/test_smof

.PHONY: test-error
test-error: $(BIN_DIR)/test_error
	@echo "Running error tests..."
	./$(BIN_DIR)/test_error

.PHONY: test-symbol
test-symbol: $(BIN_DIR)/test_symbol_table
	@echo "Running symbol table tests..."
	./$(BIN_DIR)/test_symbol_table

.PHONY: test-section
test-section: $(BIN_DIR)/test_section
	@echo "Running section tests..."
	./$(BIN_DIR)/test_section

.PHONY: test-relocation
test-relocation: $(BIN_DIR)/test_relocation
	@echo "Running relocation tests..."
	./$(BIN_DIR)/test_relocation

.PHONY: test-output
test-output: $(BIN_DIR)/test_output
	@echo "Running output tests..."
	./$(BIN_DIR)/test_output

.PHONY: test-archive
test-archive: $(BIN_DIR)/test_archive
	@echo "Running archive tests..."
	./$(BIN_DIR)/test_archive

.PHONY: test-compress
test-compress: $(BIN_DIR)/test_compress
	@echo "Running compression tests..."
	./$(BIN_DIR)/test_compress

.PHONY: test-index
test-index: $(BIN_DIR)/test_index
	@echo "Running index tests..."
	./$(BIN_DIR)/test_index

.PHONY: test-common
test-common: test-memory test-smof test-error
	@echo "All common module tests completed."

.PHONY: test-stld
test-stld: test-symbol test-section test-relocation test-output
	@echo "All STLD module tests completed."

.PHONY: test-star
test-star: test-archive test-compress test-index
	@echo "All STAR module tests completed."

# Code coverage
.PHONY: coverage
coverage: test
	@echo "Generating code coverage report..."
	@mkdir -p coverage
	gcov -o $(OBJ_DIR) $(COMMON_SOURCES) $(STLD_SOURCES) $(STAR_SOURCES)
	lcov --capture --directory $(OBJ_DIR) --output-file coverage/coverage.info
	genhtml coverage/coverage.info --output-directory coverage/html
	@echo "Coverage report generated in coverage/html/index.html"

# Valgrind memory checking
.PHONY: memcheck
memcheck: $(TEST_RUNNER)
	@echo "Running memory leak detection..."
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TEST_RUNNER)

# Static analysis
.PHONY: analyze
analyze:
	@echo "Running static analysis..."
	cppcheck --enable=all --inconclusive --std=c99 $(SRC_DIR) $(TESTS_DIR)

# Benchmark tests
.PHONY: benchmark
benchmark: $(TEST_RUNNER)
	@echo "Running performance benchmarks..."
	time ./$(TEST_RUNNER)

# Clean targets
.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)
	rm -f *.gcov *.gcda *.gcno

.PHONY: clean-coverage
clean-coverage:
	@echo "Cleaning coverage data..."
	rm -rf coverage
	rm -f *.gcov *.gcda *.gcno

.PHONY: distclean
distclean: clean clean-coverage
	@echo "Deep clean completed."

# Help target
.PHONY: help
help:
	@echo "STLD/STAR Unit Test Makefile"
	@echo "============================"
	@echo ""
	@echo "Available targets:"
	@echo "  all          - Build all tests (default)"
	@echo "  test         - Run comprehensive test suite"
	@echo "  test-<name>  - Run specific test group (memory, smof, error, etc.)"
	@echo "  test-common  - Run all common module tests"
	@echo "  test-stld    - Run all STLD linker tests"
	@echo "  test-star    - Run all STAR archiver tests"
	@echo "  coverage     - Generate code coverage report"
	@echo "  memcheck     - Run memory leak detection with Valgrind"
	@echo "  analyze      - Run static code analysis"
	@echo "  benchmark    - Run performance benchmarks"
	@echo "  clean        - Remove build artifacts"
	@echo "  clean-coverage - Remove coverage data"
	@echo "  distclean    - Deep clean all generated files"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make test                    # Run all tests"
	@echo "  make test-memory             # Run only memory tests"
	@echo "  make coverage                # Generate coverage report"
	@echo "  make TEST_RUNNER=\"memory smof\"  # Run specific test groups"

# Configuration info
.PHONY: info
info:
	@echo "Build Configuration:"
	@echo "==================="
	@echo "Project:        $(PROJECT_NAME) v$(VERSION)"
	@echo "Compiler:       $(CC)"
	@echo "C Standard:     C99"
	@echo "Build Type:     Debug with Coverage"
	@echo "Unity Version:  Latest"
	@echo "Source Dir:     $(SRC_DIR)"
	@echo "Build Dir:      $(BUILD_DIR)"
	@echo "Test Dir:       $(TESTS_DIR)"
	@echo ""
	@echo "Flags:"
	@echo "CFLAGS:         $(CFLAGS)"
	@echo "INCLUDES:       $(INCLUDES)"
	@echo "LDFLAGS:        $(LDFLAGS)"
