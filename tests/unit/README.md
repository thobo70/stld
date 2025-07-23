# STLD/STAR Unit Test Suite

## Overview

This directory contains a comprehensive unit test suite for the STLD linker and STAR archiver components. The test suite is built using the Unity testing framework and provides extensive coverage of all major functionality, error handling, and edge cases.

## Architecture

### Test Organization

The test suite is organized into three main categories:

- **Common Module Tests** (`tests/unit/common/`): Core functionality shared between STLD and STAR
- **STLD Module Tests** (`tests/unit/stld/`): Linker-specific functionality
- **STAR Module Tests** (`tests/unit/star/`): Archiver-specific functionality

### Test Files Structure

```
tests/unit/
├── common/
│   ├── test_memory.c       # Memory pool management tests
│   ├── test_smof.c         # SMOF format validation tests
│   └── test_error.c        # Error handling and context tests
├── stld/
│   ├── test_symbol_table.c # Symbol table management tests
│   ├── test_section.c      # Section management tests
│   ├── test_relocation.c   # Relocation processing tests
│   └── test_output.c       # Output generation tests
├── star/
│   ├── test_archive.c      # Archive management tests
│   ├── test_compress.c     # Compression engine tests
│   └── test_index.c        # File indexing tests
├── test_runner.c           # Main test runner with CLI
├── unity_config.h          # Unity framework configuration
├── Makefile               # Build system for tests
└── README.md              # This documentation
```

## Test Framework

### Unity Testing Framework

The test suite uses the Unity testing framework, which provides:

- C99-compliant unit testing
- Rich assertion macros
- Test fixture support (setUp/tearDown)
- Detailed test reporting
- Memory leak detection support
- Performance testing capabilities

### Custom Test Macros

Additional test macros are defined in `unity_config.h` for:

- Memory pool validation
- Error context checking
- SMOF format verification
- Performance benchmarking
- Endianness testing
- File system operations

## Building and Running Tests

### Prerequisites

- GCC compiler with C99 support
- Unity testing framework (included in external/unity)
- GNU Make
- Optional: Valgrind for memory leak detection
- Optional: LCOV/GCOV for code coverage

### Building Tests

```bash
# Build all tests
make all

# Build specific test groups
make test-memory    # Memory management tests
make test-smof      # SMOF format tests
make test-stld      # All STLD linker tests
make test-star      # All STAR archiver tests
```

### Running Tests

```bash
# Run comprehensive test suite
make test

# Run specific test groups
make test-memory
make test-symbol
make test-archive

# Run with the test runner directly
./build/bin/test_runner

# Run specific groups with test runner
./build/bin/test_runner memory smof error

# Get help for test runner options
./build/bin/test_runner --help
```

### Test Runner Options

The test runner supports various command-line options:

```bash
# Show help
./build/bin/test_runner --help

# List available test groups
./build/bin/test_runner --list

# Run specific test groups
./build/bin/test_runner memory smof

# Enable verbose output
./build/bin/test_runner --verbose memory
```

## Test Coverage

### Code Coverage Analysis

Generate code coverage reports:

```bash
# Generate coverage report
make coverage

# View coverage report
open coverage/html/index.html
```

### Memory Leak Detection

Run tests with Valgrind:

```bash
# Memory leak detection
make memcheck

# Run specific test with Valgrind
valgrind --leak-check=full ./build/bin/test_memory
```

### Static Analysis

Run static code analysis:

```bash
# Static analysis with cppcheck
make analyze
```

## Test Categories

### Common Module Tests

#### Memory Management (`test_memory.c`)
- Memory pool creation and destruction
- Allocation and deallocation
- Alignment verification
- Pool exhaustion handling
- Memory statistics tracking
- Fragmentation testing
- Reset and cleanup operations

#### SMOF Format (`test_smof.c`)
- Header validation
- Magic number verification
- Version compatibility
- Checksum calculation
- Section parsing
- Symbol table parsing
- Endianness handling
- Corruption detection

#### Error Handling (`test_error.c`)
- Error context management
- Error propagation
- Message formatting
- Severity levels
- Error chaining
- Callback mechanisms
- System error integration

### STLD Module Tests

#### Symbol Table (`test_symbol_table.c`)
- Symbol insertion and lookup
- Duplicate symbol handling
- Symbol resolution
- Weak symbol support
- Symbol iteration
- Export/import handling
- Symbol filtering
- Table serialization

#### Section Management (`test_section.c`)
- Section creation and destruction
- Data management
- Address assignment
- Alignment handling
- Section merging
- Layout calculation
- Memory mapping
- Section iteration

#### Relocation Processing (`test_relocation.c`)
- PC-relative relocations
- Absolute relocations
- Addend handling
- Overflow detection
- Unresolved symbols
- Cross-section references
- Relocation table management
- Multiple relocation types

#### Output Generation (`test_output.c`)
- Binary flat output
- Executable format generation
- Memory map creation
- Symbol table output
- Cross-reference generation
- Debug information
- Multiple output formats
- File I/O operations

### STAR Module Tests

#### Archive Management (`test_archive.c`)
- Archive creation and destruction
- File addition and removal
- Metadata management
- Archive iteration
- Compression integration
- Archive validation
- File extraction
- Archive serialization

#### Compression Engine (`test_compress.c`)
- Compression algorithm support
- DEFLATE compression
- GZIP format support
- Compression levels
- Buffer management
- Error handling
- Statistics tracking
- Multiple algorithms

#### File Indexing (`test_index.c`)
- Index creation and management
- Filename-based lookup
- Offset-based lookup
- Entry addition and removal
- Index iteration
- Sorting functionality
- Search capabilities
- Index serialization

## Writing New Tests

### Test File Template

```c
#include "unity.h"
#include "module_to_test.h"
#include "memory.h"
#include "error.h"

/* Test fixture data */
static module_t* test_module;
static memory_pool_t* test_memory_pool;
static error_context_t* test_error_ctx;

void setUp(void) {
    test_error_ctx = error_context_create();
    TEST_ASSERT_NOT_NULL(test_error_ctx);
    
    test_memory_pool = memory_pool_create(4096);
    TEST_ASSERT_NOT_NULL(test_memory_pool);
    
    test_module = module_create(test_memory_pool, test_error_ctx);
    TEST_ASSERT_NOT_NULL(test_module);
}

void tearDown(void) {
    if (test_module != NULL) {
        module_destroy(test_module);
        test_module = NULL;
    }
    
    if (test_memory_pool != NULL) {
        memory_pool_destroy(test_memory_pool);
        test_memory_pool = NULL;
    }
    
    if (test_error_ctx != NULL) {
        error_context_destroy(test_error_ctx);
        test_error_ctx = NULL;
    }
}

void test_basic_functionality(void) {
    /* Test implementation */
    TEST_ASSERT_TRUE(module_basic_operation(test_module));
    TEST_ASSERT_FALSE(error_context_has_error(test_error_ctx));
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_basic_functionality);
    
    return UNITY_END();
}
```

### Test Naming Conventions

- Test files: `test_<module>.c`
- Test functions: `test_<functionality_description>()`
- Setup/teardown: `setUp()` and `tearDown()`
- Test fixtures: `test_<module>` prefix

### Best Practices

1. **Test Isolation**: Each test should be independent and not rely on other tests
2. **Comprehensive Coverage**: Test normal operation, edge cases, and error conditions
3. **Clear Assertions**: Use descriptive assertion messages
4. **Memory Management**: Always test for memory leaks and proper cleanup
5. **Error Handling**: Verify error conditions and recovery
6. **Performance**: Include performance tests for critical paths
7. **Documentation**: Comment complex test scenarios

## Continuous Integration

### CI/CD Integration

The test suite is designed to integrate with CI/CD systems:

```bash
# CI-friendly test execution
make test 2>&1 | tee test_results.log

# Generate coverage for CI
make coverage

# XML output for CI systems
./build/bin/test_runner --xml-output test_results.xml
```

### Exit Codes

- `0`: All tests passed
- `1`: One or more tests failed
- `2`: Test execution error

## Performance Testing

### Benchmarking

Run performance benchmarks:

```bash
# Time test execution
make benchmark

# Performance-specific tests
./build/bin/test_runner --performance
```

### Memory Usage

Monitor memory usage during tests:

```bash
# Memory usage monitoring
valgrind --tool=massif ./build/bin/test_runner
```

## Troubleshooting

### Common Issues

1. **Compilation Errors**: Check include paths and dependencies
2. **Test Failures**: Review assertion messages and debug output
3. **Memory Leaks**: Use Valgrind to identify leak sources
4. **Performance Issues**: Profile with appropriate tools

### Debug Mode

Enable debug output:

```bash
# Compile with debug symbols
make CFLAGS="-g -DDEBUG_TESTS"

# Run with GDB
gdb ./build/bin/test_runner
```

### Verbose Output

Enable verbose test output:

```bash
# Verbose Unity output
make test CFLAGS="-DUNITY_VERBOSE"
```

## Contributing

When adding new tests:

1. Follow the existing structure and naming conventions
2. Include comprehensive test coverage
3. Add memory leak detection
4. Update this documentation
5. Ensure all tests pass before committing

## Test Results Format

The test suite provides detailed output including:

- Test execution summary
- Individual test results
- Memory usage statistics
- Performance metrics
- Error details for failed tests
- Code coverage information

Example output:
```
╔══════════════════════════════════════════════════════════════╗
║                    STLD/STAR Test Suite                     ║
║                                                              ║
║  Comprehensive unit tests for STLD linker and STAR archiver ║
║  Version 1.0.0 - Built with Unity Testing Framework        ║
╚══════════════════════════════════════════════════════════════╝

┌─ Running Memory Tests ──────────────────────────────────────┐
│ Memory pool management and allocation tests                  │
└─────────────────────────────────────────────────────────────┘
✓ Memory tests completed successfully

╔══════════════════════════════════════════════════════════════╗
║                        Test Summary                          ║
╠══════════════════════════════════════════════════════════════╣
║ Total Tests:       127                                       ║
║ Passed:            127                                       ║
║ Failed:              0                                       ║
║ Ignored:             0                                       ║
║ Elapsed Time:     3.45 seconds                              ║
╠══════════════════════════════════════════════════════════════╣
║                    🎉 ALL TESTS PASSED! 🎉                  ║
╚══════════════════════════════════════════════════════════════╝
```
