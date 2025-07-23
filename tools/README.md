# STLD/STAR Development Tools

This directory contains development and analysis tools for the STLD (StarTek Linker/Debugger) and STAR (StarTek Archive) project.

## Overview

The development tools provide comprehensive analysis, validation, and debugging capabilities for SMOF (StarTek Module Object Format) files and STAR archives.

## Tools

### 1. SMOF Dump Tool (`smof_dump`)

A comprehensive SMOF file analyzer and dumper.

**Features:**
- Complete SMOF file structure analysis
- Section content dumping with hex and disassembly views
- Symbol table analysis
- Relocation information display
- Header validation
- Multiple output formats (text, JSON, XML)

**Usage:**
```bash
smof_dump [options] <smof_file>

Options:
  -h, --help          Show help message
  -v, --version       Show version information
  -V, --verbose       Enable verbose output
  -q, --quiet         Suppress non-error output
  -s, --sections      Dump section contents
  -S, --symbols       Display symbol table
  -r, --relocations   Show relocation information
  -H, --header-only   Display header information only
  -x, --hex-dump      Show hex dump of sections
  -d, --disasm        Attempt disassembly (if supported)
  -j, --json          Output in JSON format
  -X, --xml           Output in XML format
  -o, --output FILE   Write output to file
```

**Examples:**
```bash
# Basic file analysis
smof_dump program.smof

# Detailed analysis with all sections
smof_dump -s -S -r program.smof

# JSON output for automation
smof_dump -j -o analysis.json program.smof

# Quick header check
smof_dump -H program.smof
```

### 2. STAR List Tool (`star_list`)

Enhanced STAR archive listing and analysis tool.

**Features:**
- Archive member listing with details
- Symbol index display
- Compression analysis
- Integrity checking
- Size and timestamp information
- Multiple output formats

**Usage:**
```bash
star_list [options] <star_archive>

Options:
  -h, --help          Show help message
  -v, --version       Show version information
  -V, --verbose       Enable verbose output
  -q, --quiet         Suppress non-error output
  -l, --long          Long listing format
  -t, --table         Display as table
  -s, --symbols       Show symbol index
  -c, --compression   Show compression details
  -i, --integrity     Check archive integrity
  -j, --json          Output in JSON format
  -X, --xml           Output in XML format
  -o, --output FILE   Write output to file
```

**Examples:**
```bash
# Basic archive listing
star_list library.star

# Detailed listing with symbols
star_list -l -s library.star

# Compression analysis
star_list -c library.star

# JSON output for processing
star_list -j -o listing.json library.star
```

### 3. SMOF Validator (`smof_validator`)

Comprehensive SMOF file validation and compliance checking tool.

**Features:**
- Multiple validation levels (basic, standard, strict, pedantic)
- Header validation
- Section integrity checking
- Symbol table validation
- Relocation validation
- Checksum verification
- Error reporting and fixing suggestions
- JSON output for CI/CD integration

**Usage:**
```bash
smof_validator [options] <smof_files...>

Options:
  -h, --help          Show help message
  -v, --version       Show version information
  -V, --verbose       Enable verbose output
  -q, --quiet         Suppress non-error output
  -l, --level LEVEL   Validation level (basic|standard|strict|pedantic)
  -f, --fix           Attempt to fix errors
  -w, --warnings      Show warnings
  -e, --errors-only   Show only errors
  -j, --json          Output in JSON format
  -X, --xml           Output in XML format
  -o, --output FILE   Write output to file
  -r, --recursive     Process directories recursively
  -x, --exclude PATTERN Exclude files matching pattern
```

**Validation Levels:**
- **Basic**: Essential header and format checks
- **Standard**: Comprehensive validation (default)
- **Strict**: Stricter compliance checking
- **Pedantic**: Maximum validation with style checks

**Examples:**
```bash
# Standard validation
smof_validator program.smof

# Strict validation with warnings
smof_validator -l strict -w program.smof

# Batch validation with JSON output
smof_validator -j -o validation.json *.smof

# Recursive validation with error fixing
smof_validator -r -f src/
```

### 4. STAR Analyzer (`star_analyzer`)

Advanced STAR archive analysis and optimization tool.

**Features:**
- Comprehensive archive statistics
- Compression efficiency analysis
- Member dependency analysis
- Optimization recommendations
- Performance metrics
- Archive integrity checking
- Multiple analysis modes

**Usage:**
```bash
star_analyzer [options] <star_archives...>

Options:
  -h, --help          Show help message
  -v, --version       Show version information
  -V, --verbose       Enable verbose output
  -q, --quiet         Suppress non-error output
  -m, --mode MODE     Analysis mode (basic|detailed|compression|integrity|optimization)
  -s, --stats         Show detailed statistics
  -c, --compression   Analyze compression efficiency
  -d, --dependencies  Show member dependencies
  -o, --optimize      Show optimization recommendations
  -b, --benchmark     Run performance benchmarks
  -j, --json          Output in JSON format
  -X, --xml           Output in XML format
  -O, --output FILE   Write output to file
  -r, --recursive     Process directories recursively
```

**Analysis Modes:**
- **Basic**: Essential archive information
- **Detailed**: Comprehensive analysis (default)
- **Compression**: Focus on compression analysis
- **Integrity**: Thorough integrity checking
- **Optimization**: Performance optimization analysis

**Examples:**
```bash
# Basic archive analysis
star_analyzer library.star

# Detailed analysis with optimization
star_analyzer -m detailed -o library.star

# Compression efficiency analysis
star_analyzer -m compression -c library.star

# Batch analysis with JSON output
star_analyzer -j -O analysis.json *.star
```

## Analysis Scripts

### 1. Coverage Analysis (`coverage.sh`)

Automated code coverage analysis using lcov/gcov.

**Features:**
- Automated build with coverage flags
- Comprehensive test execution
- HTML and XML report generation
- Coverage threshold checking
- CI/CD integration support

**Usage:**
```bash
./coverage.sh [options]

Options:
  -h, --help          Show help message
  -d, --directory DIR Source directory (default: ../src)
  -o, --output DIR    Output directory (default: coverage)
  -t, --threshold NUM Minimum coverage threshold (default: 80)
  -f, --format FORMAT Output format (html|xml|both) (default: html)
  -c, --clean         Clean previous results
  -v, --verbose       Enable verbose output
  -q, --quiet         Suppress output
  --ci                CI mode (machine-readable output)
```

**Examples:**
```bash
# Standard coverage analysis
./coverage.sh

# Coverage with 90% threshold
./coverage.sh -t 90

# CI mode with XML output
./coverage.sh --ci -f xml

# Clean run with verbose output
./coverage.sh -c -v
```

### 2. Static Analysis (`static_analysis.sh`)

Multi-tool static analysis automation.

**Features:**
- Multiple analyzer support (cppcheck, clang-analyzer, etc.)
- Automated tool detection
- Comprehensive reporting
- CI/CD integration
- Summary generation

**Usage:**
```bash
./static_analysis.sh [options]

Options:
  -h, --help          Show help message
  -d, --directory DIR Source directory (default: ../src)
  -o, --output DIR    Output directory (default: analysis)
  -t, --tools TOOLS   Analysis tools (comma-separated)
  -f, --format FORMAT Output format (text|xml|json) (default: text)
  -s, --severity LEVEL Minimum severity (info|warning|error) (default: warning)
  -c, --clean         Clean previous results
  -v, --verbose       Enable verbose output
  -q, --quiet         Suppress output
  --ci                CI mode (machine-readable output)
```

**Supported Tools:**
- **cppcheck**: C/C++ static analyzer
- **clang-analyzer**: Clang static analyzer
- **scan-build**: Clang scan-build wrapper
- **flawfinder**: Security-focused analyzer
- **rats**: Rough Auditing Tool for Security

**Examples:**
```bash
# Run all available analyzers
./static_analysis.sh

# Run specific tools
./static_analysis.sh -t "cppcheck,clang-analyzer"

# CI mode with JSON output
./static_analysis.sh --ci -f json

# Error-level issues only
./static_analysis.sh -s error
```

## Building

### Prerequisites

- C99-compliant compiler (gcc, clang)
- Standard C library
- POSIX-compliant system
- Make build system

### Optional Dependencies

For analysis scripts:
- lcov/gcov (for coverage analysis)
- cppcheck (for static analysis)
- clang-tools (for clang-analyzer)

### Building All Tools

```bash
# From tools directory
make all

# Or from project root
make tools
```

### Building Individual Tools

```bash
make smof-dump
make star-list
make smof-validator
make star-analyzer
```

### Testing Tools

```bash
make test-tools
```

## Installation

### System Installation

```bash
# Install to /usr/local/bin (default)
make install

# Install to custom prefix
make install PREFIX=/opt/stld
```

### Portable Installation

Tools can be run directly from the build directory:
```bash
build/tools/bin/smof_dump --help
```

## Integration

### CI/CD Integration

All tools support JSON output for easy integration with CI/CD pipelines:

```bash
# Validation in CI
smof_validator -j -o validation.json src/*.smof
if [ $? -ne 0 ]; then
    echo "SMOF validation failed"
    exit 1
fi

# Coverage checking
./coverage.sh --ci -t 85 -f xml
```

### Build System Integration

Tools can be integrated into the main build system:

```makefile
# In main Makefile
check: tools
	$(TOOLS_BIN_DIR)/smof_validator $(BUILD_DIR)/*.smof
	$(TOOLS_BIN_DIR)/star_analyzer $(BUILD_DIR)/*.star
```

## Development

### Adding New Tools

1. Create source file in `tools/` directory
2. Add build rules to `tools/Makefile`
3. Follow existing code style and error handling patterns
4. Include comprehensive help and version information
5. Support JSON output for automation

### Code Style

- Follow C99 standard
- Use consistent error handling
- Include comprehensive help text
- Support multiple output formats
- Implement proper command-line parsing

## Troubleshooting

### Common Issues

1. **Tool not found**: Ensure tools are built and in PATH
2. **Permission denied**: Check file permissions and executable bits
3. **Library not found**: Ensure libraries are built before tools
4. **Invalid SMOF/STAR file**: Use validator tools to check file integrity

### Debug Options

All tools support verbose mode (`-V, --verbose`) for detailed debugging information.

### Support

For issues and questions:
- Check tool help: `tool_name --help`
- Run with verbose output: `tool_name -V`
- Validate input files first
- Check project documentation
