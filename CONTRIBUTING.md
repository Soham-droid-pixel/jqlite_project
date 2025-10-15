# Contributing to jqlite 🤝

Thank you for your interest in contributing to jqlite! This document provides guidelines and information for contributors.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [How to Contribute](#how-to-contribute)
- [Coding Standards](#coding-standards)
- [Testing](#testing)
- [Pull Request Process](#pull-request-process)

## Code of Conduct

This project adheres to a code of conduct that promotes a welcoming and inclusive environment. By participating, you agree to:

- Be respectful and inclusive
- Accept constructive criticism gracefully
- Focus on what is best for the community
- Show empathy towards other community members

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/yourusername/jqlite.git
   cd jqlite
   ```
3. **Add upstream remote**:
   ```bash
   git remote add upstream https://github.com/originalauthor/jqlite.git
   ```
4. **Create a branch** for your changes:
   ```bash
   git checkout -b feature/your-feature-name
   ```

## Development Setup

### Prerequisites

- GCC or compatible C compiler
- Flex (lexical analyzer generator)
- Bison (parser generator)
- Make (or use build.ps1 on Windows)

### Building

```bash
# Linux/macOS
make

# Windows
.\build.ps1
```

### Running Tests

```bash
# Run demo script
.\demo.ps1  # Windows
./run_tests.sh  # Linux/macOS (create this)

# Manual testing
./jqlite '.name' test.json
```

## How to Contribute

### Reporting Bugs

If you find a bug, please create an issue with:

- **Clear title** describing the bug
- **Steps to reproduce** the issue
- **Expected behavior** vs actual behavior
- **Environment details** (OS, compiler version, etc.)
- **Sample JSON** and query that causes the issue

Example:
```
Title: Segmentation fault when accessing deeply nested objects

Steps to reproduce:
1. Create JSON with 5+ nested objects
2. Run: ./jqlite '.a.b.c.d.e.f' deep.json
3. Program crashes

Expected: Should return the nested value
Actual: Segmentation fault

Environment: Ubuntu 22.04, GCC 11.3, Flex 2.6.4, Bison 3.8
```

### Suggesting Enhancements

For feature requests, create an issue with:

- **Clear description** of the feature
- **Use cases** and motivation
- **Proposed implementation** (if you have ideas)
- **Examples** of how it would work

### Areas for Contribution

Here are some areas where we welcome contributions:

#### 1. New Query Operations
- Array slicing: `[0:5]`, `[-2:]`
- Array iteration: `.posts[]`
- Filter operations: `select()`, `map()`, `reduce()`
- Pipe operator: `|`

#### 2. Built-in Functions
- `length` - Get length of arrays/objects
- `keys` - Get object keys
- `values` - Get object values
- `type` - Get type of value
- `sort` - Sort arrays
- `unique` - Get unique values

#### 3. Output Formats
- Compact JSON output
- Pretty-printing options
- CSV output
- YAML output

#### 4. Error Handling
- Better error messages
- Line/column information in errors
- Suggestions for common mistakes

#### 5. Performance
- Optimize memory allocation
- Reduce parsing overhead
- Add benchmarks

#### 6. Documentation
- Improve code comments
- Add more examples
- Create video tutorials
- Translate documentation

## Coding Standards

### C Code Style

- **Indentation**: 4 spaces (no tabs)
- **Brace style**: K&R style
  ```c
  if (condition) {
      // code
  } else {
      // code
  }
  ```
- **Naming conventions**:
  - Functions: `snake_case`
  - Variables: `snake_case`
  - Constants: `UPPER_CASE`
  - Structs/Types: `PascalCase` with typedef
- **Comments**: Use `/* */` for block comments, `//` for inline
- **Max line length**: 100 characters

### Example

```c
/**
 * Create a new JSON string value.
 * 
 * @param str The string value (will be copied)
 * @return Newly allocated JsonValue
 */
JsonValue* create_json_string(const char* str) {
    JsonValue* val = (JsonValue*)malloc(sizeof(JsonValue));
    if (val == NULL) {
        return NULL;
    }
    
    val->type = JSON_STRING;
    val->value.string = strdup(str);
    
    return val;
}
```

### Flex/Bison Style

- Use clear, descriptive token names
- Add comments explaining regex patterns
- Document grammar rules
- Keep rules simple and focused

### Memory Management

- **Always free** allocated memory
- **Check for NULL** after malloc/calloc
- **Use strdup** for string copying
- **Document ownership** of pointers in comments

## Testing

### Adding Tests

When adding new features:

1. Add test JSON files to the project
2. Add test cases to demo.ps1
3. Document expected behavior
4. Test on multiple platforms if possible

### Test Coverage

Ensure your changes don't break existing tests:

```bash
# Run all demos
.\demo.ps1

# Test specific queries
.\jqlite.exe '.new_feature' test.json
```

### Edge Cases to Test

- Empty arrays and objects
- Null values
- Very large numbers
- Unicode strings
- Deeply nested structures
- Invalid JSON
- Invalid queries

## Pull Request Process

1. **Update your branch** with latest upstream:
   ```bash
   git fetch upstream
   git rebase upstream/main
   ```

2. **Make your changes** following the coding standards

3. **Test thoroughly**:
   ```bash
   # Clean build
   make clean
   make
   
   # Run tests
   .\demo.ps1
   ```

4. **Commit your changes** with clear messages:
   ```bash
   git add .
   git commit -m "Add array slicing feature

   - Implement [start:end] syntax in query parser
   - Add tests for array slicing
   - Update documentation with examples"
   ```

5. **Push to your fork**:
   ```bash
   git push origin feature/your-feature-name
   ```

6. **Create Pull Request** on GitHub with:
   - Clear title describing the change
   - Detailed description of what changed and why
   - Reference to related issues
   - Screenshots/examples if applicable

### PR Checklist

Before submitting, ensure:

- [ ] Code follows the style guidelines
- [ ] All tests pass
- [ ] New code has comments
- [ ] Documentation is updated
- [ ] No compilation warnings
- [ ] Memory leaks checked (use valgrind if available)
- [ ] Commit messages are clear

### Review Process

1. Maintainer will review your PR
2. Address any feedback or requested changes
3. Once approved, your PR will be merged
4. You'll be added to contributors list!

## Development Tips

### Debugging

Use GDB for debugging:
```bash
gcc -g -c *.c
gcc -g -o jqlite *.o
gdb ./jqlite
```

### Memory Leak Detection

Use valgrind on Linux:
```bash
valgrind --leak-check=full ./jqlite '.name' test.json
```

### Viewing Generated Code

Examine Flex/Bison output:
```bash
# See generated parser
cat json.tab.c

# See generated lexer
cat json.lex.c
```

### Understanding the Parser

Add debug output to Bison:
```bash
bison -d -p json_yy -t -o json.tab.c json.y
```

## Questions?

If you have questions:

- Open an issue with the "question" label
- Check existing issues and documentation
- Reach out to maintainers

## Recognition

Contributors will be:
- Listed in the project README
- Mentioned in release notes
- Given credit in commit history

Thank you for contributing to jqlite! 🎉
