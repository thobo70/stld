#!/bin/bash

/**
 * @file coverage.sh
 * @brief Code coverage analysis script for STLD/STAR project
 * @version 1.0.0
 * @date 2025-07-23
 * 
 * Comprehensive code coverage analysis using gcov/lcov with
 * threshold checking and CI/CD integration support.
 */

set -e

# Configuration
PROJECT_NAME="STLD/STAR"
COVERAGE_DIR="coverage"
BUILD_DIR="build"
SRC_DIR="src"
TESTS_DIR="tests"

# Coverage thresholds
FUNCTION_THRESHOLD=90
LINE_THRESHOLD=85
BRANCH_THRESHOLD=80

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
    echo "                    ${PROJECT_NAME} Coverage Analysis"
    echo "================================================================"
    echo
}

# Print usage
print_usage() {
    echo "Usage: $0 [options]"
    echo
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -c, --clean             Clean previous coverage data"
    echo "  -t, --tests             Run specific test suite (unit|integration|all)"
    echo "  -f, --format            Output format (html|xml|json|text)"
    echo "  -o, --output            Output directory (default: coverage)"
    echo "  --function-threshold    Function coverage threshold (default: 90)"
    echo "  --line-threshold        Line coverage threshold (default: 85)"
    echo "  --branch-threshold      Branch coverage threshold (default: 80)"
    echo "  --ci                    CI mode - machine readable output"
    echo "  --no-html               Skip HTML report generation"
    echo "  --exclude               Exclude pattern from coverage"
    echo
    echo "Examples:"
    echo "  $0                      # Generate full coverage report"
    echo "  $0 --clean              # Clean and regenerate coverage"
    echo "  $0 -t unit              # Coverage for unit tests only"
    echo "  $0 --ci                 # CI-friendly output format"
}

# Parse command line arguments
parse_arguments() {
    CLEAN_COVERAGE=false
    TEST_SUITE="all"
    OUTPUT_FORMAT="html"
    OUTPUT_DIR="${COVERAGE_DIR}"
    CI_MODE=false
    GENERATE_HTML=true
    EXCLUDE_PATTERNS=()
    
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                print_usage
                exit 0
                ;;
            -c|--clean)
                CLEAN_COVERAGE=true
                shift
                ;;
            -t|--tests)
                TEST_SUITE="$2"
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
            --function-threshold)
                FUNCTION_THRESHOLD="$2"
                shift 2
                ;;
            --line-threshold)
                LINE_THRESHOLD="$2"
                shift 2
                ;;
            --branch-threshold)
                BRANCH_THRESHOLD="$2"
                shift 2
                ;;
            --ci)
                CI_MODE=true
                shift
                ;;
            --no-html)
                GENERATE_HTML=false
                shift
                ;;
            --exclude)
                EXCLUDE_PATTERNS+=("$2")
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
    
    # Required tools
    local required_tools=("gcc" "gcov" "lcov" "genhtml" "make")
    
    for tool in "${required_tools[@]}"; do
        if ! command -v "$tool" &> /dev/null; then
            missing_deps+=("$tool")
        fi
    done
    
    # Optional tools
    if [[ "$OUTPUT_FORMAT" == "xml" ]] && ! command -v "gcovr" &> /dev/null; then
        log_warning "gcovr not found - XML output will not be available"
    fi
    
    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        log_error "Missing required dependencies: ${missing_deps[*]}"
        echo
        echo "Install missing dependencies:"
        echo "  Ubuntu/Debian: sudo apt-get install gcc gcov lcov"
        echo "  RHEL/CentOS:   sudo yum install gcc gcov lcov"
        echo "  macOS:         brew install lcov"
        exit 1
    fi
}

# Clean previous coverage data
clean_coverage() {
    log_info "Cleaning previous coverage data..."
    
    # Remove coverage files
    find . -name "*.gcov" -delete 2>/dev/null || true
    find . -name "*.gcda" -delete 2>/dev/null || true
    find . -name "*.gcno" -delete 2>/dev/null || true
    
    # Remove coverage directory
    rm -rf "${OUTPUT_DIR}"
    
    # Clean build artifacts with coverage info
    if [[ -d "${BUILD_DIR}" ]]; then
        find "${BUILD_DIR}" -name "*.gcov" -delete 2>/dev/null || true
        find "${BUILD_DIR}" -name "*.gcda" -delete 2>/dev/null || true
        find "${BUILD_DIR}" -name "*.gcno" -delete 2>/dev/null || true
    fi
    
    log_success "Coverage data cleaned"
}

# Build with coverage instrumentation
build_with_coverage() {
    log_info "Building with coverage instrumentation..."
    
    # Clean build first
    make clean > /dev/null 2>&1 || true
    
    # Build with coverage flags
    local coverage_flags="--coverage -fprofile-arcs -ftest-coverage -O0 -g"
    
    if ! make all CFLAGS="${coverage_flags}" LDFLAGS="--coverage" DEBUG=1; then
        log_error "Failed to build with coverage instrumentation"
        exit 1
    fi
    
    log_success "Build with coverage instrumentation completed"
}

# Run test suite
run_tests() {
    log_info "Running test suite: ${TEST_SUITE}"
    
    case "${TEST_SUITE}" in
        "unit")
            if ! make test-unit; then
                log_error "Unit tests failed"
                exit 1
            fi
            ;;
        "integration")
            if ! make test-integration; then
                log_error "Integration tests failed"
                exit 1
            fi
            ;;
        "all")
            if ! make test; then
                log_error "Test suite failed"
                exit 1
            fi
            ;;
        *)
            log_error "Unknown test suite: ${TEST_SUITE}"
            exit 1
            ;;
    esac
    
    log_success "Test suite completed successfully"
}

# Generate coverage data
generate_coverage_data() {
    log_info "Generating coverage data..."
    
    # Create output directory
    mkdir -p "${OUTPUT_DIR}"
    
    # Capture coverage data
    lcov --capture \
         --directory "${BUILD_DIR}" \
         --output-file "${OUTPUT_DIR}/coverage_raw.info" \
         --rc lcov_branch_coverage=1 \
         --quiet
    
    # Build exclude pattern
    local exclude_args=()
    
    # Default excludes
    exclude_args+=(--remove "${OUTPUT_DIR}/coverage_raw.info")
    exclude_args+=('/usr/*')
    exclude_args+=('*/tests/*')
    exclude_args+=('*/unity/*')
    exclude_args+=('*/external/*')
    
    # User-specified excludes
    for pattern in "${EXCLUDE_PATTERNS[@]}"; do
        exclude_args+=("${pattern}")
    done
    
    # Apply excludes
    lcov "${exclude_args[@]}" \
         --output-file "${OUTPUT_DIR}/coverage_filtered.info" \
         --rc lcov_branch_coverage=1 \
         --quiet
    
    log_success "Coverage data generated"
}

# Generate HTML report
generate_html_report() {
    if [[ "$GENERATE_HTML" != "true" ]]; then
        return
    fi
    
    log_info "Generating HTML coverage report..."
    
    genhtml "${OUTPUT_DIR}/coverage_filtered.info" \
            --output-directory "${OUTPUT_DIR}/html" \
            --title "${PROJECT_NAME} Code Coverage Report" \
            --show-details \
            --legend \
            --branch-coverage \
            --function-coverage \
            --rc lcov_branch_coverage=1 \
            --quiet
    
    log_success "HTML report generated: ${OUTPUT_DIR}/html/index.html"
}

# Generate XML report (for CI)
generate_xml_report() {
    if [[ "$OUTPUT_FORMAT" != "xml" ]] && [[ "$CI_MODE" != "true" ]]; then
        return
    fi
    
    if ! command -v gcovr &> /dev/null; then
        log_warning "gcovr not available - skipping XML report"
        return
    fi
    
    log_info "Generating XML coverage report..."
    
    gcovr --xml \
          --xml-pretty \
          --output "${OUTPUT_DIR}/coverage.xml" \
          --root . \
          --exclude-directories tests \
          --exclude-directories external \
          --exclude-directories unity
    
    log_success "XML report generated: ${OUTPUT_DIR}/coverage.xml"
}

# Generate JSON report
generate_json_report() {
    if [[ "$OUTPUT_FORMAT" != "json" ]]; then
        return
    fi
    
    if ! command -v gcovr &> /dev/null; then
        log_warning "gcovr not available - skipping JSON report"
        return
    fi
    
    log_info "Generating JSON coverage report..."
    
    gcovr --json \
          --json-pretty \
          --output "${OUTPUT_DIR}/coverage.json" \
          --root . \
          --exclude-directories tests \
          --exclude-directories external \
          --exclude-directories unity
    
    log_success "JSON report generated: ${OUTPUT_DIR}/coverage.json"
}

# Extract coverage metrics
extract_coverage_metrics() {
    log_info "Extracting coverage metrics..."
    
    # Generate summary
    lcov --summary "${OUTPUT_DIR}/coverage_filtered.info" \
         --rc lcov_branch_coverage=1 > "${OUTPUT_DIR}/summary.txt" 2>&1
    
    # Extract percentages
    FUNCTION_COVERAGE=$(grep "functions.." "${OUTPUT_DIR}/summary.txt" | \
                       sed 's/.*(\([0-9.]*\)%.*/\1/' || echo "0")
    
    LINE_COVERAGE=$(grep "lines......" "${OUTPUT_DIR}/summary.txt" | \
                   sed 's/.*(\([0-9.]*\)%.*/\1/' || echo "0")
    
    BRANCH_COVERAGE=$(grep "branches.." "${OUTPUT_DIR}/summary.txt" | \
                     sed 's/.*(\([0-9.]*\)%.*/\1/' || echo "0")
    
    # Handle cases where branch coverage might not be available
    if [[ -z "$BRANCH_COVERAGE" ]] || [[ "$BRANCH_COVERAGE" == "0" ]]; then
        BRANCH_COVERAGE="N/A"
    fi
}

# Print coverage summary
print_coverage_summary() {
    echo
    echo "================================================================"
    echo "                        Coverage Summary"
    echo "================================================================"
    echo
    printf "Function Coverage: %s%%\n" "$FUNCTION_COVERAGE"
    printf "Line Coverage:     %s%%\n" "$LINE_COVERAGE"
    printf "Branch Coverage:   %s\n" "$BRANCH_COVERAGE"
    echo
    
    if [[ "$CI_MODE" == "true" ]]; then
        # Machine-readable output for CI
        echo "COVERAGE_FUNCTION=${FUNCTION_COVERAGE}"
        echo "COVERAGE_LINE=${LINE_COVERAGE}"
        echo "COVERAGE_BRANCH=${BRANCH_COVERAGE}"
    fi
}

# Check coverage thresholds
check_thresholds() {
    local failed=false
    
    log_info "Checking coverage thresholds..."
    
    # Check function coverage
    if (( $(echo "${FUNCTION_COVERAGE} < ${FUNCTION_THRESHOLD}" | bc -l) )); then
        log_error "Function coverage ${FUNCTION_COVERAGE}% below threshold ${FUNCTION_THRESHOLD}%"
        failed=true
    else
        log_success "Function coverage ${FUNCTION_COVERAGE}% meets threshold ${FUNCTION_THRESHOLD}%"
    fi
    
    # Check line coverage
    if (( $(echo "${LINE_COVERAGE} < ${LINE_THRESHOLD}" | bc -l) )); then
        log_error "Line coverage ${LINE_COVERAGE}% below threshold ${LINE_THRESHOLD}%"
        failed=true
    else
        log_success "Line coverage ${LINE_COVERAGE}% meets threshold ${LINE_THRESHOLD}%"
    fi
    
    # Check branch coverage (if available)
    if [[ "$BRANCH_COVERAGE" != "N/A" ]]; then
        if (( $(echo "${BRANCH_COVERAGE} < ${BRANCH_THRESHOLD}" | bc -l) )); then
            log_error "Branch coverage ${BRANCH_COVERAGE}% below threshold ${BRANCH_THRESHOLD}%"
            failed=true
        else
            log_success "Branch coverage ${BRANCH_COVERAGE}% meets threshold ${BRANCH_THRESHOLD}%"
        fi
    fi
    
    if [[ "$failed" == "true" ]]; then
        echo
        log_error "Coverage thresholds not met!"
        exit 1
    else
        echo
        log_success "All coverage thresholds met!"
    fi
}

# Generate badge (for README)
generate_badge() {
    if [[ "$CI_MODE" != "true" ]]; then
        return
    fi
    
    local color="red"
    if (( $(echo "${LINE_COVERAGE} >= 90" | bc -l) )); then
        color="brightgreen"
    elif (( $(echo "${LINE_COVERAGE} >= 80" | bc -l) )); then
        color="yellow"
    elif (( $(echo "${LINE_COVERAGE} >= 70" | bc -l) )); then
        color="orange"
    fi
    
    local badge_url="https://img.shields.io/badge/coverage-${LINE_COVERAGE}%25-${color}"
    echo "COVERAGE_BADGE_URL=${badge_url}" >> "${OUTPUT_DIR}/badge.txt"
}

# Main execution
main() {
    print_banner
    parse_arguments "$@"
    check_dependencies
    
    if [[ "$CLEAN_COVERAGE" == "true" ]]; then
        clean_coverage
    fi
    
    build_with_coverage
    run_tests
    generate_coverage_data
    
    # Generate reports based on format
    case "$OUTPUT_FORMAT" in
        "html")
            generate_html_report
            ;;
        "xml")
            generate_xml_report
            ;;
        "json")
            generate_json_report
            ;;
        "text")
            # Text summary is always generated
            ;;
        *)
            generate_html_report
            ;;
    esac
    
    # Always generate HTML in CI mode for artifacts
    if [[ "$CI_MODE" == "true" ]]; then
        generate_html_report
        generate_xml_report
        generate_badge
    fi
    
    extract_coverage_metrics
    print_coverage_summary
    check_thresholds
    
    log_success "Coverage analysis completed successfully!"
}

# Run main function with all arguments
main "$@"
