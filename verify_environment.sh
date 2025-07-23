#!/bin/bash
# verify_environment.sh - Check development environment for STLD/STAR project
# This script verifies that all required tools and libraries are installed

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Counters
TOTAL_CHECKS=0
PASSED_CHECKS=0
FAILED_CHECKS=0
WARNINGS=0

echo -e "${BLUE}STLD/STAR Development Environment Verification${NC}"
echo "=============================================="
echo ""

# Function to check if a command exists
check_command() {
    local cmd="$1"
    local description="$2"
    local required="$3" # true/false
    
    TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
    
    if command -v "$cmd" &> /dev/null; then
        local version_info
        case "$cmd" in
            gcc|clang|g++|clang++)
                version_info="$($cmd --version | head -n1)"
                ;;
            make)
                version_info="$($cmd --version | head -n1)"
                ;;
            git)
                version_info="$($cmd --version | head -n1)"
                ;;
            pkg-config)
                version_info="$($cmd --version)"
                ;;
            valgrind)
                version_info="$($cmd --version | head -n1)"
                ;;
            gdb)
                version_info="$($cmd --version | head -n1)"
                ;;
            doxygen)
                version_info="$($cmd --version | head -n1)"
                ;;
            *)
                version_info="$($cmd --version 2>/dev/null | head -n1 || echo 'version unknown')"
                ;;
        esac
        echo -e "  ${GREEN}✓${NC} $description: ${version_info}"
        PASSED_CHECKS=$((PASSED_CHECKS + 1))
        return 0
    else
        if [ "$required" = "true" ]; then
            echo -e "  ${RED}✗${NC} $description: not found (REQUIRED)"
            FAILED_CHECKS=$((FAILED_CHECKS + 1))
        else
            echo -e "  ${YELLOW}△${NC} $description: not found (optional)"
            WARNINGS=$((WARNINGS + 1))
        fi
        return 1
    fi
}

# Function to check library with pkg-config
check_library() {
    local lib="$1"
    local description="$2"
    local required="$3" # true/false
    
    TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
    
    if pkg-config --exists "$lib" 2>/dev/null; then
        local version=$(pkg-config --modversion "$lib" 2>/dev/null || echo "unknown")
        echo -e "  ${GREEN}✓${NC} $description: v$version"
        PASSED_CHECKS=$((PASSED_CHECKS + 1))
        return 0
    else
        if [ "$required" = "true" ]; then
            echo -e "  ${RED}✗${NC} $description: not found (REQUIRED)"
            FAILED_CHECKS=$((FAILED_CHECKS + 1))
        else
            echo -e "  ${YELLOW}△${NC} $description: not found (optional)"
            WARNINGS=$((WARNINGS + 1))
        fi
        return 1
    fi
}

# Function to check file existence
check_file() {
    local file="$1"
    local description="$2"
    local required="$3"
    
    TOTAL_CHECKS=$((TOTAL_CHECKS + 1))
    
    if [ -f "$file" ]; then
        echo -e "  ${GREEN}✓${NC} $description: found"
        PASSED_CHECKS=$((PASSED_CHECKS + 1))
        return 0
    else
        if [ "$required" = "true" ]; then
            echo -e "  ${RED}✗${NC} $description: not found (REQUIRED)"
            FAILED_CHECKS=$((FAILED_CHECKS + 1))
        else
            echo -e "  ${YELLOW}△${NC} $description: not found (optional)"
            WARNINGS=$((WARNINGS + 1))
        fi
        return 1
    fi
}

echo -e "${BLUE}Core Build Tools:${NC}"
check_command "gcc" "GCC Compiler" "true"
check_command "make" "GNU Make" "true"
check_command "git" "Git VCS" "true"
check_command "pkg-config" "pkg-config" "true"

echo ""
echo -e "${BLUE}Additional Compilers:${NC}"
check_command "clang" "Clang Compiler" "false"
check_command "gcc-9" "GCC 9" "false"
check_command "gcc-10" "GCC 10" "false"
check_command "gcc-11" "GCC 11" "false"
check_command "clang-10" "Clang 10" "false"
check_command "clang-11" "Clang 11" "false"
check_command "clang-12" "Clang 12" "false"

echo ""
echo -e "${BLUE}Cross-Compilation Toolchains:${NC}"
check_command "arm-linux-gnueabihf-gcc" "ARM Linux GCC" "false"
check_command "aarch64-linux-gnu-gcc" "AArch64 Linux GCC" "false"
check_command "riscv64-linux-gnu-gcc" "RISC-V Linux GCC" "false"
check_command "arm-none-eabi-gcc" "ARM Bare Metal GCC" "false"

echo ""
echo -e "${BLUE}Testing and Debugging Tools:${NC}"
check_command "valgrind" "Valgrind" "false"
check_command "gdb" "GNU Debugger" "false"
check_command "gdb-multiarch" "GDB Multi-arch" "false"

echo ""
echo -e "${BLUE}Static Analysis Tools:${NC}"
check_command "cppcheck" "Cppcheck" "false"
check_command "clang-tidy" "Clang-Tidy" "false"
check_command "clang-format" "Clang-Format" "false"

echo ""
echo -e "${BLUE}Code Coverage Tools:${NC}"
check_command "lcov" "LCOV" "false"
check_command "gcov" "GCOV" "false"
check_command "gcovr" "GCOVR" "false"

echo ""
echo -e "${BLUE}Documentation Tools:${NC}"
check_command "doxygen" "Doxygen" "false"
check_command "graphviz" "Graphviz" "false"
check_command "dot" "Graphviz Dot" "false"

echo ""
echo -e "${BLUE}Emulation Tools:${NC}"
check_command "qemu-system-x86_64" "QEMU x86_64" "false"
check_command "qemu-system-arm" "QEMU ARM" "false"
check_command "qemu-system-aarch64" "QEMU AArch64" "false"

echo ""
echo -e "${BLUE}Required Libraries:${NC}"
check_library "zlib" "zlib compression" "false"

echo ""
echo -e "${BLUE}Optional Libraries:${NC}"
check_library "unicorn" "Unicorn Engine" "false"
check_library "liblz4" "LZ4 compression" "false"
check_library "liblzma" "LZMA compression" "false"
check_library "libzstd" "Zstandard compression" "false"

echo ""
echo -e "${BLUE}Project Files:${NC}"
check_file "Makefile" "Main Makefile" "true"
check_file "config.mk" "Build configuration" "false"
check_file "Rules.mk" "Build rules" "false"
check_file "src/common/include/smof.h" "SMOF header" "false"

# Check C99 compiler support
echo ""
echo -e "${BLUE}C99 Compliance Check:${NC}"
TOTAL_CHECKS=$((TOTAL_CHECKS + 1))

cat > /tmp/c99_test.c << 'EOF'
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// C99 features test
typedef struct {
    uint32_t field1;
    bool field2;
    uint8_t data[];  // flexible array member
} test_struct_t;

int main(void) {
    // Designated initializers
    test_struct_t test = {.field1 = 42, .field2 = true};
    
    // C99 for loop
    for (int i = 0; i < 10; i++) {
        // Variable declaration in for loop
    }
    
    // Compound literal
    int *ptr = (int[]){1, 2, 3, 4};
    
    return 0;
}
EOF

if gcc -std=c99 -pedantic -Wall -Wextra -c /tmp/c99_test.c -o /tmp/c99_test.o 2>/dev/null; then
    echo -e "  ${GREEN}✓${NC} C99 compliance: GCC supports required C99 features"
    PASSED_CHECKS=$((PASSED_CHECKS + 1))
    rm -f /tmp/c99_test.o
else
    echo -e "  ${RED}✗${NC} C99 compliance: GCC does not support required C99 features"
    FAILED_CHECKS=$((FAILED_CHECKS + 1))
fi
rm -f /tmp/c99_test.c /tmp/c99_test.o

# Check system architecture
echo ""
echo -e "${BLUE}System Information:${NC}"
echo "  Architecture: $(uname -m)"
echo "  Kernel: $(uname -s) $(uname -r)"
echo "  Distribution: $(lsb_release -d 2>/dev/null | cut -f2 || echo 'Unknown')"

# Memory and disk space check
echo "  Available RAM: $(free -h | awk '/^Mem:/ {print $7}' || echo 'Unknown')"
echo "  Available disk space: $(df -h . | awk 'NR==2 {print $4}' || echo 'Unknown')"

# CPU cores
echo "  CPU cores: $(nproc)"

# Summary
echo ""
echo "=============================================="
echo -e "${BLUE}Verification Summary:${NC}"
echo "  Total checks: $TOTAL_CHECKS"
echo -e "  ${GREEN}Passed: $PASSED_CHECKS${NC}"
echo -e "  ${RED}Failed: $FAILED_CHECKS${NC}"
echo -e "  ${YELLOW}Warnings: $WARNINGS${NC}"

echo ""

# Provide recommendations based on results
if [ $FAILED_CHECKS -eq 0 ]; then
    echo -e "${GREEN}✓ Environment verification PASSED${NC}"
    echo "Your development environment is ready for STLD/STAR development."
    
    if [ $WARNINGS -gt 0 ]; then
        echo ""
        echo -e "${YELLOW}Recommendations:${NC}"
        echo "  - Consider installing optional tools for enhanced development experience"
        echo "  - Some cross-compilation targets or advanced features may not be available"
    fi
    
    echo ""
    echo "Next steps:"
    echo "  1. Run 'make all' to build the project"
    echo "  2. Run 'make tests' to verify functionality"
    echo "  3. Run 'make docs' to generate documentation"
    
    exit 0
else
    echo -e "${RED}✗ Environment verification FAILED${NC}"
    echo "Please install the missing required tools before proceeding."
    
    echo ""
    echo -e "${YELLOW}Installation help:${NC}"
    echo "  Ubuntu/Debian: sudo apt install build-essential gcc make git pkg-config"
    echo "  RHEL/Fedora:   sudo dnf groupinstall 'Development Tools'"
    echo "  macOS:         xcode-select --install && brew install gcc make"
    echo ""
    echo "For detailed installation instructions, see README.md"
    
    exit 1
fi
