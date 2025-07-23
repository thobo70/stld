#!/bin/bash
# setup_dev.sh - Development environment setup script for STLD/STAR project
# This script helps set up the development environment on various platforms

# Note: We don't use 'set -e' here to allow graceful error handling

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Script configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_NAME="STLD/STAR"

# Global variables for tracking what needs to be installed
MISSING_CORE_TOOLS=()
MISSING_CROSS_TOOLS=()
MISSING_TEST_TOOLS=()
MISSING_ANALYSIS_TOOLS=()
MISSING_DOC_TOOLS=()
MISSING_LIBRARIES=()
MISSING_EMULATION=()

# Error tracking
INSTALL_ERRORS=()
INSTALL_WARNINGS=()

# Interactive mode flag
INTERACTIVE=true
if [[ "$1" == "--auto" ]] || [[ "$1" == "-y" ]]; then
    INTERACTIVE=false
fi

# Error handling function
handle_error() {
    local exit_code=$1
    local command="$2"
    local package="$3"
    
    if [[ $exit_code -ne 0 ]]; then
        local error_msg="Failed to install $package (exit code: $exit_code)"
        INSTALL_ERRORS+=("$error_msg")
        echo -e "${RED}✗ $error_msg${NC}"
        return 1
    else
        echo -e "${GREEN}✓ Successfully installed $package${NC}"
        return 0
    fi
}

# Safe package installation with error handling
safe_install() {
    local package_manager="$1"
    shift
    local packages=("$@")
    local failed_packages=()
    
    for package in "${packages[@]}"; do
        echo -e "${BLUE}Installing $package...${NC}"
        
        case "$package_manager" in
            "apt")
                if ! sudo apt install -y "$package" 2>/dev/null; then
                    failed_packages+=("$package")
                    INSTALL_WARNINGS+=("Failed to install $package via apt")
                    echo -e "${YELLOW}⚠ Warning: Failed to install $package${NC}"
                else
                    echo -e "${GREEN}✓ Installed $package${NC}"
                fi
                ;;
            "dnf")
                if ! sudo dnf install -y "$package" 2>/dev/null; then
                    failed_packages+=("$package")
                    INSTALL_WARNINGS+=("Failed to install $package via dnf")
                    echo -e "${YELLOW}⚠ Warning: Failed to install $package${NC}"
                else
                    echo -e "${GREEN}✓ Installed $package${NC}"
                fi
                ;;
            "brew")
                if ! brew install "$package" 2>/dev/null; then
                    failed_packages+=("$package")
                    INSTALL_WARNINGS+=("Failed to install $package via brew")
                    echo -e "${YELLOW}⚠ Warning: Failed to install $package${NC}"
                else
                    echo -e "${GREEN}✓ Installed $package${NC}"
                fi
                ;;
        esac
    done
    
    if [[ ${#failed_packages[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Some packages failed to install: ${failed_packages[*]}${NC}"
        if [[ "$INTERACTIVE" == "true" ]]; then
            echo "This might be due to:"
            echo "  - Package not available in your distribution version"
            echo "  - Network connectivity issues"
            echo "  - Repository configuration problems"
            echo "  - Insufficient permissions"
        fi
        return 1
    fi
    return 0
}

echo -e "${BLUE}${PROJECT_NAME} Development Environment Setup${NC}"
echo "================================================"
echo ""

if [[ "$INTERACTIVE" == "true" ]]; then
    echo "This script will check for required development tools and offer to install missing ones."
    echo "Use --auto or -y flag to skip confirmation prompts."
    echo ""
fi

# Function to ask user confirmation
ask_confirmation() {
    local message="$1"
    local default="${2:-y}"
    
    if [[ "$INTERACTIVE" == "false" ]]; then
        return 0  # Auto-approve in non-interactive mode
    fi
    
    while true; do
        if [[ "$default" == "y" ]]; then
            read -p "$message [Y/n]: " choice
            choice=${choice:-y}
        else
            read -p "$message [y/N]: " choice
            choice=${choice:-n}
        fi
        
        case "$choice" in
            [Yy]* ) return 0;;
            [Nn]* ) return 1;;
            * ) echo "Please answer yes or no.";;
        esac
    done
}

# Function to check if a command exists
command_exists() {
    command -v "$1" >&/dev/null
}

# Function to check if a package is installed (Ubuntu/Debian)
package_installed_apt() {
    dpkg -l "$1" 2>/dev/null | grep -q "^ii"
}

# Function to check if a package is installed (Fedora/RHEL)
package_installed_dnf() {
    dnf list installed "$1" &>/dev/null || rpm -q "$1" &>/dev/null
}

# Function to check if a package is installed (macOS)
package_installed_brew() {
    brew list "$1" &>/dev/null
}

# Function to detect available clang versions
detect_clang_versions() {
    local available_versions=()
    local os_type="$1"
    
    case "$os_type" in
        "ubuntu")
            # Check for available clang versions in Ubuntu
            for version in 18 17 16 15 14 13 12 11 10; do
                if apt-cache show "clang-$version" &>/dev/null; then
                    available_versions+=("clang-$version")
                fi
            done
            # Also check for generic clang
            if apt-cache show "clang" &>/dev/null; then
                available_versions+=("clang")
            fi
            ;;
        "fedora"|"rhel")
            # Check for available clang versions in Fedora/RHEL
            for version in 18 17 16 15 14 13 12 11 10; do
                if dnf list available "clang$version" &>/dev/null; then
                    available_versions+=("clang$version")
                fi
            done
            # Also check for generic clang
            if dnf list available "clang" &>/dev/null; then
                available_versions+=("clang")
            fi
            ;;
        "macos")
            # On macOS, use LLVM from Homebrew
            available_versions+=("llvm")
            ;;
    esac
    
    echo "${available_versions[@]}"
}

# Function to detect available GCC versions
detect_gcc_versions() {
    local available_versions=()
    local os_type="$1"
    
    case "$os_type" in
        "ubuntu")
            # Check for available GCC versions
            for version in 13 12 11 10 9 8; do
                if apt-cache show "gcc-$version" &>/dev/null; then
                    available_versions+=("gcc-$version")
                fi
            done
            # Also check for generic gcc (part of build-essential)
            if apt-cache show "build-essential" &>/dev/null; then
                available_versions+=("build-essential")
            fi
            ;;
        "fedora"|"rhel")
            # GCC is usually available as part of Development Tools
            available_versions+=("gcc")
            ;;
        "macos")
            # On macOS, use GCC from Homebrew
            available_versions+=("gcc")
            ;;
    esac
    
    echo "${available_versions[@]}"
}

# Detect operating system
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        if command -v apt &> /dev/null; then
            OS="ubuntu"
        elif command -v dnf &> /dev/null; then
            OS="fedora"
        elif command -v yum &> /dev/null; then
            OS="rhel"
        elif command -v pacman &> /dev/null; then
            OS="arch"
        else
            OS="linux"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
        OS="windows"
    else
        OS="unknown"
    fi
}

# Check what tools are missing
check_tools() {
    echo -e "${BLUE}Checking installed development tools...${NC}"
    echo ""
    
    # Core build tools
    echo -e "${YELLOW}Checking core build tools...${NC}"
    local core_tools=("gcc" "clang" "make" "git" "pkg-config")
    for tool in "${core_tools[@]}"; do
        if command_exists "$tool"; then
            echo -e "  ✓ $tool: $(command -v "$tool")"
        else
            echo -e "  ✗ $tool: missing"
            MISSING_CORE_TOOLS+=("$tool")
        fi
    done
    
    # Cross-compilation tools
    echo -e "${YELLOW}Checking cross-compilation tools...${NC}"
    local cross_tools=("arm-linux-gnueabihf-gcc" "aarch64-linux-gnu-gcc" "arm-none-eabi-gcc")
    for tool in "${cross_tools[@]}"; do
        if command_exists "$tool"; then
            echo -e "  ✓ $tool: $(command -v "$tool")"
        else
            echo -e "  ✗ $tool: missing"
            MISSING_CROSS_TOOLS+=("$tool")
        fi
    done
    
    # Testing and debugging tools
    echo -e "${YELLOW}Checking testing and debugging tools...${NC}"
    local test_tools=("valgrind" "gdb" "strace")
    for tool in "${test_tools[@]}"; do
        if command_exists "$tool"; then
            echo -e "  ✓ $tool: $(command -v "$tool")"
        else
            echo -e "  ✗ $tool: missing"
            MISSING_TEST_TOOLS+=("$tool")
        fi
    done
    
    # Static analysis tools
    echo -e "${YELLOW}Checking static analysis tools...${NC}"
    local analysis_tools=("cppcheck" "clang-tidy")
    for tool in "${analysis_tools[@]}"; do
        if command_exists "$tool"; then
            echo -e "  ✓ $tool: $(command -v "$tool")"
        else
            echo -e "  ✗ $tool: missing"
            MISSING_ANALYSIS_TOOLS+=("$tool")
        fi
    done
    
    # Documentation tools
    echo -e "${YELLOW}Checking documentation tools...${NC}"
    local doc_tools=("doxygen" "dot")
    for tool in "${doc_tools[@]}"; do
        if command_exists "$tool"; then
            echo -e "  ✓ $tool: $(command -v "$tool")"
        else
            echo -e "  ✗ $tool: missing"
            MISSING_DOC_TOOLS+=("$tool")
        fi
    done
    
    # Emulation tools
    echo -e "${YELLOW}Checking emulation tools...${NC}"
    local emulation_tools=("qemu-system-x86_64" "qemu-arm")
    for tool in "${emulation_tools[@]}"; do
        if command_exists "$tool"; then
            echo -e "  ✓ $tool: $(command -v "$tool")"
        else
            echo -e "  ✗ $tool: missing"
            MISSING_EMULATION+=("$tool")
        fi
    done
    
    # Check Unicorn Engine
    echo -e "${YELLOW}Checking Unicorn Engine...${NC}"
    if pkg-config --exists unicorn 2>/dev/null; then
        echo -e "  ✓ unicorn: $(pkg-config --modversion unicorn)"
    elif ldconfig -p | grep -q libunicorn 2>/dev/null; then
        echo -e "  ✓ unicorn: installed (system library)"
    else
        echo -e "  ✗ unicorn: missing"
        MISSING_LIBRARIES+=("unicorn")
    fi
    
    echo ""
}

# Install packages for Ubuntu/Debian
# Install packages for Ubuntu/Debian
install_ubuntu() {
    echo -e "${BLUE}Installing packages for Ubuntu/Debian...${NC}"
    
    # Update package list
    if ask_confirmation "Update package list?"; then
        echo -e "${YELLOW}Updating package list...${NC}"
        if ! sudo apt update; then
            INSTALL_WARNINGS+=("Failed to update package list")
            echo -e "${YELLOW}⚠ Warning: Failed to update package list${NC}"
        fi
    fi
    
    # Core build tools
    if [[ ${#MISSING_CORE_TOOLS[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing core tools: ${MISSING_CORE_TOOLS[*]}${NC}"
        if ask_confirmation "Install core build tools (build-essential, gcc, clang, make, git, etc.)?"; then
            
            # Install essential tools first
            echo -e "${BLUE}Installing essential build tools...${NC}"
            safe_install "apt" "build-essential" "make" "git" "pkg-config" "autoconf" "automake" "libtool"
            
            # Detect and install available GCC versions
            echo -e "${BLUE}Installing GCC compilers...${NC}"
            local gcc_versions=($(detect_gcc_versions "ubuntu"))
            if [[ ${#gcc_versions[@]} -gt 0 ]]; then
                safe_install "apt" "${gcc_versions[@]}"
            else
                INSTALL_WARNINGS+=("No GCC versions detected")
            fi
            
            # Detect and install available Clang versions
            echo -e "${BLUE}Installing Clang compilers...${NC}"
            local clang_versions=($(detect_clang_versions "ubuntu"))
            if [[ ${#clang_versions[@]} -gt 0 ]]; then
                # Try to install clang versions, but don't fail if some are unavailable
                for clang_ver in "${clang_versions[@]}"; do
                    if ! safe_install "apt" "$clang_ver"; then
                        echo -e "${YELLOW}Continuing with next clang version...${NC}"
                    fi
                done
                
                # Try to install clang-tools if available
                if apt-cache show "clang-tools" &>/dev/null; then
                    safe_install "apt" "clang-tools"
                fi
            else
                INSTALL_WARNINGS+=("No Clang versions detected")
                echo -e "${YELLOW}⚠ Warning: No Clang versions available in repositories${NC}"
            fi
        fi
    else
        echo -e "${GREEN}All core build tools are already installed${NC}"
    fi
    
    # Cross-compilation toolchains
    if [[ ${#MISSING_CROSS_TOOLS[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing cross-compilation tools: ${MISSING_CROSS_TOOLS[*]}${NC}"
        if ask_confirmation "Install cross-compilation toolchains (ARM, RISC-V, etc.)?"; then
            echo -e "${BLUE}Installing cross-compilation toolchains...${NC}"
            
            # List of cross-compilation packages to try
            local cross_packages=(
                "gcc-arm-linux-gnueabihf"
                "gcc-aarch64-linux-gnu"
                "gcc-arm-none-eabi"
                "gdb-arm-none-eabi"
                "gdb-multiarch"
            )
            
            # Try RISC-V if available (newer Ubuntu versions)
            if apt-cache show "gcc-riscv64-linux-gnu" &>/dev/null; then
                cross_packages+=("gcc-riscv64-linux-gnu")
            fi
            
            # Install cross-compilation packages individually
            for package in "${cross_packages[@]}"; do
                if apt-cache show "$package" &>/dev/null; then
                    safe_install "apt" "$package"
                else
                    INSTALL_WARNINGS+=("Package $package not available")
                    echo -e "${YELLOW}⚠ Package $package not available in this Ubuntu version${NC}"
                fi
            done
        fi
    else
        echo -e "${GREEN}Cross-compilation tools are already installed${NC}"
    fi
    
    # Testing and debugging tools
    if [[ ${#MISSING_TEST_TOOLS[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing testing tools: ${MISSING_TEST_TOOLS[*]}${NC}"
        if ask_confirmation "Install testing and debugging tools (valgrind, gdb, strace)?"; then
            safe_install "apt" "valgrind" "gdb" "strace" "ltrace"
        fi
    else
        echo -e "${GREEN}Testing and debugging tools are already installed${NC}"
    fi
    
    # Static analysis tools
    if [[ ${#MISSING_ANALYSIS_TOOLS[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing analysis tools: ${MISSING_ANALYSIS_TOOLS[*]}${NC}"
        if ask_confirmation "Install static analysis tools (cppcheck, vera++, splint)?"; then
            local analysis_packages=("cppcheck")
            
            # Check for optional analysis tools
            for package in "vera++" "splint"; do
                if apt-cache show "$package" &>/dev/null; then
                    analysis_packages+=("$package")
                else
                    INSTALL_WARNINGS+=("Package $package not available")
                fi
            done
            
            safe_install "apt" "${analysis_packages[@]}"
        fi
    else
        echo -e "${GREEN}Static analysis tools are already installed${NC}"
    fi
    
    # Code coverage tools
    if ask_confirmation "Install code coverage tools (lcov, gcovr)?"; then
        safe_install "apt" "lcov" "gcovr"
    fi
    
    # Documentation tools
    if [[ ${#MISSING_DOC_TOOLS[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing documentation tools: ${MISSING_DOC_TOOLS[*]}${NC}"
        if ask_confirmation "Install documentation tools (doxygen, graphviz, LaTeX)?"; then
            safe_install "apt" "doxygen" "graphviz"
            
            if ask_confirmation "Install LaTeX packages (large download)?"; then
                safe_install "apt" "texlive-latex-base" "texlive-fonts-recommended"
            fi
        fi
    else
        echo -e "${GREEN}Documentation tools are already installed${NC}"
    fi
    
    # Libraries
    if ask_confirmation "Install development libraries (zlib, lz4, lzma, zstd)?"; then
        safe_install "apt" "zlib1g-dev" "liblz4-dev" "liblzma-dev" "libzstd-dev"
    fi
    
    # Emulation tools
    if [[ ${#MISSING_EMULATION[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing emulation tools: ${MISSING_EMULATION[*]}${NC}"
        if ask_confirmation "Install emulation tools (QEMU)?"; then
            safe_install "apt" "qemu-system" "qemu-user" "qemu-utils"
        fi
    else
        echo -e "${GREEN}Emulation tools are already installed${NC}"
    fi
    
    # Unicorn Engine
    if [[ " ${MISSING_LIBRARIES[*]} " =~ " unicorn " ]]; then
        echo -e "${YELLOW}Unicorn Engine is missing${NC}"
        if apt-cache show libunicorn-dev &> /dev/null; then
            if ask_confirmation "Install Unicorn Engine from package?"; then
                safe_install "apt" "libunicorn-dev"
            fi
        else
            if ask_confirmation "Unicorn Engine package not available. Install from source?"; then
                install_unicorn_from_source
            fi
        fi
    else
        echo -e "${GREEN}Unicorn Engine is already installed${NC}"
    fi
}

# Install packages for Fedora/RHEL
install_fedora() {
    echo -e "${BLUE}Installing packages for Fedora/RHEL...${NC}"
    
    # Core build tools
    if [[ ${#MISSING_CORE_TOOLS[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing core tools: ${MISSING_CORE_TOOLS[*]}${NC}"
        if ask_confirmation "Install Development Tools group and core compilers?"; then
            echo -e "${BLUE}Installing Development Tools group...${NC}"
            if ! sudo dnf groupinstall -y "Development Tools" 2>/dev/null; then
                INSTALL_WARNINGS+=("Failed to install Development Tools group")
                echo -e "${YELLOW}⚠ Warning: Failed to install Development Tools group${NC}"
            fi
            
            # Install individual packages
            local core_packages=("gcc" "gcc-c++" "make" "git" "pkgconfig" "autoconf" "automake" "libtool")
            safe_install "dnf" "${core_packages[@]}"
            
            # Try to install clang
            local clang_packages=("clang")
            if dnf list available "clang-tools-extra" &>/dev/null; then
                clang_packages+=("clang-tools-extra")
            fi
            safe_install "dnf" "${clang_packages[@]}"
        fi
    else
        echo -e "${GREEN}All core build tools are already installed${NC}"
    fi
    
    # Testing and debugging
    if [[ ${#MISSING_TEST_TOOLS[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing testing tools: ${MISSING_TEST_TOOLS[*]}${NC}"
        if ask_confirmation "Install testing and debugging tools?"; then
            safe_install "dnf" "valgrind" "gdb" "strace" "ltrace"
        fi
    else
        echo -e "${GREEN}Testing and debugging tools are already installed${NC}"
    fi
    
    # Static analysis
    if [[ ${#MISSING_ANALYSIS_TOOLS[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing analysis tools: ${MISSING_ANALYSIS_TOOLS[*]}${NC}"
        if ask_confirmation "Install static analysis tools?"; then
            safe_install "dnf" "cppcheck"
        fi
    else
        echo -e "${GREEN}Static analysis tools are already installed${NC}"
    fi
    
    # Coverage tools
    if ask_confirmation "Install coverage tools?"; then
        safe_install "dnf" "lcov" "gcovr"
    fi
    
    # Documentation
    if [[ ${#MISSING_DOC_TOOLS[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing documentation tools: ${MISSING_DOC_TOOLS[*]}${NC}"
        if ask_confirmation "Install documentation tools?"; then
            safe_install "dnf" "doxygen" "graphviz"
        fi
    else
        echo -e "${GREEN}Documentation tools are already installed${NC}"
    fi
    
    # Libraries
    if ask_confirmation "Install development libraries?"; then
        safe_install "dnf" "zlib-devel" "lz4-devel" "xz-devel" "libzstd-devel"
    fi
    
    # Emulation
    if [[ ${#MISSING_EMULATION[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing emulation tools: ${MISSING_EMULATION[*]}${NC}"
        if ask_confirmation "Install emulation tools?"; then
            safe_install "dnf" "qemu" "qemu-user"
        fi
    else
        echo -e "${GREEN}Emulation tools are already installed${NC}"
    fi
    
    # Cross-compilation (may require EPEL)
    if [[ ${#MISSING_CROSS_TOOLS[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing cross-compilation tools: ${MISSING_CROSS_TOOLS[*]}${NC}"
        if ask_confirmation "Install cross-compilation tools (may require EPEL)?"; then
            local cross_packages=()
            
            # Check for available cross-compilation packages
            for package in "gcc-arm-linux-gnu" "gcc-aarch64-linux-gnu"; do
                if dnf list available "$package" &>/dev/null; then
                    cross_packages+=("$package")
                else
                    INSTALL_WARNINGS+=("Cross-compilation package $package not available")
                fi
            done
            
            if [[ ${#cross_packages[@]} -gt 0 ]]; then
                safe_install "dnf" "${cross_packages[@]}"
            else
                echo -e "${YELLOW}⚠ No cross-compilation packages available. You may need to enable EPEL repository.${NC}"
                INSTALL_WARNINGS+=("No cross-compilation tools available")
            fi
        fi
    else
        echo -e "${GREEN}Cross-compilation tools are already installed${NC}"
    fi
    
    # Install Unicorn Engine from source
    if [[ " ${MISSING_LIBRARIES[*]} " =~ " unicorn " ]]; then
        if ask_confirmation "Install Unicorn Engine from source?"; then
            install_unicorn_from_source
        fi
    else
        echo -e "${GREEN}Unicorn Engine is already installed${NC}"
    fi
}

# Install packages for macOS
install_macos() {
    echo -e "${BLUE}Installing packages for macOS...${NC}"
    
    # Check if Homebrew is installed
    if ! command -v brew &> /dev/null; then
        if ask_confirmation "Homebrew is not installed. Install it?"; then
            echo -e "${YELLOW}Installing Homebrew...${NC}"
            if ! /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"; then
                INSTALL_ERRORS+=("Failed to install Homebrew")
                echo -e "${RED}Failed to install Homebrew${NC}"
                return 1
            fi
        else
            echo -e "${RED}Homebrew is required for macOS package management${NC}"
            INSTALL_ERRORS+=("Homebrew installation declined")
            return 1
        fi
    fi
    
    # Core development tools
    if [[ ${#MISSING_CORE_TOOLS[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing core tools: ${MISSING_CORE_TOOLS[*]}${NC}"
        if ask_confirmation "Install core development tools (gcc, llvm, make, git)?"; then
            safe_install "brew" "gcc" "llvm" "make" "git" "pkg-config" "autoconf" "automake" "libtool"
        fi
    else
        echo -e "${GREEN}All core development tools are already installed${NC}"
    fi
    
    # Testing and debugging
    if [[ ${#MISSING_TEST_TOOLS[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing testing tools: ${MISSING_TEST_TOOLS[*]}${NC}"
        if ask_confirmation "Install debugging tools (lldb)?"; then
            # lldb is usually included with Xcode command line tools
            if ! command_exists "lldb"; then
                safe_install "brew" "lldb"
            fi
        fi
    else
        echo -e "${GREEN}Testing and debugging tools are already installed${NC}"
    fi
    
    # Static analysis
    if [[ ${#MISSING_ANALYSIS_TOOLS[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing analysis tools: ${MISSING_ANALYSIS_TOOLS[*]}${NC}"
        if ask_confirmation "Install static analysis tools?"; then
            safe_install "brew" "cppcheck"
        fi
    else
        echo -e "${GREEN}Static analysis tools are already installed${NC}"
    fi
    
    # Coverage tools
    if ask_confirmation "Install coverage tools?"; then
        safe_install "brew" "lcov" "gcovr"
    fi
    
    # Documentation
    if [[ ${#MISSING_DOC_TOOLS[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing documentation tools: ${MISSING_DOC_TOOLS[*]}${NC}"
        if ask_confirmation "Install documentation tools?"; then
            safe_install "brew" "doxygen" "graphviz"
        fi
    else
        echo -e "${GREEN}Documentation tools are already installed${NC}"
    fi
    
    # Libraries
    if ask_confirmation "Install development libraries?"; then
        safe_install "brew" "zlib" "lz4" "xz" "zstd"
    fi
    
    # Emulation
    if [[ ${#MISSING_EMULATION[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing emulation tools: ${MISSING_EMULATION[*]}${NC}"
        if ask_confirmation "Install emulation tools?"; then
            safe_install "brew" "qemu"
        fi
    else
        echo -e "${GREEN}Emulation tools are already installed${NC}"
    fi
    
    # Cross-compilation
    if [[ ${#MISSING_CROSS_TOOLS[@]} -gt 0 ]]; then
        echo -e "${YELLOW}Missing cross-compilation tools: ${MISSING_CROSS_TOOLS[*]}${NC}"
        if ask_confirmation "Install cross-compilation tools?"; then
            # Check what's available and install
            local cross_packages=()
            for package in "arm-none-eabi-gcc" "riscv64-elf-gcc"; do
                if brew search "$package" | grep -q "^$package\$"; then
                    cross_packages+=("$package")
                else
                    INSTALL_WARNINGS+=("Cross-compilation package $package not available")
                fi
            done
            
            if [[ ${#cross_packages[@]} -gt 0 ]]; then
                safe_install "brew" "${cross_packages[@]}"
            fi
        fi
    else
        echo -e "${GREEN}Cross-compilation tools are already installed${NC}"
    fi
    
    # Install Unicorn Engine
    if [[ " ${MISSING_LIBRARIES[*]} " =~ " unicorn " ]]; then
        if ask_confirmation "Install Unicorn Engine?"; then
            if ! safe_install "brew" "unicorn"; then
                if ask_confirmation "Homebrew installation failed. Install from source?"; then
                    install_unicorn_from_source
                fi
            fi
        fi
    else
        echo -e "${GREEN}Unicorn Engine is already installed${NC}"
    fi
}

# Install Unicorn Engine from source
install_unicorn_from_source() {
    echo -e "${YELLOW}Installing Unicorn Engine from source...${NC}"
    
    # Check prerequisites
    local missing_prereqs=()
    for tool in git cmake make; do
        if ! command_exists "$tool"; then
            missing_prereqs+=("$tool")
        fi
    done
    
    if [[ ${#missing_prereqs[@]} -gt 0 ]]; then
        echo -e "${RED}Missing prerequisites for source installation: ${missing_prereqs[*]}${NC}"
        INSTALL_ERRORS+=("Cannot install Unicorn Engine: missing prerequisites")
        return 1
    fi
    
    TEMP_DIR=$(mktemp -d)
    local original_dir="$PWD"
    
    # Install with error handling
    (
        cd "$TEMP_DIR" || exit 1
        
        echo -e "${BLUE}Cloning Unicorn Engine repository...${NC}"
        if ! git clone https://github.com/unicorn-engine/unicorn.git; then
            echo -e "${RED}Failed to clone Unicorn Engine repository${NC}"
            exit 1
        fi
        
        cd unicorn || exit 1
        
        echo -e "${BLUE}Configuring build...${NC}"
        if ! mkdir build || ! cd build; then
            echo -e "${RED}Failed to create build directory${NC}"
            exit 1
        fi
        
        if ! cmake .. -DCMAKE_BUILD_TYPE=Release; then
            echo -e "${RED}CMake configuration failed${NC}"
            exit 1
        fi
        
        echo -e "${BLUE}Building Unicorn Engine...${NC}"
        local cpu_count=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
        if ! make -j"$cpu_count"; then
            echo -e "${RED}Build failed${NC}"
            exit 1
        fi
        
        echo -e "${BLUE}Installing Unicorn Engine...${NC}"
        if ! sudo make install; then
            echo -e "${RED}Installation failed${NC}"
            exit 1
        fi
        
        # Update library cache (Linux only)
        if command -v ldconfig &> /dev/null; then
            sudo ldconfig
        fi
        
    ) || {
        local exit_code=$?
        cd "$original_dir"
        rm -rf "$TEMP_DIR"
        INSTALL_ERRORS+=("Unicorn Engine source installation failed")
        echo -e "${RED}Unicorn Engine installation failed${NC}"
        return $exit_code
    }
    
    cd "$original_dir"
    rm -rf "$TEMP_DIR"
    
    echo -e "${GREEN}Unicorn Engine installed from source${NC}"
    return 0
}

# Set up IDE configurations
setup_ide() {
    echo -e "${BLUE}Setting up IDE configurations...${NC}"
    
    # VS Code settings
    if command -v code &> /dev/null; then
        echo -e "${YELLOW}Setting up VS Code...${NC}"
        
        # Create .vscode directory
        mkdir -p .vscode
        
        # Create settings.json
        cat > .vscode/settings.json << 'EOF'
{
    "C_Cpp.default.cStandard": "c99",
    "C_Cpp.default.cppStandard": "c++17",
    "C_Cpp.default.compilerPath": "/usr/bin/gcc",
    "C_Cpp.default.intelliSenseMode": "gcc-x64",
    "C_Cpp.default.includePath": [
        "${workspaceFolder}/src/common/include",
        "${workspaceFolder}/src/stld/include",
        "${workspaceFolder}/src/star/include",
        "${workspaceFolder}/tests/unity"
    ],
    "C_Cpp.default.defines": [
        "_GNU_SOURCE",
        "_POSIX_C_SOURCE=200112L"
    ],
    "files.associations": {
        "*.h": "c",
        "*.c": "c"
    },
    "editor.insertSpaces": true,
    "editor.tabSize": 4,
    "editor.detectIndentation": false,
    "C_Cpp.clang_format_fallbackStyle": "{ BasedOnStyle: LLVM, IndentWidth: 4, ColumnLimit: 80 }"
}
EOF

        # Create tasks.json
        cat > .vscode/tasks.json << 'EOF'
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build All",
            "type": "shell",
            "command": "make",
            "args": ["all"],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "Run Tests",
            "type": "shell",
            "command": "make",
            "args": ["tests"],
            "group": "test"
        },
        {
            "label": "Code Coverage",
            "type": "shell",
            "command": "make",
            "args": ["coverage"],
            "group": "test"
        },
        {
            "label": "Static Analysis",
            "type": "shell",
            "command": "make",
            "args": ["static-analysis"],
            "group": "test"
        },
        {
            "label": "Clean",
            "type": "shell",
            "command": "make",
            "args": ["clean"],
            "group": "build"
        }
    ]
}
EOF

        # Create launch.json for debugging
        cat > .vscode/launch.json << 'EOF'
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug STLD",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/stld",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        },
        {
            "name": "Debug STAR",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/star",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb"
        }
    ]
}
EOF

        echo -e "${GREEN}VS Code configuration created${NC}"
    fi
    
    # Git hooks
    echo -e "${YELLOW}Setting up Git hooks...${NC}"
    mkdir -p .git/hooks
    
    # Pre-commit hook for code formatting
    cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash
# Pre-commit hook for STLD/STAR project

echo "Running pre-commit checks..."

# Check if Makefile exists and has the required targets
if [ -f "Makefile" ]; then
    # Check if the check-c99 target exists
    if make -n check-c99 &>/dev/null; then
        echo "Running C99 compliance check..."
        if ! make check-c99 &>/dev/null; then
            echo "Error: C99 compliance check failed"
            exit 1
        fi
        echo "✓ C99 compliance check passed"
    else
        echo "⚠ Warning: C99 compliance check target not available"
    fi
    
    # Check if the project builds
    if make -n all &>/dev/null; then
        echo "Running build test..."
        # Check if there are actually source files to build
        if find src -name "*.c" -type f | grep -q .; then
            if ! make -j$(nproc 2>/dev/null || echo 4) all &>/dev/null; then
                echo "Error: Build failed"
                exit 1
            fi
            echo "✓ Build test passed"
        else
            echo "⚠ No source files found, skipping build test"
        fi
    fi
else
    echo "⚠ Warning: Makefile not found, skipping build checks"
fi

# Run static analysis on changed files if available
if command -v cppcheck &> /dev/null; then
    echo "Running static analysis on changed files..."
    changed_files=$(git diff --cached --name-only --diff-filter=ACM | grep '\.[ch]$' || true)
    if [ -n "$changed_files" ]; then
        echo "Analyzing: $changed_files"
        if ! echo "$changed_files" | xargs -r cppcheck --error-exitcode=1 --quiet 2>/dev/null; then
            echo "⚠ Warning: Static analysis found issues (not blocking commit)"
        else
            echo "✓ Static analysis passed"
        fi
    else
        echo "No C/C++ files changed"
    fi
else
    echo "⚠ cppcheck not available, skipping static analysis"
fi

# Check for basic C99 syntax in changed files
changed_c_files=$(git diff --cached --name-only --diff-filter=ACM | grep '\.c$' || true)
if [ -n "$changed_c_files" ]; then
    echo "Checking C99 syntax in changed files..."
    for file in $changed_c_files; do
        if command -v gcc &> /dev/null; then
            if ! gcc -std=c99 -fsyntax-only -Wall -Wextra "$file" 2>/dev/null; then
                echo "⚠ Warning: Syntax issues found in $file"
            fi
        fi
    done
fi

echo "✓ Pre-commit checks completed"
exit 0
EOF

    chmod +x .git/hooks/pre-commit
    
    echo -e "${GREEN}Git hooks configured${NC}"
}

# Create development scripts
create_dev_scripts() {
    echo -e "${BLUE}Creating development scripts...${NC}"
    
    # Quick build script
    cat > build.sh << 'EOF'
#!/bin/bash
# Quick build script for STLD/STAR

set -e

echo "Building STLD/STAR..."

# Clean build
make clean

# Build with all cores
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) all

echo "Build complete!"
echo "Run './test.sh' to run tests"
EOF

    chmod +x build.sh
    
    # Quick test script
    cat > test.sh << 'EOF'
#!/bin/bash
# Quick test script for STLD/STAR

set -e

echo "Running STLD/STAR tests..."

# Run tests
make tests

# Generate coverage if available
if command -v lcov &> /dev/null; then
    echo "Generating coverage report..."
    make coverage
    echo "Coverage report: coverage/html/index.html"
fi

echo "Tests complete!"
EOF

    chmod +x test.sh
    
    # Development environment info script
    cat > dev-status.sh << 'EOF'
#!/bin/bash
# Development environment status

echo "STLD/STAR Development Status"
echo "============================"
echo ""

echo "Git status:"
git status --short || echo "Not a git repository"
echo ""

echo "Build status:"
if [ -f "build/stld" ]; then
    echo "✓ STLD binary exists"
else
    echo "✗ STLD binary missing"
fi

if [ -f "build/star" ]; then
    echo "✓ STAR binary exists"
else
    echo "✗ STAR binary missing"
fi
echo ""

echo "Test results:"
if [ -f "build/test_runner" ]; then
    echo "✓ Test runner exists"
else
    echo "✗ Test runner missing"
fi
echo ""

echo "Recent changes:"
git log --oneline -5 2>/dev/null || echo "No git history"
EOF

    chmod +x dev-status.sh
    
    echo -e "${GREEN}Development scripts created${NC}"
}

# Main installation function
main() {
    echo "Detected OS: $OS"
    echo ""
    
    # First, check what's already installed
    check_tools
    
    # Calculate totals
    local total_missing=$((${#MISSING_CORE_TOOLS[@]} + ${#MISSING_CROSS_TOOLS[@]} + ${#MISSING_TEST_TOOLS[@]} + ${#MISSING_ANALYSIS_TOOLS[@]} + ${#MISSING_DOC_TOOLS[@]} + ${#MISSING_LIBRARIES[@]} + ${#MISSING_EMULATION[@]}))
    
    if [[ $total_missing -eq 0 ]]; then
        echo -e "${GREEN}All required tools appear to be installed!${NC}"
        echo ""
        if ask_confirmation "Would you like to proceed with IDE setup and development scripts anyway?" "n"; then
            setup_ide
            create_dev_scripts
        fi
        
        if ask_confirmation "Run environment verification?" "y"; then
            if [ -f "$SCRIPT_DIR/verify_environment.sh" ]; then
                "$SCRIPT_DIR/verify_environment.sh"
            else
                echo -e "${YELLOW}Environment verification script not found${NC}"
            fi
        fi
        return 0
    fi
    
    echo -e "${YELLOW}Found $total_missing missing tools/libraries${NC}"
    echo ""
    
    if [[ "$INTERACTIVE" == "true" ]]; then
        echo "The following categories have missing tools:"
        [[ ${#MISSING_CORE_TOOLS[@]} -gt 0 ]] && echo "  - Core build tools (${#MISSING_CORE_TOOLS[@]} missing)"
        [[ ${#MISSING_CROSS_TOOLS[@]} -gt 0 ]] && echo "  - Cross-compilation tools (${#MISSING_CROSS_TOOLS[@]} missing)"
        [[ ${#MISSING_TEST_TOOLS[@]} -gt 0 ]] && echo "  - Testing/debugging tools (${#MISSING_TEST_TOOLS[@]} missing)"
        [[ ${#MISSING_ANALYSIS_TOOLS[@]} -gt 0 ]] && echo "  - Static analysis tools (${#MISSING_ANALYSIS_TOOLS[@]} missing)"
        [[ ${#MISSING_DOC_TOOLS[@]} -gt 0 ]] && echo "  - Documentation tools (${#MISSING_DOC_TOOLS[@]} missing)"
        [[ ${#MISSING_LIBRARIES[@]} -gt 0 ]] && echo "  - Libraries (${#MISSING_LIBRARIES[@]} missing)"
        [[ ${#MISSING_EMULATION[@]} -gt 0 ]] && echo "  - Emulation tools (${#MISSING_EMULATION[@]} missing)"
        echo ""
        
        if ! ask_confirmation "Proceed with installation?"; then
            echo "Installation cancelled by user."
            exit 0
        fi
    fi
    
    case $OS in
        ubuntu)
            install_ubuntu
            ;;
        fedora|rhel)
            install_fedora
            ;;
        macos)
            install_macos
            ;;
        *)
            echo -e "${RED}Unsupported operating system: $OS${NC}"
            echo "Please install the required tools manually."
            echo "See README.md for detailed instructions."
            exit 1
            ;;
    esac
    
    echo ""
    echo -e "${GREEN}Package installation complete!${NC}"
    
    # Show summary of errors and warnings
    if [[ ${#INSTALL_ERRORS[@]} -gt 0 ]] || [[ ${#INSTALL_WARNINGS[@]} -gt 0 ]]; then
        echo ""
        echo -e "${YELLOW}Installation Summary:${NC}"
        
        if [[ ${#INSTALL_ERRORS[@]} -gt 0 ]]; then
            echo -e "${RED}Errors encountered:${NC}"
            for error in "${INSTALL_ERRORS[@]}"; do
                echo -e "  ${RED}✗ $error${NC}"
            done
        fi
        
        if [[ ${#INSTALL_WARNINGS[@]} -gt 0 ]]; then
            echo -e "${YELLOW}Warnings:${NC}"
            for warning in "${INSTALL_WARNINGS[@]}"; do
                echo -e "  ${YELLOW}⚠ $warning${NC}"
            done
        fi
        
        echo ""
        echo -e "${BLUE}Note: Some packages may not be available in your distribution version.${NC}"
        echo -e "${BLUE}The core functionality should still work with the successfully installed tools.${NC}"
    fi
    echo ""
    
    # Set up IDE and development environment
    if ask_confirmation "Set up IDE configurations and development scripts?"; then
        setup_ide
        create_dev_scripts
    fi
    
    echo ""
    if ask_confirmation "Run environment verification?"; then
        echo -e "${BLUE}Running environment verification...${NC}"
        if [ -f "$SCRIPT_DIR/verify_environment.sh" ]; then
            "$SCRIPT_DIR/verify_environment.sh"
        else
            echo -e "${YELLOW}Environment verification script not found${NC}"
        fi
    fi
    
    echo ""
    echo -e "${GREEN}Development environment setup complete!${NC}"
    echo ""
    echo "Next steps:"
    echo "  1. Run './build.sh' to build the project"
    echo "  2. Run './test.sh' to run tests"
    echo "  3. Run './dev-status.sh' to check development status"
    echo "  4. Open the project in your preferred IDE"
    echo ""
    echo "For Docker-based development:"
    echo "  docker-compose up stld-dev"
    echo ""
}

# Show help message
show_help() {
    echo "STLD/STAR Development Environment Setup Script"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "OPTIONS:"
    echo "  --auto, -y    Run in non-interactive mode (auto-approve all installations)"
    echo "  --help, -h    Show this help message"
    echo ""
    echo "This script will:"
    echo "  1. Check for installed development tools"
    echo "  2. Ask for confirmation before installing missing tools"
    echo "  3. Set up IDE configurations (VS Code)"
    echo "  4. Create development helper scripts"
    echo "  5. Run environment verification"
    echo ""
    echo "Examples:"
    echo "  $0              # Interactive mode (default)"
    echo "  $0 --auto       # Non-interactive mode"
    echo "  $0 -y           # Non-interactive mode (short form)"
    echo ""
}

# Parse command line arguments
case "$1" in
    --help|-h)
        show_help
        exit 0
        ;;
    --auto|-y)
        # Already handled in global variable section
        ;;
    *)
        if [[ -n "$1" ]]; then
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
        fi
        ;;
esac

# Detect OS and run main function
detect_os
main "$@"
