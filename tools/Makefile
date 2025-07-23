# Makefile for STLD/STAR Development Tools
# Builds utility tools for development and analysis

# Include main configuration
ROOT_DIR = ..
include $(ROOT_DIR)/config.mk

# Tool-specific configuration
TOOLS_DIR = .
TOOLS_BUILD_DIR = $(BUILD_DIR)/tools
TOOLS_BIN_DIR = $(TOOLS_BUILD_DIR)/bin
TOOLS_OBJ_DIR = $(TOOLS_BUILD_DIR)/obj

# Tool source files
SMOF_DUMP_SRC = smof_dump.c
STAR_LIST_SRC = star_list.c
SMOF_VALIDATOR_SRC = smof_validator.c
STAR_ANALYZER_SRC = star_analyzer.c

# Tool object files
SMOF_DUMP_OBJ = $(TOOLS_OBJ_DIR)/smof_dump.o
STAR_LIST_OBJ = $(TOOLS_OBJ_DIR)/star_list.o
SMOF_VALIDATOR_OBJ = $(TOOLS_OBJ_DIR)/smof_validator.o
STAR_ANALYZER_OBJ = $(TOOLS_OBJ_DIR)/star_analyzer.o

# Tool executables
SMOF_DUMP = $(TOOLS_BIN_DIR)/smof_dump
STAR_LIST = $(TOOLS_BIN_DIR)/star_list
SMOF_VALIDATOR = $(TOOLS_BIN_DIR)/smof_validator
STAR_ANALYZER = $(TOOLS_BIN_DIR)/star_analyzer

# All tools
ALL_TOOLS = $(SMOF_DUMP) $(STAR_LIST) $(SMOF_VALIDATOR) $(STAR_ANALYZER)

# Library dependencies
LIBCOMMON = $(BUILD_DIR)/libcommon.a
LIBSTLD = $(BUILD_DIR)/libstld.a
LIBSTAR = $(BUILD_DIR)/libstar.a

# Compiler flags for tools
TOOLS_CFLAGS = $(CFLAGS) -I$(SRC_DIR)/common/include -I$(SRC_DIR)/stld/include -I$(SRC_DIR)/star/include
TOOLS_LDFLAGS = $(LDFLAGS) -L$(BUILD_DIR)

# Default target
.PHONY: all
all: $(ALL_TOOLS) scripts

# Create build directories
$(TOOLS_BUILD_DIR) $(TOOLS_BIN_DIR) $(TOOLS_OBJ_DIR):
	@mkdir -p $@

# Build libraries first if they don't exist
$(LIBCOMMON):
	@echo "Building libcommon..."
	@$(MAKE) -C $(ROOT_DIR) libcommon

$(LIBSTLD): $(LIBCOMMON)
	@echo "Building libstld..."
	@$(MAKE) -C $(ROOT_DIR) libstld

$(LIBSTAR): $(LIBCOMMON)
	@echo "Building libstar..."
	@$(MAKE) -C $(ROOT_DIR) libstar

# Tool object files
$(TOOLS_OBJ_DIR)/%.o: $(TOOLS_DIR)/%.c | $(TOOLS_OBJ_DIR)
	@echo "Compiling $<..."
	$(CC) $(TOOLS_CFLAGS) -c -o $@ $<

# SMOF dump tool
$(SMOF_DUMP): $(SMOF_DUMP_OBJ) $(LIBCOMMON) | $(TOOLS_BIN_DIR)
	@echo "Linking smof_dump..."
	$(CC) $(TOOLS_LDFLAGS) -o $@ $< -lcommon

# STAR list tool
$(STAR_LIST): $(STAR_LIST_OBJ) $(LIBSTAR) $(LIBCOMMON) | $(TOOLS_BIN_DIR)
	@echo "Linking star_list..."
	$(CC) $(TOOLS_LDFLAGS) -o $@ $< -lstar -lcommon

# SMOF validator tool
$(SMOF_VALIDATOR): $(SMOF_VALIDATOR_OBJ) $(LIBCOMMON) | $(TOOLS_BIN_DIR)
	@echo "Linking smof_validator..."
	$(CC) $(TOOLS_LDFLAGS) -o $@ $< -lcommon

# STAR analyzer tool
$(STAR_ANALYZER): $(STAR_ANALYZER_OBJ) $(LIBSTAR) $(LIBCOMMON) | $(TOOLS_BIN_DIR)
	@echo "Linking star_analyzer..."
	$(CC) $(TOOLS_LDFLAGS) -o $@ $< -lstar -lcommon

# Scripts (already executable)
.PHONY: scripts
scripts: coverage.sh static_analysis.sh
	@echo "Development scripts ready"

# Individual tool targets
.PHONY: smof-dump star-list smof-validator star-analyzer
smof-dump: $(SMOF_DUMP)
star-list: $(STAR_LIST)
smof-validator: $(SMOF_VALIDATOR)
star-analyzer: $(STAR_ANALYZER)

# Tool testing targets
.PHONY: test-tools
test-tools: $(ALL_TOOLS)
	@echo "Testing development tools..."
	@echo "Testing smof_dump..."
	$(SMOF_DUMP) --version
	@echo "Testing star_list..."
	$(STAR_LIST) --version
	@echo "Testing smof_validator..."
	$(SMOF_VALIDATOR) --version
	@echo "Testing star_analyzer..."
	$(STAR_ANALYZER) --version
	@echo "All tools working correctly!"

# Coverage analysis
.PHONY: coverage
coverage: scripts
	@echo "Running coverage analysis..."
	./coverage.sh

# Static analysis
.PHONY: static-analysis
static-analysis: scripts
	@echo "Running static analysis..."
	./static_analysis.sh

# Install tools
.PHONY: install
install: $(ALL_TOOLS) scripts
	@echo "Installing development tools to $(PREFIX)/bin..."
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 $(SMOF_DUMP) $(DESTDIR)$(PREFIX)/bin/
	install -m 755 $(STAR_LIST) $(DESTDIR)$(PREFIX)/bin/
	install -m 755 $(SMOF_VALIDATOR) $(DESTDIR)$(PREFIX)/bin/
	install -m 755 $(STAR_ANALYZER) $(DESTDIR)$(PREFIX)/bin/
	install -m 755 coverage.sh $(DESTDIR)$(PREFIX)/bin/stld-coverage
	install -m 755 static_analysis.sh $(DESTDIR)$(PREFIX)/bin/stld-static-analysis

# Clean targets
.PHONY: clean
clean:
	rm -rf $(TOOLS_BUILD_DIR)

.PHONY: clean-scripts
clean-scripts:
	rm -rf analysis coverage

# Help target
.PHONY: help
help:
	@echo "STLD/STAR Development Tools Makefile"
	@echo "====================================="
	@echo ""
	@echo "Available targets:"
	@echo "  all              - Build all development tools"
	@echo "  smof-dump        - Build SMOF file dumper"
	@echo "  star-list        - Build STAR archive lister"
	@echo "  smof-validator   - Build SMOF file validator"
	@echo "  star-analyzer    - Build STAR archive analyzer"
	@echo "  scripts          - Prepare analysis scripts"
	@echo "  test-tools       - Test all tools"
	@echo "  coverage         - Run code coverage analysis"
	@echo "  static-analysis  - Run static code analysis"
	@echo "  install          - Install tools to system"
	@echo "  clean            - Clean build artifacts"
	@echo "  clean-scripts    - Clean analysis results"
	@echo "  help             - Show this help"
	@echo ""
	@echo "Tool usage examples:"
	@echo "  $(TOOLS_BIN_DIR)/smof_dump --help"
	@echo "  $(TOOLS_BIN_DIR)/star_list --help"
	@echo "  $(TOOLS_BIN_DIR)/smof_validator --help"
	@echo "  $(TOOLS_BIN_DIR)/star_analyzer --help"
	@echo ""
	@echo "Analysis scripts:"
	@echo "  ./coverage.sh --help"
	@echo "  ./static_analysis.sh --help"

# Dependencies
-include $(SMOF_DUMP_OBJ:.o=.d)
-include $(STAR_LIST_OBJ:.o=.d)
-include $(SMOF_VALIDATOR_OBJ:.o=.d)
-include $(STAR_ANALYZER_OBJ:.o=.d)
