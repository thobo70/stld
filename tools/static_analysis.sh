#!/bin/bash

/**
 * @file static_analysis.sh
 * @brief Static analysis script for STLD/STAR project
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * Comprehensive static analysis using multiple tools including
 * cppcheck, clang-static-analyzer, and optional commercial tools.
 */

set -e

# Configuration
PROJECT_NAME="STLD/STAR"
SRC_DIR="src"
ANALYSIS_DIR="analysis"
BUILD_DIR="build"
TESTS_DIR="tests"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Print banner
print_banner() {
    echo
    echo "================================================================"
    echo "                  ${PROJECT_NAME} Static Analysis"
    echo "================================================================"
    echo
}

# Print usage
print_usage() {
    echo "Usage: $0 [options]"
    echo
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -c, --clean             Clean previous analysis results"
    echo "  -t, --tool              Run specific tool (cppcheck|clang|all)"
    echo "  -f, --format            Output format (html|xml|text)"
    echo "  -o, --output            Output directory (default: analysis)"
    echo "  --severity              Minimum severity level (error|warning|style|performance|portability)"
    echo "  --std                   C standard version (c89|c99|c11)"
    echo "  --platform              Target platform (unix32|unix64|win32|win64)"
    echo "  --ci                    CI mode - machine readable output"
    echo "  --fail-on-error         Exit with error code if issues found"
    echo "  --exclude               Exclude directory from analysis"
    echo
    echo "Examples:"
    echo "  $0                      # Run full static analysis"
    echo "  $0 -t cppcheck          # Run only cppcheck"
    echo "  $0 --ci                 # CI-friendly output"
    echo "  $0 --fail-on-error      # Fail build on static analysis errors"
}

# Parse command line arguments
parse_arguments() {
    CLEAN_ANALYSIS=false
    ANALYSIS_TOOL="all"
    OUTPUT_FORMAT="html"
    OUTPUT_DIR="${ANALYSIS_DIR}"
    SEVERITY_LEVEL="warning"
    C_STANDARD="c99"
    PLATFORM="unix32"
    CI_MODE=false
    FAIL_ON_ERROR=false
    EXCLUDE_DIRS=()
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                print_usage
                exit 0
                ;;
            -c|--clean)
                CLEAN_ANALYSIS=true
                shift
                ;;
            -t|--tool)
                ANALYSIS_TOOL="$2"
                shift 2
                ;;
            -f|--format)
                OUTPUT_FORMAT="$2"
                shift 2
                ;;
            -o|--output)
                OUTPUT_DIR="$2"
                shift 2
                ;;
            --severity)
                SEVERITY_LEVEL="$2"
                shift 2
                ;;
            --std)
                C_STANDARD="$2"
                shift 2
                ;;
            --platform)
                PLATFORM="$2"
                shift 2
                ;;
            --ci)
                CI_MODE=true
                shift
                ;;
            --fail-on-error)
                FAIL_ON_ERROR=true
                shift
                ;;
            --exclude)
                EXCLUDE_DIRS+=("$2")
                shift 2
                ;;
            *)
                log_error "Unknown option: $1"
                print_usage
                exit 1
                ;;
        esac
    done
}

# Check dependencies
check_dependencies() {
    local missing_deps=()
    local available_tools=()
    
    # Check for analysis tools
    if command -v cppcheck &> /dev/null; then
        available_tools+=("cppcheck")
    else
        case "$ANALYSIS_TOOL" in
            "cppcheck"|"all")
                missing_deps+=("cppcheck")
                ;;
        esac
    fi
    
    if command -v clang &> /dev/null && command -v scan-build &> /dev/null; then
        available_tools+=("clang-static-analyzer")
    else
        case "$ANALYSIS_TOOL" in
            "clang"|"all")
                if [[ "$ANALYSIS_TOOL" == "clang" ]]; then
                    missing_deps+=("clang-tools")
                else
                    log_warning "clang-static-analyzer not available - skipping"
                fi
                ;;
        esac
    fi
    
    # Optional tools
    if command -v splint &> /dev/null; then
        available_tools+=("splint")
    fi
    
    if command -v pc-lint &> /dev/null; then
        available_tools+=("pc-lint")
    fi
    
    if command -v pvs-studio &> /dev/null; then
        available_tools+=("pvs-studio")
    fi
    
    # Report available tools
    if [[ ${#available_tools[@]} -gt 0 ]]; then
        log_info "Available static analysis tools: ${available_tools[*]}"
    fi
    
    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        log_error "Missing required dependencies: ${missing_deps[*]}"
        echo
        echo "Install missing dependencies:"
        echo "  Ubuntu/Debian: sudo apt-get install cppcheck clang-tools"
        echo "  RHEL/CentOS:   sudo yum install cppcheck clang-analyzer"
        echo "  macOS:         brew install cppcheck llvm"
        exit 1
    fi
}

# Clean previous analysis
clean_analysis() {
    log_info "Cleaning previous analysis results..."
    
    rm -rf "${OUTPUT_DIR}"
    
    # Clean scan-build output
    rm -rf /tmp/scan-build-* 2>/dev/null || true
    
    log_success "Analysis results cleaned"
}

# Run cppcheck analysis
run_cppcheck() {
    log_info "Running cppcheck static analysis..."
    
    local output_file="${OUTPUT_DIR}/cppcheck"
    local enable_checks="all"
    
    # Adjust severity
    case "${SEVERITY_LEVEL}" in
        "error")
            enable_checks="error"
            ;;
        "warning")
            enable_checks="warning,error"
            ;;
        "style")
            enable_checks="style,warning,error"
            ;;
        "performance")
            enable_checks="performance,style,warning,error"
            ;;
        "portability")
            enable_checks="all"
            ;;
    esac
    
    # Build include arguments
    local include_args=()
    include_args+=("-I${SRC_DIR}/common/include")
    include_args+=("-I${SRC_DIR}/stld/include")
    include_args+=("-I${SRC_DIR}/star/include")
    
    # Build exclude arguments
    local exclude_args=()
    for dir in "${EXCLUDE_DIRS[@]}"; do
        exclude_args+=("-i${dir}")
    done
    
    # Default excludes
    exclude_args+=("-i${TESTS_DIR}")
    exclude_args+=("-iexternal")
    exclude_args+=("-iunity")
    
    # Run cppcheck
    mkdir -p "${OUTPUT_DIR}"
    
    if [[ "$OUTPUT_FORMAT" == "xml" ]] || [[ "$CI_MODE" == "true" ]]; then
        cppcheck --enable="${enable_checks}" \
                 --std="${C_STANDARD}" \
                 --platform="${PLATFORM}" \
                 --suppress=missingIncludeSystem \
                 --suppress=unusedFunction \
                 --inline-suppr \
                 --xml \
                 --xml-version=2 \
                 "${include_args[@]}" \
                 "${exclude_args[@]}" \
                 "${SRC_DIR}" 2> "${output_file}.xml"
        
        # Convert to HTML if requested
        if [[ "$OUTPUT_FORMAT" == "html" ]] && command -v cppcheck-htmlreport &> /dev/null; then
            cppcheck-htmlreport --file="${output_file}.xml" \
                               --report-dir="${output_file}-html" \
                               --source-dir="${SRC_DIR}"
            log_success "cppcheck HTML report: ${output_file}-html/index.html"
        fi
        
        log_success "cppcheck XML report: ${output_file}.xml"
    else
        cppcheck --enable="${enable_checks}" \
                 --std="${C_STANDARD}" \
                 --platform="${PLATFORM}" \
                 --suppress=missingIncludeSystem \
                 --suppress=unusedFunction \
                 --inline-suppr \
                 "${include_args[@]}" \
                 "${exclude_args[@]}" \
                 "${SRC_DIR}" 2> "${output_file}.txt"
        
        log_success "cppcheck text report: ${output_file}.txt"
    fi
    
    # Count issues
    local issue_count=0
    if [[ -f "${output_file}.xml" ]]; then
        issue_count=$(grep -c "<error " "${output_file}.xml" 2>/dev/null || echo "0")
    elif [[ -f "${output_file}.txt" ]]; then
        issue_count=$(grep -v "^$" "${output_file}.txt" | wc -l)
    fi
    
    if [[ "$issue_count" -gt 0 ]]; then
        log_warning "cppcheck found ${issue_count} issues"
        if [[ "$FAIL_ON_ERROR" == "true" ]]; then
            return 1
        fi
    else
        log_success "cppcheck found no issues"
    fi
    
    return 0
}

# Run clang static analyzer
run_clang_analyzer() {
    log_info "Running clang static analyzer..."
    
    local output_dir="${OUTPUT_DIR}/clang-analysis"
    
    # Clean build first
    make clean > /dev/null 2>&1 || true
    
    # Run scan-build
    if scan-build --use-analyzer="$(which clang)" \
                  --html-title="${PROJECT_NAME} Static Analysis" \
                  --output="${output_dir}" \
                  --status-bugs \
                  --keep-going \
                  -v \
                  make all; then
        log_success "clang-static-analyzer completed successfully"
        
        # Find the actual report directory
        local report_dir=$(find "${output_dir}" -name "20*" -type d | head -1)
        if [[ -n "$report_dir" ]]; then
            log_success "clang analyzer report: ${report_dir}/index.html"
        fi
        
        return 0
    else
        log_warning "clang-static-analyzer found issues"
        
        # Find the actual report directory
        local report_dir=$(find "${output_dir}" -name "20*" -type d | head -1)
        if [[ -n "$report_dir" ]]; then
            log_info "clang analyzer report: ${report_dir}/index.html"
        fi
        
        if [[ "$FAIL_ON_ERROR" == "true" ]]; then
            return 1
        fi
        return 0
    fi
}

# Run splint analysis (if available)
run_splint() {
    if ! command -v splint &> /dev/null; then
        return 0
    fi
    
    log_info "Running splint analysis..."
    
    local output_file="${OUTPUT_DIR}/splint.txt"
    
    # Run splint on source files
    find "${SRC_DIR}" -name "*.c" -exec splint \
        -I"${SRC_DIR}/common/include" \
        -I"${SRC_DIR}/stld/include" \
        -I"${SRC_DIR}/star/include" \
        +posixlib \
        -preproc \
        {} \; > "${output_file}" 2>&1 || true
    
    log_success "splint report: ${output_file}"
}

# Run PC-lint analysis (if available)
run_pc_lint() {
    if ! command -v pc-lint &> /dev/null; then
        return 0
    fi
    
    log_info "Running PC-lint analysis..."
    
    local output_file="${OUTPUT_DIR}/pc-lint.txt"
    
    # Create lint configuration
    cat > "${OUTPUT_DIR}/project.lnt" << EOF
// PC-lint configuration for ${PROJECT_NAME}
-i${SRC_DIR}/common/include
-i${SRC_DIR}/stld/include  
-i${SRC_DIR}/star/include
+libclass()
std.lnt
EOF
    
    # Run PC-lint
    find "${SRC_DIR}" -name "*.c" -exec pc-lint \
        "${OUTPUT_DIR}/project.lnt" \
        {} \; > "${output_file}" 2>&1 || true
    
    log_success "PC-lint report: ${output_file}"
}

# Run memory analysis with Valgrind
run_valgrind_analysis() {
    if ! command -v valgrind &> /dev/null; then
        return 0
    fi
    
    log_info "Running Valgrind memory analysis..."
    
    local output_file="${OUTPUT_DIR}/valgrind.xml"
    
    # Build test suite if not available
    if [[ ! -f "${BUILD_DIR}/test_runner" ]]; then
        make tests > /dev/null 2>&1 || return 0
    fi
    
    # Run Valgrind memcheck
    valgrind --tool=memcheck \
             --leak-check=full \
             --show-leak-kinds=all \
             --track-origins=yes \
             --xml=yes \
             --xml-file="${output_file}" \
             "${BUILD_DIR}/test_runner" > /dev/null 2>&1 || true
    
    log_success "Valgrind report: ${output_file}"
}

# Generate summary report
generate_summary() {
    log_info "Generating analysis summary..."
    
    local summary_file="${OUTPUT_DIR}/summary.txt"
    
    cat > "${summary_file}" << EOF
${PROJECT_NAME} Static Analysis Summary
Generated: $(date)

Analysis Tools Run:
EOF
    
    # Check which reports exist
    local total_issues=0
    
    if [[ -f "${OUTPUT_DIR}/cppcheck.xml" ]]; then
        local cppcheck_issues=$(grep -c "<error " "${OUTPUT_DIR}/cppcheck.xml" 2>/dev/null || echo "0")
        echo "  - cppcheck: ${cppcheck_issues} issues" >> "${summary_file}"
        total_issues=$((total_issues + cppcheck_issues))
    elif [[ -f "${OUTPUT_DIR}/cppcheck.txt" ]]; then
        local cppcheck_issues=$(grep -v "^$" "${OUTPUT_DIR}/cppcheck.txt" | wc -l)
        echo "  - cppcheck: ${cppcheck_issues} issues" >> "${summary_file}"
        total_issues=$((total_issues + cppcheck_issues))
    fi
    
    if [[ -d "${OUTPUT_DIR}/clang-analysis" ]]; then
        local clang_reports=$(find "${OUTPUT_DIR}/clang-analysis" -name "*.html" | wc -l)
        echo "  - clang-static-analyzer: ${clang_reports} reports generated" >> "${summary_file}"
    fi
    
    if [[ -f "${OUTPUT_DIR}/splint.txt" ]]; then
        echo "  - splint: report available" >> "${summary_file}"
    fi
    
    if [[ -f "${OUTPUT_DIR}/pc-lint.txt" ]]; then
        echo "  - PC-lint: report available" >> "${summary_file}"
    fi
    
    if [[ -f "${OUTPUT_DIR}/valgrind.xml" ]]; then
        echo "  - Valgrind: memory analysis complete" >> "${summary_file}"
    fi
    
    echo "" >> "${summary_file}"
    echo "Total Issues Found: ${total_issues}" >> "${summary_file}"
    
    # Display summary
    cat "${summary_file}"
    
    log_success "Analysis summary: ${summary_file}"
    
    # Return exit code based on issues found
    if [[ "$FAIL_ON_ERROR" == "true" ]] && [[ "$total_issues" -gt 0 ]]; then
        return 1
    fi
    
    return 0
}

# Generate CI output
generate_ci_output() {
    if [[ "$CI_MODE" != "true" ]]; then
        return
    fi
    
    local ci_file="${OUTPUT_DIR}/ci-results.json"
    
    # Count issues from various tools
    local cppcheck_issues=0
    if [[ -f "${OUTPUT_DIR}/cppcheck.xml" ]]; then
        cppcheck_issues=$(grep -c "<error " "${OUTPUT_DIR}/cppcheck.xml" 2>/dev/null || echo "0")
    fi
    
    # Generate JSON output for CI systems
    cat > "${ci_file}" << EOF
{
  "analysis_date": "$(date -u +"%Y-%m-%dT%H:%M:%SZ")",
  "project": "${PROJECT_NAME}",
  "tools": {
    "cppcheck": {
      "issues": ${cppcheck_issues},
      "report": "cppcheck.xml"
    }
  },
  "summary": {
    "total_issues": ${cppcheck_issues},
    "status": "$([[ $cppcheck_issues -eq 0 ]] && echo "passed" || echo "failed")"
  }
}
EOF
    
    log_success "CI results: ${ci_file}"
}

# Main execution
main() {
    print_banner
    parse_arguments "$@"
    check_dependencies
    
    if [[ "$CLEAN_ANALYSIS" == "true" ]]; then
        clean_analysis
    fi
    
    mkdir -p "${OUTPUT_DIR}"
    
    local analysis_failed=false
    
    # Run selected analysis tools
    case "${ANALYSIS_TOOL}" in
        "cppcheck")
            run_cppcheck || analysis_failed=true
            ;;
        "clang")
            run_clang_analyzer || analysis_failed=true
            ;;
        "all")
            run_cppcheck || analysis_failed=true
            run_clang_analyzer || analysis_failed=true
            run_splint
            run_pc_lint
            run_valgrind_analysis
            ;;
        *)
            log_error "Unknown analysis tool: ${ANALYSIS_TOOL}"
            exit 1
            ;;
    esac
    
    generate_summary || analysis_failed=true
    generate_ci_output
    
    if [[ "$analysis_failed" == "true" ]]; then
        log_error "Static analysis completed with issues!"
        exit 1
    else
        log_success "Static analysis completed successfully!"
    fi
}

# Run main function with all arguments
main "$@"
