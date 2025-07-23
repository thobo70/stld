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
             -Wformat=2 -Wformat-security -Werror

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
TEST_CPPFLAGS := $(CPPFLAGS) -I$(TESTS_DIR)/unity -I$(SRC_DIR)/common/include -I$(SRC_DIR)/stld/include -I$(SRC_DIR)/star/include
TEST_LDFLAGS := $(LDFLAGS)

# Unicorn engine configuration
UNICORN_CFLAGS := $(shell pkg-config --cflags unicorn 2>/dev/null || echo "-I/usr/include/unicorn")
UNICORN_LIBS := $(shell pkg-config --libs unicorn 2>/dev/null || echo "-lunicorn")

# Coverage flags (added when coverage target is used)
COVERAGE_CFLAGS := --coverage -fprofile-arcs -ftest-coverage
COVERAGE_LDFLAGS := --coverage

# Source files
COMMON_SOURCES := $(wildcard $(SRC_DIR)/common/*.c)
STLD_SOURCES := $(wildcard $(SRC_DIR)/stld/*.c)
STAR_SOURCES := $(wildcard $(SRC_DIR)/star/*.c)
TEST_SOURCES := $(wildcard $(TESTS_DIR)/test_*.c) \
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
