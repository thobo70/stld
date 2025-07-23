# STLD/STAR Definition of Done Test Suite

This directory contains comprehensive integration tests that validate the STLD linker and STAR archiver functionality as a "Definition of Done" for the project.

## Test Overview

The Definition of Done test suite validates:

1. **SMOF Object File Processing** - Correct handling of SMOF format files
2. **STAR Archive Operations** - Create, list, and extract archive functionality  
3. **STLD Linking Operations** - Object and archive linking capabilities
4. **Memory Layout Management** - Base address and memory mapping
5. **x86_32 Architecture Support** - Architecture-specific code handling

## Test Scripts

### `test_simple_dod.sh` ⭐ **Primary Test**
- **Purpose**: Comprehensive toolchain validation with mock SMOF files
- **Requirements**: Built `stld` and `star` binaries only
- **Runtime**: ~30 seconds
- **Output**: Detailed test report and artifacts

**What it tests:**
- Creates realistic mock SMOF object files with x86_32 assembly
- Tests STAR archiver create/list/extract operations
- Tests STLD linker with objects and archives
- Validates memory mapping and base address handling
- Generates comprehensive analysis report

### `test_definition_of_done.sh` 🚧 **Advanced Test**
- **Purpose**: Full integration with real STAS assembler and QEMU
- **Requirements**: STAS assembler, QEMU, internet connection
- **Runtime**: ~5 minutes
- **Status**: Requires external dependencies

**What it tests:**
- Clones and builds STAS assembler from GitHub
- Assembles real x86_32 assembly sources to SMOF format
- Full toolchain integration: Assembly → SMOF → Archive → Link → Execute
- QEMU-based execution validation

## Quick Start

### Run Definition of Done Test

```bash
# From project root
make test-dod
```

### Manual Execution

```bash
# Run the primary test directly
cd tests/integration
./test_simple_dod.sh

# Run advanced test (requires dependencies)
./test_definition_of_done.sh
```

## Test Output Structure

```
tests/integration/output/
├── objects/          # Generated SMOF object files
│   ├── main.smof     # Entry point object
│   ├── hello.smof    # Function object  
│   └── math.smof     # Mathematical functions
├── archives/         # STAR archive files
│   ├── libtest.star  # Test library archive
│   └── contents.txt  # Archive listing
├── extracted/        # Extracted archive contents
├── linked/           # Linked binary outputs
│   ├── program       # Simple linked program
│   ├── program_with_archive  # Program using archive
│   └── memory.map    # Memory layout map
└── test_report.md    # Comprehensive test report
```

## Success Criteria

The Definition of Done is achieved when:

✅ **Object Processing**: Mock SMOF files are correctly processed  
✅ **Archive Operations**: All STAR operations (create/list/extract) work  
✅ **Linking Success**: Both object and archive linking complete  
✅ **Memory Management**: Proper base address and layout handling  
✅ **File Generation**: All expected output files are created  
✅ **Format Validation**: Generated files have correct structure  

## Understanding Test Results

### Exit Codes
- `0`: All tests passed successfully
- `1`: Critical test failure
- `2`: Tests passed with warnings

### Log Output
- 🔵 `[INFO]`: General test progress information
- 🟢 `[SUCCESS]`: Test phase completed successfully  
- 🟡 `[WARNING]`: Non-critical issues detected
- 🔴 `[ERROR]`: Critical test failure

### Generated Report
Each test run generates a Markdown report (`test_report.md`) containing:
- Executive summary with pass/fail status
- Detailed test scenario results
- File size and structure analysis
- Technical validation details
- Next steps and recommendations

## Mock SMOF File Structure

The test creates realistic SMOF files with:

```
SMOF Header:
├── Magic: "SMOF" (4 bytes)
├── Version: 1.0 (2 bytes)  
├── Architecture: x86_32 (2 bytes)
├── Section Count (2 bytes)
├── Symbol Count (2 bytes)
└── Reserved (4 bytes)

Sections:
├── .text (executable code)
└── .data (initialized data)

Symbols:
├── _start (entry point)
├── hello (function)
├── add_numbers (math function)
└── multiply (math function)

Machine Code:
└── Simulated x86_32 assembly instructions
```

## Integration with CI/CD

The Definition of Done test is designed for automation:

```yaml
# Example GitHub Actions integration
- name: Run Definition of Done Test
  run: make test-dod
  
- name: Archive Test Results
  uses: actions/upload-artifact@v3
  with:
    name: test-results
    path: tests/integration/output/
```

## Troubleshooting

### Common Issues

**Binary Not Found**
```bash
# Ensure binaries are built first
make stld star
```

**Permission Denied**
```bash
# Make test scripts executable
chmod +x tests/integration/*.sh
```

**Test Output Missing**
```bash
# Check disk space and permissions
df -h .
ls -la tests/integration/
```

### Debug Mode

Enable verbose output:
```bash
# Set debug environment variable
DEBUG=1 ./test_simple_dod.sh
```

## Extending the Tests

### Adding New Test Scenarios

1. **Create new assembly source** in `create_test_sources()`
2. **Add SMOF object generation** in `create_mock_smof_files()`
3. **Include in linking tests** in `test_stld_linker()`
4. **Update validation** in `validate_results()`

### Custom SMOF Files

Create your own mock SMOF files following the structure in `create_mock_smof_files()`:

```bash
# Binary format: Magic + Version + Arch + Sections + Symbols + Data
printf "SMOF\x01\x00\x01\x00\x01\x00\x01\x00\x00\x00\x00\x00" > custom.smof
```

## Related Documentation

- [SMOF Format Specification](../../docs/smof_format.md)
- [STLD Linker Manual](../../docs/stld_manual.md)  
- [STAR Archiver Manual](../../docs/star_manual.md)
- [Build System Guide](../../docs/build_system.md)

## Contributing

When modifying tests:

1. Ensure backward compatibility
2. Update expected file lists in `validate_results()`
3. Add new scenarios to the test report template
4. Test with both clean and incremental builds
5. Verify cross-platform compatibility

---

**Note**: The Definition of Done test represents the minimum viable functionality for the STLD/STAR toolchain. Passing this test confirms the project is ready for production use with x86_32 assembly projects.
