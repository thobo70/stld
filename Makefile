# Main Makefile for STLD/STAR project
# C99 compliance and cross-platform support

# Include configuration
include config.mk
include Rules.mk

# Project information
PROJECT_NAME := stld-star
VERSION := 1.0.0
BUILD_DATE := $(shell date -u +"%Y-%m-%d %H:%M:%S UTC")

# Default target
.PHONY: all
all: build-info stld star tools

# Main targets
.PHONY: stld star tools tests test-all test-memory test-smof test-error test-integration clean install coverage docs

stld: $(BUILD_DIR)/stld_exe
star: $(BUILD_DIR)/star_exe
tools: $(BUILD_DIR)/smof_dump $(BUILD_DIR)/star_list

# Library targets
$(BUILD_DIR)/libcommon.a: $(COMMON_OBJS)
	@mkdir -p $(dir $@)
	$(call print_info,Creating common library)
	$(Q)$(AR) $(ARFLAGS) $@ $^
	$(Q)$(RANLIB) $@

$(BUILD_DIR)/libstld.a: $(STLD_OBJS) $(BUILD_DIR)/libcommon.a
	@mkdir -p $(dir $@)
	$(call print_info,Creating STLD library)
	$(Q)$(AR) $(ARFLAGS) $@ $(STLD_OBJS)
	$(Q)$(RANLIB) $@

$(BUILD_DIR)/libstar.a: $(STAR_OBJS) $(BUILD_DIR)/libcommon.a
	@mkdir -p $(dir $@)
	$(call print_info,Creating STAR library)
	$(Q)$(AR) $(ARFLAGS) $@ $(STAR_OBJS)
	$(Q)$(RANLIB) $@

# Executable targets
$(BUILD_DIR)/stld_exe: $(SRC_DIR)/stld/main.c $(BUILD_DIR)/libstld.a
	@mkdir -p $(dir $@)
	$(call print_info,Linking STLD executable)
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $< -L$(BUILD_DIR) -lstld -lcommon

$(BUILD_DIR)/star_exe: $(SRC_DIR)/star/main.c $(BUILD_DIR)/libstar.a
	@mkdir -p $(dir $@)
	$(call print_info,Linking STAR executable)
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $< -L$(BUILD_DIR) -lstar -lcommon

# Tool targets
$(BUILD_DIR)/smof_dump: $(TOOLS_DIR)/smof_dump.c $(BUILD_DIR)/libcommon.a
	@mkdir -p $(dir $@)
	$(call print_info,Building SMOF dump tool)
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $< -L$(BUILD_DIR) -lcommon

$(BUILD_DIR)/star_list: $(TOOLS_DIR)/star_list.c $(BUILD_DIR)/libstar.a
	@mkdir -p $(dir $@)
	$(call print_info,Building STAR list tool)
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $< -L$(BUILD_DIR) -lstar -lcommon

# Test targets
tests: $(BUILD_DIR)/test_runner
	$(call print_info,Running test suite)
	$(Q)$(BUILD_DIR)/test_runner

# Individual test executables
$(BUILD_DIR)/test_memory: $(BUILD_DIR)/tests/test_memory.o $(BUILD_DIR)/tests/unity/unity.o $(BUILD_DIR)/libcommon.a
	@mkdir -p $(dir $@)
	$(call print_info,Building memory test)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/test_smof: $(BUILD_DIR)/tests/test_smof.o $(BUILD_DIR)/tests/unity/unity.o $(BUILD_DIR)/libcommon.a
	@mkdir -p $(dir $@)
	$(call print_info,Building SMOF test)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/test_error: $(BUILD_DIR)/tests/test_error.o $(BUILD_DIR)/tests/unity/unity.o $(BUILD_DIR)/libcommon.a
	@mkdir -p $(dir $@)
	$(call print_info,Building error test)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/test_integration: $(BUILD_DIR)/tests/test_integration.o $(BUILD_DIR)/tests/unity/unity.o $(BUILD_DIR)/libstld.a $(BUILD_DIR)/libcommon.a
	@mkdir -p $(dir $@)
	$(call print_info,Building integration test)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(BUILD_DIR)/tests/unity/unity.o -L$(BUILD_DIR) -lstld -lcommon

# Run individual tests
test-memory: $(BUILD_DIR)/test_memory
	$(call print_info,Running memory tests)
	$(Q)$(BUILD_DIR)/test_memory

test-smof: $(BUILD_DIR)/test_smof
	$(call print_info,Running SMOF tests)
	$(Q)$(BUILD_DIR)/test_smof

test-error: $(BUILD_DIR)/test_error
	$(call print_info,Running error tests)
	$(Q)$(BUILD_DIR)/test_error

test-integration: $(BUILD_DIR)/test_integration
	$(call print_info,Running integration tests)
	$(Q)$(BUILD_DIR)/test_integration

# Run all individual tests
test-all: test-memory test-smof test-error test-integration
	$(call print_success,All individual tests passed!)

$(BUILD_DIR)/test_runner: $(TEST_OBJS) $(BUILD_DIR)/libstld.a $(BUILD_DIR)/libstar.a
	@mkdir -p $(dir $@)
	$(call print_info,Linking test runner)
	$(Q)$(CC) $(CFLAGS) $(TEST_LDFLAGS) -o $@ $(TEST_OBJS) \
		-L$(BUILD_DIR) -lstld -lstar -lcommon $(UNICORN_LIBS)

# Coverage analysis
coverage: CFLAGS += --coverage
coverage: LDFLAGS += --coverage
coverage: clean tests
	$(call print_info,Generating coverage report)
	@mkdir -p $(COVERAGE_DIR)
	$(Q)lcov --capture --directory $(BUILD_DIR) --output-file $(COVERAGE_DIR)/coverage.info
	$(Q)lcov --remove $(COVERAGE_DIR)/coverage.info '/usr/*' --output-file $(COVERAGE_DIR)/coverage.info
	$(Q)lcov --list $(COVERAGE_DIR)/coverage.info
	$(Q)genhtml $(COVERAGE_DIR)/coverage.info --output-directory $(COVERAGE_DIR)/html
	$(call print_success,Coverage report generated in $(COVERAGE_DIR)/html/index.html)

# Documentation
docs:
	$(call print_info,Generating documentation)
	@mkdir -p $(DOCS_DIR)/api
	$(Q)doxygen $(DOCS_DIR)/Doxyfile

# Static analysis
static-analysis:
	$(call print_info,Running static analysis)
	$(Q)$(TOOLS_DIR)/static_analysis.sh

# Installation
install: all
	$(call print_info,Installing to $(PREFIX))
	$(Q)install -d $(DESTDIR)$(PREFIX)/bin
	$(Q)install -d $(DESTDIR)$(PREFIX)/lib
	$(Q)install -d $(DESTDIR)$(PREFIX)/include/stld
	$(Q)install -d $(DESTDIR)$(PREFIX)/include/star
	$(Q)install -m 755 $(BUILD_DIR)/stld $(DESTDIR)$(PREFIX)/bin/
	$(Q)install -m 755 $(BUILD_DIR)/star $(DESTDIR)$(PREFIX)/bin/
	$(Q)install -m 755 $(BUILD_DIR)/smof_dump $(DESTDIR)$(PREFIX)/bin/
	$(Q)install -m 755 $(BUILD_DIR)/star_list $(DESTDIR)$(PREFIX)/bin/
	$(Q)install -m 644 $(BUILD_DIR)/libstld.a $(DESTDIR)$(PREFIX)/lib/
	$(Q)install -m 644 $(BUILD_DIR)/libstar.a $(DESTDIR)$(PREFIX)/lib/
	$(Q)install -m 644 $(BUILD_DIR)/libcommon.a $(DESTDIR)$(PREFIX)/lib/
	$(Q)cp -r $(SRC_DIR)/stld/include/* $(DESTDIR)$(PREFIX)/include/stld/
	$(Q)cp -r $(SRC_DIR)/star/include/* $(DESTDIR)$(PREFIX)/include/star/
	$(Q)cp -r $(SRC_DIR)/common/include/* $(DESTDIR)$(PREFIX)/include/

# Clean
clean:
	$(call print_info,Cleaning build artifacts)
	$(Q)rm -rf $(BUILD_DIR) $(COVERAGE_DIR)

distclean: clean
	$(call print_info,Cleaning all generated files)
	$(Q)rm -rf docs/html coverage.info
	$(call print_success,All generated files cleaned)

# Pattern rules for object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(call print_info,Compiling $(notdir $<))
	$(Q)$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BUILD_DIR)/tests/%.o: $(TESTS_DIR)/%.c
	@mkdir -p $(dir $@)
	$(call print_info,Compiling test $(notdir $<))
	$(Q)$(CC) $(CFLAGS) $(TEST_CPPFLAGS) -c -o $@ $<

# Dependencies
-include $(DEPS)

# C99 compliance check
check-c99:
	$(call print_info,Checking C99 compliance)
	$(Q)$(CC) $(CSTD) -fsyntax-only -pedantic -Werror $(SRC_DIR)/common/include/*.h
	$(Q)$(CC) $(CSTD) -fsyntax-only -pedantic -Werror $(SRC_DIR)/stld/include/*.h
	$(Q)$(CC) $(CSTD) -fsyntax-only -pedantic -Werror $(SRC_DIR)/star/include/*.h
	$(call print_success,C99 compliance check passed)

# Help target
help:
	@echo "Available targets:"
	@echo "  all           - Build all components"
	@echo "  stld          - Build STLD linker"
	@echo "  star          - Build STAR archiver"
	@echo "  tools         - Build utility tools"
	@echo "  tests         - Run test suite"
	@echo "  test-all      - Run all individual tests"
	@echo "  test-memory   - Run memory pool tests"
	@echo "  test-smof     - Run SMOF format tests"
	@echo "  test-error    - Run error handling tests"
	@echo "  test-integration - Run integration tests"
	@echo "  coverage      - Generate coverage report"
	@echo "  docs          - Generate documentation"
	@echo "  static-analysis - Run static analysis"
	@echo "  install       - Install to system"
	@echo "  clean         - Clean build artifacts"
	@echo "  check-c99     - Check C99 compliance"
	@echo "  help          - Show this help"

# Create build directories
.PHONY: directories
directories:
	@mkdir -p $(BUILD_DIR)/src/common
	@mkdir -p $(BUILD_DIR)/src/stld
	@mkdir -p $(BUILD_DIR)/src/star
	@mkdir -p $(BUILD_DIR)/tests
