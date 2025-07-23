# STLD/STAR Project Implementation Guide

## Executive Summary

This document describes the complete implementation strategy for STLD (STIX Linker) and STAR (STIX Archiver) using C99 standard and Makefile-based build system. The project includes comprehensive testing using Unity framework and Unicorn engine for CPU emulation, code coverage analysis, and adherence to embedded systems best practices.

## Technology Stack

### Core Requirements
- **Language**: C99 (ISO/IEC 9899:1999) compliance
- **Build System**: GNU Make with dependency tracking
- **Testing Framework**: Unity for unit/integration testing
- **Emulation**: Unicorn Engine for execution testing
- **Coverage**: gcov/lcov for code coverage analysis
- **Documentation**: Doxygen for API documentation
- **Static Analysis**: cppcheck, clang-static-analyzer

### Compiler Support
- **Primary**: GCC 4.9+ with C99 strict mode
- **Secondary**: Clang 3.8+ with C99 compliance
- **Embedded**: Cross-compilation support for ARM, RISC-V, x86

## Project Structure

```
stld/
├── Makefile                    # Main build configuration
├── config.mk                  # Build configuration variables
├── Rules.mk                   # Common build rules
├── README.md                  # Project overview
├── LICENSE                    # Project license
├── CHANGELOG.md               # Version history
│
├── src/                       # Source code
│   ├── common/                # Shared utilities
│   │   ├── include/           # Common headers
│   │   │   ├── smof.h         # SMOF format definitions
│   │   │   ├── memory.h       # Memory management
│   │   │   ├── error.h        # Error handling
│   │   │   └── utils.h        # Utility functions
│   │   ├── smof.c             # SMOF format implementation
│   │   ├── memory.c           # Memory pool allocator
│   │   ├── error.c            # Error handling
│   │   └── utils.c            # Utility functions
│   │
│   ├── stld/                  # STLD linker
│   │   ├── include/           # STLD headers
│   │   │   ├── stld.h         # Main API
│   │   │   ├── symbol_table.h # Symbol management
│   │   │   ├── section.h      # Section handling
│   │   │   ├── relocation.h   # Relocation processing
│   │   │   └── output.h       # Output generation
│   │   ├── main.c             # Command line interface
│   │   ├── symbol_table.c     # Symbol table implementation
│   │   ├── section.c          # Section management
│   │   ├── relocation.c       # Relocation engine
│   │   ├── output.c           # Output file generation
│   │   └── linker.c           # Main linking logic
│   │
│   └── star/                  # STAR archiver
│       ├── include/           # STAR headers
│       │   ├── star.h         # Main API
│       │   ├── archive.h      # Archive format
│       │   ├── compress.h     # Compression engine
│       │   └── index.h        # File indexing
│       ├── main.c             # Command line interface
│       ├── archive.c          # Archive management
│       ├── compress.c         # Compression implementation
│       ├── index.c            # File indexing
│       └── archiver.c         # Main archiver logic
│
├── tests/                     # Test suite
│   ├── Makefile              # Test build configuration
│   ├── unity/                # Unity testing framework
│   │   ├── unity.c           # Unity implementation
│   │   ├── unity.h           # Unity header
│   │   └── unity_internals.h # Unity internals
│   ├── fixtures/             # Test data files
│   │   ├── simple.smof       # Basic SMOF test file
│   │   ├── complex.smof      # Complex SMOF test file
│   │   └── library.star      # Test archive file
│   ├── unit/                 # Unit tests
│   │   ├── common/           # Common module tests
│   │   │   ├── test_memory.c # Memory pool tests
│   │   │   ├── test_smof.c   # SMOF format tests
│   │   │   └── test_utils.c  # Utility tests
│   │   ├── stld/             # STLD tests
│   │   │   ├── test_symbol_table.c
│   │   │   ├── test_section.c
│   │   │   ├── test_relocation.c
│   │   │   └── test_output.c
│   │   └── star/             # STAR tests
│   │       ├── test_archive.c
│   │       ├── test_compress.c
│   │       └── test_index.c
│   ├── integration/          # Integration tests
│   │   ├── test_full_linking.c
│   │   ├── test_archive_creation.c
│   │   └── test_cross_tool.c
│   ├── emulation/            # Unicorn engine tests
│   │   ├── test_execution.c  # Execution validation
│   │   ├── emulator.c        # Emulation helpers
│   │   └── emulator.h        # Emulation interface
│   └── performance/          # Performance benchmarks
│       ├── benchmark_linking.c
│       ├── benchmark_archiving.c
│       └── memory_profiling.c
│
├── tools/                    # Development tools
│   ├── smof_dump.c          # SMOF file analyzer
│   ├── star_list.c          # Archive listing tool
│   ├── coverage.sh          # Coverage analysis script
│   └── static_analysis.sh   # Static analysis script
│
├── docs/                    # Documentation
│   ├── Doxyfile             # Doxygen configuration
│   ├── api/                 # Generated API docs
│   ├── user_guide.md        # User documentation
│   └── developer_guide.md   # Developer documentation
│
└── examples/                # Usage examples
    ├── simple_program/      # Basic linking example
    ├── shared_library/      # Shared library example
    └── embedded_system/     # Embedded target example
```

## Build System Design

### Main Makefile

```makefile
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
all: stld star tools

# Main targets
.PHONY: stld star tools tests clean install coverage docs

stld: $(BUILD_DIR)/stld
star: $(BUILD_DIR)/star
tools: $(BUILD_DIR)/smof_dump $(BUILD_DIR)/star_list

# Library targets
$(BUILD_DIR)/libcommon.a: $(COMMON_OBJS)
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

$(BUILD_DIR)/libstld.a: $(STLD_OBJS) $(BUILD_DIR)/libcommon.a
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $(STLD_OBJS)
	$(RANLIB) $@

$(BUILD_DIR)/libstar.a: $(STAR_OBJS) $(BUILD_DIR)/libcommon.a
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $(STAR_OBJS)
	$(RANLIB) $@

# Executable targets
$(BUILD_DIR)/stld: $(SRC_DIR)/stld/main.c $(BUILD_DIR)/libstld.a
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< -L$(BUILD_DIR) -lstld -lcommon

$(BUILD_DIR)/star: $(SRC_DIR)/star/main.c $(BUILD_DIR)/libstar.a
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< -L$(BUILD_DIR) -lstar -lcommon

# Tool targets
$(BUILD_DIR)/smof_dump: $(TOOLS_DIR)/smof_dump.c $(BUILD_DIR)/libcommon.a
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< -L$(BUILD_DIR) -lcommon

$(BUILD_DIR)/star_list: $(TOOLS_DIR)/star_list.c $(BUILD_DIR)/libstar.a
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< -L$(BUILD_DIR) -lstar -lcommon

# Test targets
tests: $(BUILD_DIR)/test_runner
	@echo "Running test suite..."
	$(BUILD_DIR)/test_runner

$(BUILD_DIR)/test_runner: $(TEST_OBJS) $(BUILD_DIR)/libstld.a $(BUILD_DIR)/libstar.a
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(TEST_LDFLAGS) -o $@ $(TEST_OBJS) \
		-L$(BUILD_DIR) -lstld -lstar -lcommon $(UNICORN_LIBS)

# Coverage analysis
coverage: CFLAGS += --coverage
coverage: LDFLAGS += --coverage
coverage: clean tests
	@echo "Generating coverage report..."
	@mkdir -p $(COVERAGE_DIR)
	lcov --capture --directory $(BUILD_DIR) --output-file $(COVERAGE_DIR)/coverage.info
	lcov --remove $(COVERAGE_DIR)/coverage.info '/usr/*' --output-file $(COVERAGE_DIR)/coverage.info
	lcov --list $(COVERAGE_DIR)/coverage.info
	genhtml $(COVERAGE_DIR)/coverage.info --output-directory $(COVERAGE_DIR)/html
	@echo "Coverage report generated in $(COVERAGE_DIR)/html/index.html"

# Documentation
docs:
	@echo "Generating documentation..."
	@mkdir -p $(DOCS_DIR)/api
	doxygen $(DOCS_DIR)/Doxyfile

# Static analysis
static-analysis:
	@echo "Running static analysis..."
	$(TOOLS_DIR)/static_analysis.sh

# Installation
install: all
	@echo "Installing to $(PREFIX)..."
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/lib
	install -d $(DESTDIR)$(PREFIX)/include/stld
	install -d $(DESTDIR)$(PREFIX)/include/star
	install -m 755 $(BUILD_DIR)/stld $(DESTDIR)$(PREFIX)/bin/
	install -m 755 $(BUILD_DIR)/star $(DESTDIR)$(PREFIX)/bin/
	install -m 755 $(BUILD_DIR)/smof_dump $(DESTDIR)$(PREFIX)/bin/
	install -m 755 $(BUILD_DIR)/star_list $(DESTDIR)$(PREFIX)/bin/
	install -m 644 $(BUILD_DIR)/libstld.a $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(BUILD_DIR)/libstar.a $(DESTDIR)$(PREFIX)/lib/
	install -m 644 $(BUILD_DIR)/libcommon.a $(DESTDIR)$(PREFIX)/lib/
	cp -r $(SRC_DIR)/stld/include/* $(DESTDIR)$(PREFIX)/include/stld/
	cp -r $(SRC_DIR)/star/include/* $(DESTDIR)$(PREFIX)/include/star/
	cp -r $(SRC_DIR)/common/include/* $(DESTDIR)$(PREFIX)/include/

# Clean
clean:
	rm -rf $(BUILD_DIR) $(COVERAGE_DIR)

# Pattern rules for object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

$(BUILD_DIR)/tests/%.o: $(TESTS_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(TEST_CPPFLAGS) -c -o $@ $<

# Dependencies
-include $(DEPS)

# Help target
help:
	@echo "Available targets:"
	@echo "  all           - Build all components"
	@echo "  stld          - Build STLD linker"
	@echo "  star          - Build STAR archiver"
	@echo "  tools         - Build utility tools"
	@echo "  tests         - Run test suite"
	@echo "  coverage      - Generate coverage report"
	@echo "  docs          - Generate documentation"
	@echo "  static-analysis - Run static analysis"
	@echo "  install       - Install to system"
	@echo "  clean         - Clean build artifacts"
	@echo "  help          - Show this help"
```

### Configuration (config.mk)

```makefile
# Build configuration for STLD/STAR project

# Directories
SRC_DIR := src
BUILD_DIR := build
TESTS_DIR := tests
TOOLS_DIR := tools
DOCS_DIR := docs
COVERAGE_DIR := coverage

# Installation paths
PREFIX ?= /usr/local
DESTDIR ?=

# Compiler settings
CC := gcc
AR := ar
RANLIB := ranlib
ARFLAGS := rcs

# C99 compliance flags
CSTD := -std=c99
CWARNINGS := -Wall -Wextra -Wpedantic -Wstrict-prototypes \
             -Wmissing-prototypes -Wold-style-definition \
             -Wdeclaration-after-statement -Wshadow \
             -Wpointer-arith -Wcast-qual -Wwrite-strings \
             -Wmissing-declarations -Wredundant-decls \
             -Wnested-externs -Winline -Wno-long-long \
             -Wuninitialized -Wconversion -Wstrict-aliasing \
             -Wformat=2 -Wformat-security

# Feature test macros for C99
CFEATURES := -D_POSIX_C_SOURCE=200112L -D_DEFAULT_SOURCE

# Debug/Release configuration
DEBUG ?= 1
ifeq ($(DEBUG),1)
    COPTIM := -O0 -g3 -DDEBUG
    BUILD_TYPE := debug
else
    COPTIM := -O2 -DNDEBUG
    BUILD_TYPE := release
endif

# Base compiler flags
CFLAGS := $(CSTD) $(CWARNINGS) $(CFEATURES) $(COPTIM) -fPIC

# Include paths
CPPFLAGS := -I$(SRC_DIR)/common/include \
            -I$(SRC_DIR)/stld/include \
            -I$(SRC_DIR)/star/include \
            -DPROJECT_VERSION=\"$(VERSION)\" \
            -DBUILD_DATE=\"$(BUILD_DATE)\"

# Linker flags
LDFLAGS := -Wl,--as-needed -Wl,--no-undefined

# Test-specific flags
TEST_CPPFLAGS := $(CPPFLAGS) -I$(TESTS_DIR)/unity
TEST_LDFLAGS := $(LDFLAGS)

# Unicorn engine configuration
UNICORN_CFLAGS := $(shell pkg-config --cflags unicorn || echo "-I/usr/include/unicorn")
UNICORN_LIBS := $(shell pkg-config --libs unicorn || echo "-lunicorn")

# Coverage flags (added when coverage target is used)
COVERAGE_CFLAGS := --coverage -fprofile-arcs -ftest-coverage
COVERAGE_LDFLAGS := --coverage

# Source files
COMMON_SOURCES := $(wildcard $(SRC_DIR)/common/*.c)
STLD_SOURCES := $(wildcard $(SRC_DIR)/stld/*.c)
STAR_SOURCES := $(wildcard $(SRC_DIR)/star/*.c)
TEST_SOURCES := $(wildcard $(TESTS_DIR)/unit/*/*.c) \
                $(wildcard $(TESTS_DIR)/integration/*.c) \
                $(wildcard $(TESTS_DIR)/emulation/*.c) \
                $(wildcard $(TESTS_DIR)/performance/*.c) \
                $(TESTS_DIR)/unity/unity.c

# Object files
COMMON_OBJS := $(COMMON_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
STLD_OBJS := $(filter-out %/main.c,$(STLD_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o))
STAR_OBJS := $(filter-out %/main.c,$(STAR_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o))
TEST_OBJS := $(TEST_SOURCES:$(TESTS_DIR)/%.c=$(BUILD_DIR)/tests/%.o)

# Dependency files
DEPS := $(COMMON_OBJS:.o=.d) $(STLD_OBJS:.o=.d) $(STAR_OBJS:.o=.d) $(TEST_OBJS:.o=.d)

# Cross-compilation support
ifdef CROSS_COMPILE
    CC := $(CROSS_COMPILE)gcc
    AR := $(CROSS_COMPILE)ar
    RANLIB := $(CROSS_COMPILE)ranlib
endif

# Target-specific optimizations
ifdef TARGET_ARCH
    CFLAGS += -march=$(TARGET_ARCH)
endif

ifdef TARGET_CPU
    CFLAGS += -mcpu=$(TARGET_CPU)
endif

# Memory debugging (Valgrind, AddressSanitizer)
ifdef MEMORY_DEBUG
    CFLAGS += -fsanitize=address -fsanitize=undefined
    LDFLAGS += -fsanitize=address -fsanitize=undefined
endif
```

### Common Rules (Rules.mk)

```makefile
# Common build rules and utilities

# Automatic dependency generation
%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

# Colored output
NO_COLOR := \033[0m
RED := \033[0;31m
GREEN := \033[0;32m
YELLOW := \033[0;33m
BLUE := \033[0;34m

define print_info
	@printf "$(BLUE)[INFO]$(NO_COLOR) %s\n" "$(1)"
endef

define print_success
	@printf "$(GREEN)[SUCCESS]$(NO_COLOR) %s\n" "$(1)"
endef

define print_warning
	@printf "$(YELLOW)[WARNING]$(NO_COLOR) %s\n" "$(1)"
endef

define print_error
	@printf "$(RED)[ERROR]$(NO_COLOR) %s\n" "$(1)"
endef

# Build information
build-info:
	$(call print_info,Building $(PROJECT_NAME) v$(VERSION))
	$(call print_info,Build type: $(BUILD_TYPE))
	$(call print_info,Compiler: $(CC))
	$(call print_info,Flags: $(CFLAGS))

# Verbose output control
ifeq ($(V),1)
    Q :=
else
    Q := @
endif

# Create build directories
$(shell mkdir -p $(BUILD_DIR)/src/common $(BUILD_DIR)/src/stld $(BUILD_DIR)/src/star)
$(shell mkdir -p $(BUILD_DIR)/tests/unit/common $(BUILD_DIR)/tests/unit/stld $(BUILD_DIR)/tests/unit/star)
$(shell mkdir -p $(BUILD_DIR)/tests/integration $(BUILD_DIR)/tests/emulation $(BUILD_DIR)/tests/performance)
$(shell mkdir -p $(BUILD_DIR)/tests/unity)
```

## C99 Compliance Strategy

### Header Guard Standard

```c
/* src/common/include/smof.h */
#ifndef SMOF_H_INCLUDED
#define SMOF_H_INCLUDED

/* C99 standard headers */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SMOF format definitions with C99 compliance */
#define SMOF_MAGIC 0x534D4F46U  /* 'SMOF' */
#define SMOF_VERSION 1U

/* C99 exact-width integer types */
typedef struct smof_header {
    uint32_t magic;           /* File magic number */
    uint16_t version;         /* Format version */
    uint16_t flags;           /* File flags */
    uint32_t entry_point;     /* Entry point address */
    uint16_t section_count;   /* Number of sections */
    uint16_t symbol_count;    /* Number of symbols */
    uint32_t section_table_offset; /* Section table offset */
    uint32_t symbol_table_offset;  /* Symbol table offset */
    uint32_t string_table_offset;  /* String table offset */
    uint32_t checksum;        /* Header checksum */
} smof_header_t;

/* C99 static assertions for structure sizes */
_Static_assert(sizeof(smof_header_t) == 32, "SMOF header must be 32 bytes");

/* Function prototypes with C99 inline support */
static inline bool smof_header_is_valid(const smof_header_t* header) {
    return header != NULL && header->magic == SMOF_MAGIC;
}

/* C99 designated initializers support */
extern const smof_header_t smof_default_header;

#ifdef __cplusplus
}
#endif

#endif /* SMOF_H_INCLUDED */
```

### Memory Management with C99

```c
/* src/common/memory.c - C99 compliant memory management */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "memory.h"

/* C99 flexible array member */
typedef struct memory_pool {
    size_t size;
    size_t used;
    size_t alignment;
    uint8_t data[];  /* C99 flexible array member */
} memory_pool_t;

/* C99 compound literals and designated initializers */
memory_pool_t* memory_pool_create(size_t size) {
    assert(size > 0);
    
    memory_pool_t* pool = malloc(sizeof(memory_pool_t) + size);
    if (pool == NULL) {
        return NULL;
    }
    
    /* C99 designated initializer style */
    *pool = (memory_pool_t) {
        .size = size,
        .used = 0,
        .alignment = sizeof(void*)
    };
    
    return pool;
}

/* C99 inline function definitions */
void* memory_pool_alloc(memory_pool_t* pool, size_t size) {
    assert(pool != NULL);
    assert(size > 0);
    
    /* Align size to pool alignment */
    size_t aligned_size = (size + pool->alignment - 1) & ~(pool->alignment - 1);
    
    if (pool->used + aligned_size > pool->size) {
        return NULL;  /* Out of memory */
    }
    
    void* ptr = pool->data + pool->used;
    pool->used += aligned_size;
    
    return ptr;
}

/* C99 restrict keyword for optimization */
void memory_pool_copy(memory_pool_t* restrict dst, 
                     const memory_pool_t* restrict src, 
                     size_t size) {
    assert(dst != NULL);
    assert(src != NULL);
    assert(size <= dst->size);
    assert(size <= src->size);
    
    memcpy(dst->data, src->data, size);
    dst->used = size;
}
```

## Testing Framework Integration

### Unity Framework Setup

```c
/* tests/unit/common/test_memory.c - Unity test example */
#include "unity.h"
#include "memory.h"

/* Test fixture setup/teardown */
static memory_pool_t* test_pool;

void setUp(void) {
    test_pool = memory_pool_create(1024);
    TEST_ASSERT_NOT_NULL(test_pool);
}

void tearDown(void) {
    if (test_pool != NULL) {
        memory_pool_destroy(test_pool);
        test_pool = NULL;
    }
}

/* C99 designated initializers in tests */
void test_memory_pool_allocation(void) {
    void* ptr1 = memory_pool_alloc(test_pool, 64);
    void* ptr2 = memory_pool_alloc(test_pool, 128);
    
    TEST_ASSERT_NOT_NULL(ptr1);
    TEST_ASSERT_NOT_NULL(ptr2);
    TEST_ASSERT_NOT_EQUAL(ptr1, ptr2);
    
    /* Verify alignment */
    TEST_ASSERT_EQUAL_INT(0, ((uintptr_t)ptr1) % sizeof(void*));
    TEST_ASSERT_EQUAL_INT(0, ((uintptr_t)ptr2) % sizeof(void*));
}

void test_memory_pool_exhaustion(void) {
    /* Allocate until exhaustion */
    void* ptrs[16];
    size_t count = 0;
    
    for (size_t i = 0; i < 16; i++) {
        ptrs[i] = memory_pool_alloc(test_pool, 100);
        if (ptrs[i] != NULL) {
            count++;
        } else {
            break;
        }
    }
    
    TEST_ASSERT_GREATER_THAN(0, count);
    TEST_ASSERT_LESS_THAN(16, count); /* Should exhaust before 16 allocations */
    
    /* Next allocation should fail */
    void* overflow = memory_pool_alloc(test_pool, 100);
    TEST_ASSERT_NULL(overflow);
}

void test_memory_pool_reset(void) {
    /* Allocate some memory */
    void* ptr = memory_pool_alloc(test_pool, 256);
    TEST_ASSERT_NOT_NULL(ptr);
    
    size_t used_before = memory_pool_get_used(test_pool);
    TEST_ASSERT_GREATER_THAN(0, used_before);
    
    /* Reset pool */
    memory_pool_reset(test_pool);
    
    size_t used_after = memory_pool_get_used(test_pool);
    TEST_ASSERT_EQUAL_INT(0, used_after);
    
    /* Should be able to allocate again */
    void* ptr2 = memory_pool_alloc(test_pool, 256);
    TEST_ASSERT_NOT_NULL(ptr2);
    TEST_ASSERT_EQUAL_PTR(ptr, ptr2); /* Should get same address */
}

/* Test runner with C99 features */
int main(void) {
    UNITY_BEGIN();
    
    /* C99 compound literal for test array */
    void (*tests[])(void) = {
        test_memory_pool_allocation,
        test_memory_pool_exhaustion,
        test_memory_pool_reset
    };
    
    /* C99 for loop with declaration */
    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        RUN_TEST(tests[i]);
    }
    
    return UNITY_END();
}
```

### Unicorn Engine Integration

```c
/* tests/emulation/test_execution.c - Unicorn engine testing */
#include "unity.h"
#include "emulator.h"
#include <unicorn/unicorn.h>

/* Test context structure */
typedef struct {
    uc_engine* uc;
    uint32_t base_address;
    size_t code_size;
    uint8_t* code_buffer;
} emulation_context_t;

static emulation_context_t* ctx;

void setUp(void) {
    ctx = emulator_context_create(UC_ARCH_X86, UC_MODE_32);
    TEST_ASSERT_NOT_NULL(ctx);
}

void tearDown(void) {
    if (ctx != NULL) {
        emulator_context_destroy(ctx);
        ctx = NULL;
    }
}

/* Test SMOF executable execution */
void test_smof_executable_execution(void) {
    /* Load test SMOF file */
    const char* test_file = "tests/fixtures/simple.smof";
    int result = emulator_load_smof(ctx, test_file);
    TEST_ASSERT_EQUAL(0, result);
    
    /* Set up execution environment */
    result = emulator_setup_stack(ctx, 0x1000, 4096);
    TEST_ASSERT_EQUAL(0, result);
    
    /* Execute until return */
    uint32_t return_value;
    result = emulator_execute_until_return(ctx, &return_value);
    TEST_ASSERT_EQUAL(0, result);
    
    /* Verify expected return value */
    TEST_ASSERT_EQUAL_HEX32(0x42, return_value);
}

/* Test relocation correctness */
void test_relocation_validation(void) {
    /* Load object file with relocations */
    const char* obj_file = "tests/fixtures/relocatable.smof";
    int result = emulator_load_smof_at_address(ctx, obj_file, 0x10000);
    TEST_ASSERT_EQUAL(0, result);
    
    /* Verify relocated addresses */
    uint32_t relocated_addr;
    result = emulator_read_memory_u32(ctx, 0x10020, &relocated_addr);
    TEST_ASSERT_EQUAL(0, result);
    
    /* Address should be adjusted for base */
    TEST_ASSERT_EQUAL_HEX32(0x10100, relocated_addr);
}

/* Memory protection testing */
void test_memory_protection(void) {
    /* Map code section as executable only */
    result = emulator_map_memory(ctx, 0x1000, 4096, UC_PROT_EXEC);
    TEST_ASSERT_EQUAL(0, result);
    
    /* Map data section as read-write */
    result = emulator_map_memory(ctx, 0x2000, 4096, UC_PROT_READ | UC_PROT_WRITE);
    TEST_ASSERT_EQUAL(0, result);
    
    /* Try to write to code section (should fail) */
    uint32_t test_value = 0xDEADBEEF;
    result = emulator_write_memory(ctx, 0x1000, &test_value, sizeof(test_value));
    TEST_ASSERT_NOT_EQUAL(0, result);
    
    /* Write to data section (should succeed) */
    result = emulator_write_memory(ctx, 0x2000, &test_value, sizeof(test_value));
    TEST_ASSERT_EQUAL(0, result);
}

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_smof_executable_execution);
    RUN_TEST(test_relocation_validation);
    RUN_TEST(test_memory_protection);
    
    return UNITY_END();
}
```

### Emulator Helper Implementation

```c
/* tests/emulation/emulator.c - Unicorn engine wrapper */
#include "emulator.h"
#include <stdlib.h>
#include <string.h>

typedef struct emulation_context {
    uc_engine* uc;
    uint32_t base_address;
    size_t mapped_size;
    /* C99 flexible array member for tracking mapped regions */
    struct {
        uint32_t address;
        size_t size;
        uint32_t perms;
    } mappings[];
} emulation_context_t;

emulation_context_t* emulator_context_create(uc_arch arch, uc_mode mode) {
    emulation_context_t* ctx = calloc(1, sizeof(emulation_context_t));
    if (ctx == NULL) {
        return NULL;
    }
    
    uc_err err = uc_open(arch, mode, &ctx->uc);
    if (err != UC_ERR_OK) {
        free(ctx);
        return NULL;
    }
    
    ctx->base_address = 0x1000;
    return ctx;
}

void emulator_context_destroy(emulation_context_t* ctx) {
    if (ctx != NULL) {
        if (ctx->uc != NULL) {
            uc_close(ctx->uc);
        }
        free(ctx);
    }
}

int emulator_load_smof(emulation_context_t* ctx, const char* filename) {
    /* Implementation to load and parse SMOF file */
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        return -1;
    }
    
    /* Read SMOF header */
    smof_header_t header;
    if (fread(&header, sizeof(header), 1, file) != 1) {
        fclose(file);
        return -1;
    }
    
    /* Validate header */
    if (!smof_header_is_valid(&header)) {
        fclose(file);
        return -1;
    }
    
    /* Map memory for sections and load data */
    /* Implementation details... */
    
    fclose(file);
    return 0;
}

int emulator_execute_until_return(emulation_context_t* ctx, uint32_t* return_value) {
    /* Set up hooks for return detection */
    uc_hook hook;
    bool returned = false;
    uint32_t ret_val = 0;
    
    /* Hook for return instruction */
    auto ret_hook = [](uc_engine* uc, uint64_t address, uint32_t size, void* user_data) {
        bool* returned_flag = (bool*)user_data;
        *returned_flag = true;
        uc_emu_stop(uc);
    };
    
    uc_err err = uc_hook_add(ctx->uc, &hook, UC_HOOK_CODE, ret_hook, &returned, 1, 0);
    if (err != UC_ERR_OK) {
        return -1;
    }
    
    /* Start execution */
    err = uc_emu_start(ctx->uc, ctx->base_address, 0, 0, 0);
    
    /* Get return value from EAX register */
    if (returned && return_value != NULL) {
        uc_reg_read(ctx->uc, UC_X86_REG_EAX, return_value);
    }
    
    uc_hook_del(ctx->uc, hook);
    return (err == UC_ERR_OK) ? 0 : -1;
}
```

## Code Coverage and Quality Assurance

### Coverage Analysis Script

```bash
#!/bin/bash
# tools/coverage.sh - Code coverage analysis

set -e

COVERAGE_DIR="coverage"
BUILD_DIR="build"

echo "Generating code coverage report..."

# Clean previous coverage data
rm -rf ${COVERAGE_DIR}
mkdir -p ${COVERAGE_DIR}

# Build with coverage flags
make clean
make coverage DEBUG=1

# Run all tests
echo "Running test suite with coverage..."
${BUILD_DIR}/test_runner

# Generate coverage data
echo "Processing coverage data..."
lcov --capture \
     --directory ${BUILD_DIR} \
     --output-file ${COVERAGE_DIR}/coverage.info \
     --rc lcov_branch_coverage=1

# Remove system headers and test files from coverage
lcov --remove ${COVERAGE_DIR}/coverage.info \
     '/usr/*' \
     '*/tests/*' \
     '*/unity/*' \
     --output-file ${COVERAGE_DIR}/coverage_filtered.info \
     --rc lcov_branch_coverage=1

# Generate HTML report
genhtml ${COVERAGE_DIR}/coverage_filtered.info \
        --output-directory ${COVERAGE_DIR}/html \
        --title "STLD/STAR Code Coverage" \
        --show-details \
        --legend \
        --rc lcov_branch_coverage=1

# Generate summary
lcov --summary ${COVERAGE_DIR}/coverage_filtered.info

echo "Coverage report generated in ${COVERAGE_DIR}/html/index.html"

# Check coverage thresholds
FUNCTION_COVERAGE=$(lcov --summary ${COVERAGE_DIR}/coverage_filtered.info 2>&1 | \
                   grep "functions.." | \
                   sed 's/.*(\([0-9.]*\)%.*/\1/')

LINE_COVERAGE=$(lcov --summary ${COVERAGE_DIR}/coverage_filtered.info 2>&1 | \
               grep "lines......" | \
               sed 's/.*(\([0-9.]*\)%.*/\1/')

echo "Function coverage: ${FUNCTION_COVERAGE}%"
echo "Line coverage: ${LINE_COVERAGE}%"

# Set coverage thresholds
FUNCTION_THRESHOLD=90
LINE_THRESHOLD=85

if (( $(echo "${FUNCTION_COVERAGE} < ${FUNCTION_THRESHOLD}" | bc -l) )); then
    echo "ERROR: Function coverage ${FUNCTION_COVERAGE}% below threshold ${FUNCTION_THRESHOLD}%"
    exit 1
fi

if (( $(echo "${LINE_COVERAGE} < ${LINE_THRESHOLD}" | bc -l) )); then
    echo "ERROR: Line coverage ${LINE_COVERAGE}% below threshold ${LINE_THRESHOLD}%"
    exit 1
fi

echo "Coverage thresholds met!"
```

### Static Analysis Script

```bash
#!/bin/bash
# tools/static_analysis.sh - Static analysis tools

set -e

SRC_DIR="src"
ANALYSIS_DIR="analysis"

mkdir -p ${ANALYSIS_DIR}

echo "Running static analysis..."

# cppcheck analysis
echo "Running cppcheck..."
cppcheck --enable=all \
         --std=c99 \
         --platform=unix32 \
         --suppress=missingIncludeSystem \
         --suppress=unusedFunction \
         --xml \
         --xml-version=2 \
         ${SRC_DIR} 2> ${ANALYSIS_DIR}/cppcheck.xml

# Convert to HTML
cppcheck-htmlreport --file=${ANALYSIS_DIR}/cppcheck.xml \
                   --report-dir=${ANALYSIS_DIR}/cppcheck-html \
                   --source-dir=${SRC_DIR}

# Clang static analyzer
echo "Running clang static analyzer..."
scan-build --use-analyzer=$(which clang) \
           --html-title="STLD/STAR Static Analysis" \
           --output=${ANALYSIS_DIR}/clang-analysis \
           --status-bugs \
           make clean all

# PC-lint (if available)
if command -v pc-lint &> /dev/null; then
    echo "Running PC-lint..."
    pc-lint -i${SRC_DIR}/common/include \
            -i${SRC_DIR}/stld/include \
            -i${SRC_DIR}/star/include \
            std.lnt \
            ${SRC_DIR}/*/*.c > ${ANALYSIS_DIR}/pc-lint.txt
fi

# Valgrind memcheck (if tests available)
if [ -f "${BUILD_DIR}/test_runner" ]; then
    echo "Running Valgrind memcheck..."
    valgrind --tool=memcheck \
             --leak-check=full \
             --show-leak-kinds=all \
             --track-origins=yes \
             --xml=yes \
             --xml-file=${ANALYSIS_DIR}/valgrind.xml \
             ${BUILD_DIR}/test_runner
fi

echo "Static analysis complete. Reports in ${ANALYSIS_DIR}/"
```

## Continuous Integration Setup

### GitHub Actions Workflow

```yaml
# .github/workflows/ci.yml
name: CI

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  build-and-test:
    runs-on: ubuntu-latest
    
    strategy:
      matrix:
        compiler: [gcc-9, gcc-10, gcc-11, clang-10, clang-12]
        build-type: [debug, release]
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y \
          build-essential \
          lcov \
          cppcheck \
          clang-tools \
          valgrind \
          doxygen \
          libunicorn-dev \
          pkg-config
    
    - name: Setup compiler
      run: |
        if [[ "${{ matrix.compiler }}" == gcc-* ]]; then
          sudo apt-get install -y ${{ matrix.compiler }}
          echo "CC=${{ matrix.compiler }}" >> $GITHUB_ENV
        elif [[ "${{ matrix.compiler }}" == clang-* ]]; then
          sudo apt-get install -y ${{ matrix.compiler }}
          echo "CC=${{ matrix.compiler }}" >> $GITHUB_ENV
        fi
    
    - name: Build
      run: |
        DEBUG=${{ matrix.build-type == 'debug' && '1' || '0' }}
        make all DEBUG=${DEBUG}
    
    - name: Run tests
      run: make tests
    
    - name: Run static analysis
      if: matrix.compiler == 'gcc-11' && matrix.build-type == 'debug'
      run: tools/static_analysis.sh
    
    - name: Generate coverage
      if: matrix.compiler == 'gcc-11' && matrix.build-type == 'debug'
      run: make coverage
    
    - name: Upload coverage to Codecov
      if: matrix.compiler == 'gcc-11' && matrix.build-type == 'debug'
      uses: codecov/codecov-action@v3
      with:
        file: coverage/coverage_filtered.info
        flags: unittests
        name: codecov-umbrella
    
    - name: Generate documentation
      if: matrix.compiler == 'gcc-11' && matrix.build-type == 'release'
      run: make docs
    
    - name: Archive artifacts
      uses: actions/upload-artifact@v3
      with:
        name: build-artifacts-${{ matrix.compiler }}-${{ matrix.build-type }}
        path: |
          build/stld
          build/star
          build/libstld.a
          build/libstar.a
          build/libcommon.a

  cross-compile:
    runs-on: ubuntu-latest
    
    strategy:
      matrix:
        target: [arm-linux-gnueabihf, aarch64-linux-gnu, riscv64-linux-gnu]
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Install cross-compilation tools
      run: |
        sudo apt-get update
        sudo apt-get install -y gcc-${{ matrix.target }}
    
    - name: Cross-compile
      run: |
        make all CROSS_COMPILE=${{ matrix.target }}-
    
    - name: Archive cross-compiled binaries
      uses: actions/upload-artifact@v3
      with:
        name: cross-compile-${{ matrix.target }}
        path: |
          build/stld
          build/star

  embedded-test:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Install embedded tools
      run: |
        sudo apt-get update
        sudo apt-get install -y \
          gcc-arm-none-eabi \
          qemu-system-arm
    
    - name: Build for embedded target
      run: |
        make all CROSS_COMPILE=arm-none-eabi- \
                  TARGET_ARCH=cortex-m4 \
                  CFLAGS="$(CFLAGS) -mthumb -specs=nosys.specs"
    
    - name: Test on QEMU
      run: |
        # Run basic functionality test on emulated ARM Cortex-M4
        qemu-system-arm -machine mps2-an385 \
                       -cpu cortex-m4 \
                       -kernel build/stld \
                       -nographic \
                       -monitor none \
                       -serial stdio \
                       -semihosting-config enable=on,target=native
```

## Performance Optimization and Best Practices

### Memory Usage Optimization

```c
/* Memory-optimized symbol table with C99 features */
/* src/stld/symbol_table.c */

/* Use bit fields for compact storage */
typedef struct stld_symbol_compact {
    uint32_t name_hash;
    uint32_t value;
    uint16_t section_index;
    /* C99 bit fields for flags */
    uint8_t type : 4;
    uint8_t binding : 2;
    uint8_t visibility : 2;
    uint8_t reserved : 0;  /* Force alignment */
} stld_symbol_compact_t;

/* C99 static assertion for size optimization */
_Static_assert(sizeof(stld_symbol_compact_t) == 12, 
               "Compact symbol must be 12 bytes");

/* Memory pool for string storage */
typedef struct string_pool {
    char* data;
    size_t size;
    size_t used;
    /* C99 flexible array member for hash table */
    uint32_t hash_table[];
} string_pool_t;

/* C99 designated initializers for configuration */
static const struct symbol_table_config {
    size_t initial_capacity;
    double load_factor;
    size_t string_pool_size;
} default_config = {
    .initial_capacity = 256,
    .load_factor = 0.75,
    .string_pool_size = 8192
};
```

### Embedded Systems Considerations

```c
/* Embedded-friendly memory management */
/* src/common/embedded_memory.c */

#ifdef EMBEDDED_TARGET

/* Fixed-size pools for deterministic allocation */
#define MAX_SYMBOLS 1024
#define MAX_SECTIONS 64
#define STRING_POOL_SIZE 4096

static struct {
    stld_symbol_t symbols[MAX_SYMBOLS];
    stld_section_t sections[MAX_SECTIONS];
    char string_pool[STRING_POOL_SIZE];
    /* C99 compound literal for free lists */
    struct {
        uint16_t symbol_free_list[MAX_SYMBOLS];
        uint16_t section_free_list[MAX_SECTIONS];
        size_t string_pool_used;
    } allocator;
} static_memory;

/* C99 inline functions for embedded allocation */
static inline stld_symbol_t* alloc_symbol(void) {
    if (static_memory.allocator.symbol_free_list[0] == 0) {
        return NULL;  /* Out of symbols */
    }
    
    uint16_t index = static_memory.allocator.symbol_free_list[0];
    static_memory.allocator.symbol_free_list[0] = 
        static_memory.allocator.symbol_free_list[index];
    
    return &static_memory.symbols[index - 1];
}

static inline void free_symbol(stld_symbol_t* symbol) {
    if (symbol == NULL) return;
    
    uint16_t index = (uint16_t)(symbol - static_memory.symbols) + 1;
    static_memory.allocator.symbol_free_list[index] = 
        static_memory.allocator.symbol_free_list[0];
    static_memory.allocator.symbol_free_list[0] = index;
}

#endif /* EMBEDDED_TARGET */
```

## Documentation and Examples

### API Documentation Template

```c
/**
 * @file stld.h
 * @brief STIX Linker (STLD) main API
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * @details
 * This header provides the main API for the STIX linker. The linker
 * processes SMOF (STIX Minimal Object Format) files and produces
 * executable programs or shared libraries optimized for resource-
 * constrained environments.
 * 
 * @par C99 Compliance
 * This implementation strictly follows the C99 standard (ISO/IEC 9899:1999)
 * and does not use any compiler-specific extensions.
 * 
 * @par Memory Requirements
 * The linker is designed to operate within 64KB of memory during
 * the linking process, making it suitable for embedded systems.
 * 
 * @example simple_linking.c
 * @code{.c}
 * #include <stld.h>
 * 
 * int main(void) {
 *     // C99 designated initializers for SMOF executable
 *     const struct stld_options opts = {
 *         .output_type = STLD_OUTPUT_EXECUTABLE,
 *         .optimize_size = true,
 *         .entry_point = 0x1000
 *     };
 *     
 *     const char* inputs[] = {"main.smof", "lib.smof"};
 *     
 *     int result = stld_link(inputs, 2, "program.exe", &opts);
 *     return result;
 * }
 * @endcode
 * 
 * @example kernel_binary.c
 * @code{.c}
 * #include <stld.h>
 * 
 * int main(void) {
 *     // Binary flat output for OS kernel
 *     const struct stld_options opts = {
 *         .output_type = STLD_OUTPUT_BINARY_FLAT,
 *         .base_address = 0x100000,    // 1MB load address
 *         .max_memory = 32768,         // 32KB linker memory limit
 *         .optimize_size = true
 *     };
 *     
 *     const char* kernel_objects[] = {
 *         "kernel_main.smof",
 *         "kernel_mm.smof", 
 *         "kernel_drivers.smof"
 *     };
 *     
 *     // Link kernel binary - produces raw binary + memory map
 *     int result = stld_link(kernel_objects, 3, "kernel.bin", &opts);
 *     if (result == 0) {
 *         printf("Kernel binary generated: kernel.bin\n");
 *         printf("Memory map generated: kernel.map\n");
 *     }
 *     return result;
 * }
 * @endcode
 * 
 * @example bootloader_binary.c
 * @code{.c}
 * #include <stld.h>
 * 
 * int main(void) {
 *     // Bootloader binary with size constraints
 *     const struct stld_options opts = {
 *         .output_type = STLD_OUTPUT_BINARY_FLAT,
 *         .base_address = 0x7C00,      // Boot sector load address
 *         .max_memory = 16384,         // 16KB linker memory limit
 *         .optimize_size = true
 *     };
 *     
 *     const char* boot_objects[] = {"bootloader.smof"};
 *     
 *     // Generate 512-byte boot sector
 *     int result = stld_link(boot_objects, 1, "boot.bin", &opts);
 *     return result;
 * }
 * @endcode
 * 
 * @copyright Copyright (c) 2025 STIX Project
 * @license MIT License
 */

#ifndef STLD_H_INCLUDED
#define STLD_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup stld_api STLD Public API
 * @{
 */

/**
 * @brief Linker output type enumeration
 */
typedef enum {
    STLD_OUTPUT_EXECUTABLE,     /**< Executable program */
    STLD_OUTPUT_SHARED_LIBRARY, /**< Shared library */
    STLD_OUTPUT_STATIC_LIBRARY, /**< Static library */
    STLD_OUTPUT_OBJECT,         /**< Relocatable object */
    STLD_OUTPUT_BINARY_FLAT     /**< Binary flat format (OS/embedded) */
} stld_output_type_t;

/**
 * @brief Linker options structure
 * 
 * @details
 * Configuration structure for linker behavior. All fields use
 * C99 standard types and follow embedded systems best practices.
 */
typedef struct stld_options {
    stld_output_type_t output_type; /**< Type of output to generate */
    uint32_t entry_point;           /**< Entry point address (0 = auto) */
    uint32_t base_address;          /**< Base load address for binary flat */
    bool optimize_size;             /**< Optimize for size over speed */
    bool strip_debug;               /**< Remove debug information */
    bool position_independent;      /**< Generate position independent code */
    size_t max_memory;              /**< Maximum memory usage (0 = unlimited) */
    bool fill_gaps;                 /**< Fill gaps in binary flat output */
    uint8_t fill_value;             /**< Byte value for gap filling */
    uint32_t max_file_size;         /**< Maximum output file size */
    bool generate_map;              /**< Generate memory map file */
} stld_options_t;

/**
 * @brief Link multiple object files into output
 * 
 * @param[in] input_files Array of input file paths
 * @param[in] input_count Number of input files
 * @param[in] output_file Output file path
 * @param[in] options Linker options (NULL for defaults)
 * 
 * @return 0 on success, negative error code on failure
 * 
 * @par Thread Safety
 * This function is not thread-safe. Multiple concurrent calls
 * may interfere with each other.
 * 
 * @par Memory Usage
 * Peak memory usage is approximately 2-3 times the total size
 * of input files, plus symbol table overhead.
 * 
 * @note All file paths must be valid and accessible. The function
 *       will create the output file and overwrite if it exists.
 */
int stld_link(const char* const* input_files,
              size_t input_count,
              const char* output_file,
              const stld_options_t* options);

/** @} */ /* End of stld_api group */

#ifdef __cplusplus
}
#endif

#endif /* STLD_H_INCLUDED */
```

This comprehensive implementation guide provides a complete framework for building STLD and STAR with C99 compliance, Makefile-based build system, Unity testing framework, and Unicorn engine integration. The architecture emphasizes embedded systems best practices, memory efficiency, and rigorous testing methodology.
