# jqlite Quick Reference Guide

## Installation

```bash
# Windows
winget install WinFlexBison.win_flex_bison
.\build.ps1

# Linux/macOS
sudo apt-get install flex bison  # or brew install
make
```

## Basic Syntax

```bash
jqlite '<query>' <json_file>
```

## Operators

| Operator | Description | Example |
|----------|-------------|---------|
| `.` | Identity | `jqlite '.' data.json` |
| `.field` | Field access | `jqlite '.name' data.json` |
| `[n]` | Array index | `jqlite '[0]' data.json` |
| Chaining | Multiple ops | `jqlite '.users[0].name' data.json` |

## Common Queries

```bash
# Get entire document
jqlite '.' file.json

# Get single field
jqlite '.username' file.json

# Get nested field
jqlite '.user.profile.email' file.json

# Get first array element
jqlite '.items[0]' file.json

# Get specific array field
jqlite '.posts[2].title' file.json

# Deep nesting
jqlite '.data.users[0].address.city' file.json
```

## Examples with test.json

```json
{
  "name": "John Doe",
  "age": 30,
  "posts": [
    {"title": "First", "likes": 42},
    {"title": "Second", "likes": 128}
  ],
  "settings": {
    "theme": "dark"
  }
}
```

```bash
# Get name
jqlite '.name' test.json
# → "John Doe"

# Get age
jqlite '.age' test.json
# → 30

# Get first post
jqlite '.posts[0]' test.json
# → {"title": "First", "likes": 42}

# Get first post title
jqlite '.posts[0].title' test.json
# → "First"

# Get second post likes
jqlite '.posts[1].likes' test.json
# → 128

# Get theme
jqlite '.settings.theme' test.json
# → "dark"
```

## Error Messages

```bash
# Field not found
jqlite '.invalid' test.json
# → Error: Field 'invalid' not found in object

# Array out of bounds
jqlite '.posts[999]' test.json
# → Error: Array index 999 out of bounds

# Type mismatch
jqlite '.name[0]' test.json
# → Error: Cannot index non-array with [0]
```

## Tips

1. **Always quote queries**: Use single quotes around queries
2. **Check JSON validity**: Ensure your JSON is valid before querying
3. **Start simple**: Test with `.` first to see full document
4. **Chain carefully**: Build complex queries step by step

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Command not found | Add to PATH or use `./jqlite` |
| Parse error | Check JSON syntax with validator |
| Segfault | Update to latest version, report bug |
| Compilation error | Ensure Flex/Bison installed |

## Quick Build

```bash
# Windows (PowerShell)
.\build.ps1

# Linux/macOS
make

# Manual
bison -d -p json_yy -o json.tab.c json.y
bison -d -p query_yy -o query.tab.c query.y
flex -P json_yy -o json.lex.c json.l
flex -P query_yy -o query.lex.c query.l
gcc -Wall -g -c main.c engine.c json.tab.c json.lex.c query.tab.c query.lex.c
gcc -Wall -g -o jqlite *.o
```

## More Help

- Full docs: [README.md](README.md)
- Technical details: [PROJECT_SUMMARY.md](PROJECT_SUMMARY.md)
- Contributing: [CONTRIBUTING.md](CONTRIBUTING.md)
- Issues: [GitHub Issues](https://github.com/yourusername/jqlite/issues)

---

**Made with ❤️ using Flex and Bison**
