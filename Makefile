# Makefile for jqlite - A simplified JSON query tool
# Compiles all components including Flex lexers and Bison parsers

CC = gcc
CFLAGS = -Wall -g
LEX = flex
YACC = bison

# Target executable
TARGET = jqlite

# Source files
SOURCES = main.c engine.c json.tab.c json.lex.c query.tab.c query.lex.c
OBJECTS = $(SOURCES:.c=.o)

# Header dependencies
HEADERS = json_value.h json.tab.h query.tab.h

# Default target
all: $(TARGET)

# Link all object files to create the executable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

# Compile C source files to object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Generate JSON parser C code and header from Bison grammar
# Use -p json_yy to prefix all parser functions with json_yy (avoids name collision)
json.tab.c json.tab.h: json.y
	$(YACC) -d -p json_yy -o json.tab.c json.y

# Generate JSON lexer C code from Flex specification
# Use -P json_yy to prefix all lexer functions with json_yy
json.lex.c: json.l json.tab.h
	$(LEX) -P json_yy -o json.lex.c json.l

# Generate Query parser C code and header from Bison grammar
# Use -p query_yy to prefix all parser functions with query_yy (avoids name collision)
query.tab.c query.tab.h: query.y
	$(YACC) -d -p query_yy -o query.tab.c query.y

# Generate Query lexer C code from Flex specification
# Use -P query_yy to prefix all lexer functions with query_yy
query.lex.c: query.l query.tab.h
	$(LEX) -P query_yy -o query.lex.c query.l

# Clean generated files
clean:
	rm -f $(OBJECTS) $(TARGET) json.tab.c json.tab.h json.lex.c query.tab.c query.tab.h query.lex.c

# Phony targets
.PHONY: all clean
