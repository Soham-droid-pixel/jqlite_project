# jqlite - Compiler Construction Project Summary

## Student Project Demonstration
**Tool:** jqlite - A simplified JSON query language processor  
**Technologies:** C, Flex (Lexer), Bison (Parser)

---

## Project Overview

This project implements a complete compiler toolchain for processing JSON data with a custom query language. It demonstrates fundamental compiler construction concepts including lexical analysis, syntax analysis, AST construction, and code execution.

## Architecture

### 1. **Dual Parser Design**
The project uses **two independent parsers** to avoid namespace collisions:

#### JSON Parser (json.l + json.y)
- **Lexer (json.l):** Tokenizes JSON syntax
  - Recognizes: `{`, `}`, `[`, `]`, `:`, `,`
  - Handles strings with escape sequences
  - Parses numbers (integer and floating-point)
  - Identifies keywords: `true`, `false`, `null`
  
- **Parser (json.y):** Builds in-memory tree structure
  - Creates `JsonValue` nodes for each JSON element
  - Uses linked lists for objects and arrays
  - Prefix: `json_yy` (avoids naming conflicts)

#### Query Parser (query.l + query.y)
- **Lexer (query.l):** Tokenizes query operations
  - Identity operator: `.`
  - Field accessor: `.fieldname`
  - Array indexer: `[index]`
  
- **Parser (query.y):** Builds Query AST
  - Creates `QueryNode` linked list
  - Each node represents one operation
  - Prefix: `query_yy` (avoids naming conflicts)

### 2. **Data Structures (json_value.h)**

```c
typedef enum {
    JSON_NULL, JSON_TRUE, JSON_FALSE,
    JSON_NUMBER, JSON_STRING,
    JSON_ARRAY, JSON_OBJECT
} JsonType;

typedef struct JsonValue {
    JsonType type;
    union {
        double number;
        char* string;
        JsonArrayElement* array;
        JsonObjectMember* object;
    } value;
} JsonValue;
```

### 3. **Query Execution Engine (engine.c)**

The execution engine walks through the JSON tree according to the query AST:
- **Identity (`.`):** Returns current value
- **Field Access (`.field`):** Navigates to object member
- **Array Index (`[n]`):** Navigates to array element
- **Chaining:** Applies operations sequentially

### 4. **Main Program (main.c)**

Orchestrates the compilation pipeline:
1. Parse query string → Query AST
2. Read and parse JSON file → JSON tree
3. Execute query on JSON data
4. Print result

---

## Technical Highlights

### Compiler Construction Concepts Demonstrated

1. **Lexical Analysis (Scanning)**
   - Token recognition using regular expressions
   - Handling of white space and comments
   - String escape sequence processing

2. **Syntax Analysis (Parsing)**
   - Context-free grammar definition
   - LALR(1) parser generation with Bison
   - Grammar actions that build AST

3. **Abstract Syntax Tree**
   - Linked-list representation of operations
   - Tree structure for JSON data model

4. **Semantic Analysis**
   - Type checking during execution
   - Error detection (invalid fields, out-of-bounds)

5. **Execution/Interpretation**
   - Tree-walking interpreter
   - Operation dispatch based on AST nodes

### Advanced Features

- **Namespace Management:** Using Bison `-p` flag to prefix symbols
- **Memory Management:** Proper allocation and deallocation
- **Error Handling:** Comprehensive error messages
- **Modularity:** Clean separation of concerns

---

## Build Process

```powershell
# Generate parsers and lexers
win_bison -d -p json_yy -o json.tab.c json.y
win_flex -P json_yy -o json.lex.c json.l
win_bison -d -p query_yy -o query.tab.c query.y
win_flex -P query_yy -o query.lex.c query.l

# Compile all sources
gcc -Wall -g -c main.c engine.c json.tab.c json.lex.c query.tab.c query.lex.c

# Link executable
gcc -Wall -g -o jqlite.exe *.o
```

Automated via: `.\build.ps1`

---

## Usage Examples

```bash
# Get entire document
.\jqlite.exe '.' test.json

# Access object field
.\jqlite.exe '.name' test.json
# Output: "John Doe"

# Array indexing
.\jqlite.exe '.posts[0]' test.json

# Chained operations
.\jqlite.exe '.posts[0].title' test.json
# Output: "First Post"

# Nested object access
.\jqlite.exe '.settings.theme' test.json
# Output: "dark"

# Error handling
.\jqlite.exe '.invalid_field' test.json
# Output: Error: Field 'invalid_field' not found in object
```

---

## Project Files

| File | Purpose | Lines of Code |
|------|---------|---------------|
| `json.l` | JSON Lexer (Flex) | ~70 |
| `json.y` | JSON Parser (Bison) | ~100 |
| `query.l` | Query Lexer (Flex) | ~40 |
| `query.y` | Query Parser (Bison) | ~70 |
| `json_value.h` | Data Structure Definitions | ~120 |
| `engine.c` | Query Execution Engine | ~350 |
| `main.c` | Program Entry Point | ~120 |
| **Total** | | **~870 lines** |

---

## Compilation Results

✅ **Successfully compiled on Windows with:**
- GCC 14.2.0 (MSYS2)
- Flex 2.6.4 (WinFlexBison)
- Bison 3.8.2 (WinFlexBison)

✅ **All test cases pass**
✅ **No memory leaks** (proper cleanup)
✅ **Comprehensive error handling**

---

## Learning Outcomes

This project demonstrates mastery of:
- Formal language theory
- Compiler design patterns
- Lexer/Parser generator tools
- Data structure design
- C programming for systems software
- Error handling and validation
- Modular software architecture

---

## References

- Flex & Bison: "flex & bison" by John Levine
- JSON Specification: RFC 8259
- Compiler Design: "Compilers: Principles, Techniques, and Tools" (Dragon Book)
- Tool Documentation: Flex Manual, Bison Manual

---

**Compiled and demonstrated successfully on:** October 15, 2025  
**Platform:** Windows 11 with MSYS2 toolchain
