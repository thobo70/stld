# STLD/STAR Project - C99 & Makefile Implementation

## Overview

This project implements STLD (STIX Linker) and STAR (STIX Archiver) using strict C99 compliance and Makefile-based build system. The tools are designed for the STIX operating system and work with SMOF (STIX Minimal Object Format) files optimized for resource-constrained embedded systems.

## Quick Start

### Automated Setup

For the fastest development environment setup, use the provided automation scripts:

```bash
# Clone the repository
git clone <repository-url>
cd stld

# Interactive setup (checks existing tools, asks for confirmation)
./setup_dev.sh

# OR: Non-interactive setup (auto-installs everything)
./setup_dev.sh --auto

# Verify installation
./verify_environment.sh

# Build the project
./build.sh

# Run tests
./test.sh

# Check development status
./dev-status.sh
```

**Setup Script Features:**
- **Smart Detection**: Checks what tools are already installed
- **Interactive Mode**: Asks for confirmation before installing each category
- **Non-interactive Mode**: Use `--auto` or `-y` for automated installation
- **Robust Error Handling**: Continues installation even if some packages fail
- **Version Detection**: Automatically detects available compiler versions
- **Graceful Degradation**: Works even when some packages are unavailable
- **Installation Summary**: Shows errors and warnings at completion
- **Cross-platform**: Works on Ubuntu/Debian, Fedora/RHEL, and macOS

### Manual Setup

If you prefer manual installation, follow the detailed platform-specific instructions in the [Development Environment Setup](#development-environment-setup) section below.

## Architecture Updates

### Language Standard: C99 (ISO/IEC 9899:1999)

All source code has been updated to strictly comply with C99:

- **Header Guards**: Using `#ifndef`/`#define`/`#endif` pattern
- **Type Definitions**: Using `typedef struct` instead of anonymous structs
- **Standard Headers**: Using `<stdint.h>`, `<stdbool.h>`, `<stddef.h>`
- **Comments**: C-style `/* */` comments only
- **Static Assertions**: Using `_Static_assert` for compile-time checks
- **Designated Initializers**: `{.field = value}` syntax
- **Flexible Array Members**: Using `type[]` at end of structs
- **Compound Literals**: `(type){...}` syntax
- **Bool Type**: Using `bool`, `true`, `false` from `<stdbool.h>`

### Build System: Makefile-based

Replaced CMake with GNU Make:

- **Main Makefile**: Root build configuration
- **config.mk**: Build variables and compiler settings
- **Rules.mk**: Common build rules and utilities
- **Dependency Tracking**: Automatic `.d` file generation
- **Cross-compilation**: Support for embedded targets
- **Parallel Builds**: `-j` flag support

## Project Structure

```
stld/
├── Makefile                    # Main build file
├── config.mk                  # Build configuration
├── Rules.mk                   # Common rules
├── project_implementation.md  # Implementation guide
├── stld_architecture.md       # STLD architecture (updated)
├── star_architecture.md       # STAR architecture (updated)
├── stix_minimal_object_format.md
├── object_formats_comparison.md
│
├── src/                       # Source code (C99)
│   ├── common/                # Shared components
│   ├── stld/                  # STLD linker
│   └── star/                  # STAR archiver
│
├── tests/                     # Test suite
│   ├── unity/                 # Unity framework
│   ├── unit/                  # Unit tests
│   ├── integration/           # Integration tests
│   ├── emulation/             # Unicorn engine tests
│   └── performance/           # Benchmarks
│
├── tools/                     # Development tools
│   ├── smof_dump.c           # SMOF analyzer
│   ├── coverage.sh           # Coverage script
│   └── static_analysis.sh    # Static analysis
│
├── docs/                      # Documentation
└── examples/                  # Usage examples
```

## Development Environment Setup

### Prerequisites

Before building STLD/STAR, you need to install the required development tools and libraries. This section provides installation instructions for various operating systems.

### Core Build Tools

#### Ubuntu/Debian Linux

```bash
# Update package manager
sudo apt update

# Core build tools
sudo apt install -y \
    build-essential \
    gcc \
    g++ \
    make \
    git \
    pkg-config \
    autoconf \
    automake \
    libtool

# Additional compilers for testing
sudo apt install -y \
    gcc-9 \
    gcc-10 \
    gcc-11 \
    clang-10 \
    clang-11 \
    clang-12 \
    clang-tools

# Cross-compilation toolchains
sudo apt install -y \
    gcc-arm-linux-gnueabihf \
    gcc-aarch64-linux-gnu \
    gcc-riscv64-linux-gnu \
    gcc-arm-none-eabi
```

#### Red Hat/CentOS/Fedora

```bash
# For RHEL/CentOS 8+
sudo dnf groupinstall "Development Tools"
sudo dnf install -y \
    gcc \
    gcc-c++ \
    make \
    git \
    pkgconfig \
    autoconf \
    automake \
    libtool

# Additional compilers
sudo dnf install -y \
    clang \
    clang-tools-extra

# Cross-compilation (may require EPEL)
sudo dnf install -y \
    gcc-arm-linux-gnu \
    gcc-aarch64-linux-gnu
```

#### macOS

```bash
# Install Xcode command line tools
xcode-select --install

# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install development tools
brew install \
    gcc \
    llvm \
    make \
    git \
    pkg-config \
    autoconf \
    automake \
    libtool

# Cross-compilation toolchains
brew install \
    arm-none-eabi-gcc \
    riscv64-elf-gcc
```

#### Windows (WSL2 or MSYS2)

**Option 1: WSL2 (Recommended)**
```bash
# Install WSL2 Ubuntu and follow Ubuntu instructions above
wsl --install -d Ubuntu-22.04
```

**Option 2: MSYS2**
```bash
# In MSYS2 terminal
pacman -S \
    mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-clang \
    mingw-w64-x86_64-make \
    git \
    pkg-config
```

### Testing Framework Dependencies

#### Unity Testing Framework

Unity is included in the repository, but you may want additional testing tools:

```bash
# Ubuntu/Debian
sudo apt install -y \
    check \
    libcriterion-dev \
    cunit-dev

# Fedora/RHEL
sudo dnf install -y \
    check-devel \
    CUnit-devel

# macOS
brew install \
    check \
    cunit
```

#### Unicorn Engine for CPU Emulation

```bash
# Ubuntu/Debian
sudo apt install -y \
    libunicorn-dev \
    libunicorn1

# From source (if package not available)
git clone https://github.com/unicorn-engine/unicorn.git
cd unicorn
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
sudo ldconfig
```

```bash
# Fedora/RHEL
sudo dnf install -y \
    unicorn-devel

# macOS
brew install unicorn
```

### Code Quality Tools

#### Static Analysis

```bash
# Ubuntu/Debian
sudo apt install -y \
    cppcheck \
    clang-tidy \
    clang-format \
    vera++ \
    splint

# Fedora/RHEL
sudo dnf install -y \
    cppcheck \
    clang-analyzer \
    clang-tools-extra

# macOS
brew install \
    cppcheck \
    llvm
```

#### Code Coverage

```bash
# Ubuntu/Debian
sudo apt install -y \
    lcov \
    gcovr \
    llvm

# Fedora/RHEL
sudo dnf install -y \
    lcov \
    gcovr

# macOS
brew install \
    lcov \
    gcovr
```

### Memory Debugging Tools

```bash
# Ubuntu/Debian
sudo apt install -y \
    valgrind \
    gdb \
    strace \
    ltrace

# Fedora/RHEL
sudo dnf install -y \
    valgrind \
    gdb \
    strace \
    ltrace

# macOS (limited support)
brew install \
    lldb
```

### Documentation Tools

```bash
# Ubuntu/Debian
sudo apt install -y \
    doxygen \
    graphviz \
    texlive-latex-base \
    texlive-fonts-recommended

# Fedora/RHEL
sudo dnf install -y \
    doxygen \
    graphviz \
    texlive-scheme-basic

# macOS
brew install \
    doxygen \
    graphviz
```

### Compression Libraries (Optional)

For enhanced STAR archive compression:

```bash
# Ubuntu/Debian
sudo apt install -y \
    zlib1g-dev \
    liblz4-dev \
    liblzma-dev \
    libzstd-dev

# Fedora/RHEL
sudo dnf install -y \
    zlib-devel \
    lz4-devel \
    xz-devel \
    libzstd-devel

# macOS
brew install \
    zlib \
    lz4 \
    xz \
    zstd
```

### Embedded Development Tools

For embedded and OS development:

```bash
# Ubuntu/Debian
sudo apt install -y \
    qemu-system \
    qemu-user \
    qemu-utils \
    gdb-multiarch

# Debuggers for embedded targets
sudo apt install -y \
    openocd \
    gdb-arm-none-eabi

# Fedora/RHEL
sudo dnf install -y \
    qemu \
    qemu-user \
    arm-none-eabi-gdb

# macOS
brew install \
    qemu \
    openocd
```

### Verification Script

Create a script to verify your development environment:

```bash
#!/bin/bash
# verify_environment.sh - Check development environment

echo "Checking STLD/STAR development environment..."

# Check compilers
check_command() {
    if command -v "$1" &> /dev/null; then
        echo "✓ $1 found: $($1 --version | head -n1)"
    else
        echo "✗ $1 not found"
        return 1
    fi
}

echo "Core tools:"
check_command gcc
check_command clang
check_command make
check_command git
check_command pkg-config

echo -e "\nTesting tools:"
check_command valgrind
check_command gdb

echo -e "\nStatic analysis:"
check_command cppcheck
check_command clang-tidy

echo -e "\nCoverage tools:"
check_command lcov
check_command gcov

echo -e "\nDocumentation:"
check_command doxygen

echo -e "\nOptional libraries:"
pkg-config --exists unicorn && echo "✓ Unicorn engine found" || echo "✗ Unicorn engine not found"
pkg-config --exists zlib && echo "✓ zlib found" || echo "✗ zlib not found"
pkg-config --exists liblz4 && echo "✓ LZ4 found" || echo "✗ LZ4 not found"

echo -e "\nCross-compilation:"
check_command arm-linux-gnueabihf-gcc
check_command aarch64-linux-gnu-gcc
check_command arm-none-eabi-gcc

echo -e "\nEmulation:"
check_command qemu-system-x86_64
check_command qemu-arm

echo "Environment check complete!"
```

Make it executable and run:
```bash
chmod +x verify_environment.sh
./verify_environment.sh
```

### Docker Development Environment

For a consistent development environment across platforms:

```dockerfile
# Dockerfile.dev
FROM ubuntu:22.04

# Install development tools
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-9 gcc-10 gcc-11 \
    clang-10 clang-11 clang-12 \
    clang-tools \
    make \
    git \
    pkg-config \
    libunicorn-dev \
    cppcheck \
    valgrind \
    lcov \
    doxygen \
    graphviz \
    zlib1g-dev \
    liblz4-dev \
    qemu-system \
    gcc-arm-linux-gnueabihf \
    gcc-aarch64-linux-gnu \
    gcc-arm-none-eabi \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
CMD ["bash"]
```

Build and use:
```bash
docker build -f Dockerfile.dev -t stld-dev .
docker run -it -v $(pwd):/workspace stld-dev
```

### IDE Setup

#### Visual Studio Code

Install recommended extensions:
```bash
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cmake-tools
code --install-extension twxs.cmake
code --install-extension ms-vscode.makefile-tools
code --install-extension llvm-vs-code-extensions.vscode-clangd
```

#### Vim/Neovim

For C development with LSP support:
```bash
# Install clangd
sudo apt install clangd

# Add to .vimrc or init.vim
# Plugin manager setup for LSP
```

### Troubleshooting

#### Common Issues

**1. Unicorn Engine not found:**
```bash
# Check if installed
pkg-config --exists unicorn
# If not, install from source or package manager
```

**2. Cross-compiler not working:**
```bash
# Verify installation
arm-linux-gnueabihf-gcc --version
# Check if multiarch is enabled (Debian/Ubuntu)
sudo dpkg --add-architecture armhf
sudo apt update
```

**3. Valgrind issues on newer systems:**
```bash
# May need newer version
sudo apt install valgrind/jammy-backports
```

**4. Coverage data not generated:**
```bash
# Ensure gcov matches gcc version
gcov --version
gcc --version
```

This comprehensive setup ensures you have all the tools needed for STLD/STAR development, testing, and deployment across different target platforms.

## Build Commands

### Basic Build
```bash
make all                      # Build everything
make stld                     # Build STLD linker only
make star                     # Build STAR archiver only
make tools                    # Build utility tools
```

### Development
```bash
make tests                    # Build and run tests
make coverage                 # Generate coverage report
make docs                     # Generate documentation
make static-analysis          # Run static analysis
```

### Configuration
```bash
make DEBUG=1                  # Debug build
make DEBUG=0                  # Release build
make CROSS_COMPILE=arm-linux-gnueabihf-  # Cross-compile
make V=1                      # Verbose output
```

## Testing Framework

### Unity Framework
- **Unit Tests**: Individual component testing
- **Integration Tests**: Multi-component scenarios  
- **C99 Compliance**: All test code follows C99 standard
- **Memory Testing**: Valgrind and AddressSanitizer support

### Unicorn Engine Integration
- **Execution Testing**: Validate linked executables
- **CPU Emulation**: Test on different architectures
- **Memory Protection**: Verify section permissions
- **Relocation Validation**: Check address calculations

### Code Coverage
- **gcov/lcov**: Line and function coverage
- **Threshold Enforcement**: Minimum 85% line, 90% function
- **HTML Reports**: Detailed coverage visualization
- **CI Integration**: Automated coverage reporting

## Key Features

### C99 Compliance Benefits
- **Portability**: Works on any C99-compliant compiler
- **Embedded Support**: Suitable for resource-constrained systems
- **Standards Compliance**: No compiler-specific extensions
- **Static Analysis**: Better tool support for C99 code

### Makefile Advantages
- **Simplicity**: No complex build system dependencies
- **Speed**: Faster incremental builds
- **Embedded Toolchains**: Easy integration with cross-compilers
- **Debugging**: Transparent build process

### Memory Optimization
- **Fixed Pools**: Predictable memory allocation
- **<64KB Linker**: STLD operates within 64KB memory
- **<32KB Archiver**: STAR operates within 32KB memory
- **Static Assertions**: Compile-time size verification

## Documentation Updates

### Architecture Documents
- **STLD Architecture**: Updated with C99 code examples and Makefile rules
- **STAR Architecture**: Updated with C99 structures and build configuration
- **Implementation Guide**: Comprehensive C99/Makefile setup instructions

### API Documentation
- **Doxygen Comments**: C99-style documentation
- **Type Safety**: Proper const-correctness
- **Error Handling**: Consistent return value conventions
- **Example Code**: C99-compliant usage examples

## Continuous Integration

### GitHub Actions Workflow
- **Multiple Compilers**: GCC 9-11, Clang 10-12
- **Cross-compilation**: ARM, AArch64, RISC-V
- **Static Analysis**: cppcheck, clang-static-analyzer
- **Code Coverage**: Codecov integration
- **Embedded Testing**: QEMU-based validation

### Quality Assurance
- **C99 Strict Mode**: `-std=c99 -pedantic`
- **Warning Level**: `-Wall -Wextra` with additional checks
- **Memory Safety**: AddressSanitizer and Valgrind
- **Performance**: Benchmarking and profiling

## Migration from CMake

The project has been successfully migrated from CMake to Makefile-based build:

1. **Build Scripts**: Converted CMakeLists.txt to Makefile rules
2. **Dependencies**: Manual dependency tracking with `.d` files  
3. **Configuration**: Moved from CMake variables to Make variables
4. **Testing**: Integrated Unity framework with Make targets
5. **Documentation**: Updated all build references

## Getting Started

### Prerequisites

Before building the project, ensure you have installed all required development tools and libraries as described in the [Development Environment Setup](#development-environment-setup) section above.

### Quick Start

1. **Clone Repository**:
   ```bash
   git clone <repository-url>
   cd stld
   ```

2. **Verify Environment**:
   ```bash
   # Run the verification script
   ./verify_environment.sh
   ```

3. **Build Project**:
   ```bash
   make all
   ```

4. **Run Tests**:
   ```bash
   make tests
   ```

5. **Generate Documentation**:
   ```bash
   make docs
   ```

6. **Install**:
   ```bash
   make install PREFIX=/usr/local
   ```

### Development Workflow

1. **Setup Development Environment**:
   ```bash
   # Install all dependencies (see installation section)
   # Verify with verification script
   ./verify_environment.sh
   ```

2. **Configure Build**:
   ```bash
   # Debug build (default)
   make clean && make DEBUG=1
   
   # Release build
   make clean && make DEBUG=0
   
   # Cross-compile for ARM
   make clean && make CROSS_COMPILE=arm-linux-gnueabihf-
   ```

3. **Development Cycle**:
   ```bash
   # Edit source code
   vim src/stld/symbol_table.c
   
   # Build and test
   make stld
   make tests
   
   # Check code quality
   make static-analysis
   make coverage
   ```

4. **Documentation**:
   ```bash
   # Generate API documentation
   make docs
   
   # View in browser
   firefox docs/api/html/index.html
   ```

### Docker Development (Alternative)

If you prefer a containerized development environment:

```bash
# Build development container
docker build -f Dockerfile.dev -t stld-dev .

# Start development session
docker run -it -v $(pwd):/workspace stld-dev

# Inside container
cd /workspace
make all
make tests
```

## Performance Targets

- **Link Time**: <1 second for typical embedded applications
- **Memory Usage**: STLD <64KB, STAR <32KB during operation
- **File Size**: <200 bytes overhead per object file
- **Compression**: 20-40% size reduction in STAR archives

## Compatibility

- **Compilers**: GCC 4.9+, Clang 3.8+, embedded toolchains
- **Platforms**: Linux, embedded systems, cross-compilation
- **Standards**: Strict C99 compliance, POSIX.1-2001
- **Memory**: Optimized for systems with <100KB available memory

This implementation provides a robust, standards-compliant foundation for the STLD/STAR toolchain optimized for embedded and resource-constrained environments.
