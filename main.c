/**
 * main.c
 * 
 * Entry point for the jqlite program.
 * Handles command-line arguments, orchestrates parsing of query and JSON,
 * executes the query, and prints the result.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_value.h"

/* External declarations for the parsers */

/* JSON parser functions and variables */
extern int json_yyparse(void);
extern void json_yy_scan_string(const char* str);
extern void json_yy_delete_buffer(void* buffer);
extern JsonValue* json_result;

/* Query parser functions and variables */
extern int query_yyparse(void);
extern void query_yy_scan_string(const char* str);
extern void query_yy_delete_buffer(void* buffer);
extern QueryNode* query_result;

/**
 * Read the entire contents of a file into a string.
 * 
 * @param filename The path to the file
 * @return A newly allocated string containing the file contents, or NULL on error
 */
char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file '%s'\n", filename);
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate buffer and read file
    char* buffer = (char*)malloc(size + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }
    
    size_t bytes_read = fread(buffer, 1, size, file);
    buffer[bytes_read] = '\0';
    
    fclose(file);
    return buffer;
}

/**
 * Main entry point.
 * 
 * Usage: jqlite '<query>' <json_file>
 */
int main(int argc, char** argv) {
    // Check command-line arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s '<query>' <json_file>\n", argv[0]);
        fprintf(stderr, "Example: %s '.posts[0].title' data.json\n", argv[0]);
        return 1;
    }
    
    const char* query_string = argv[1];
    const char* json_filename = argv[2];
    
    // Step 1: Parse the query string
    printf("Parsing query: %s\n", query_string);
    query_yy_scan_string(query_string);
    
    if (query_yyparse() != 0) {
        fprintf(stderr, "Error: Failed to parse query\n");
        return 1;
    }
    
    if (query_result == NULL) {
        fprintf(stderr, "Error: Query parsing produced no result\n");
        return 1;
    }
    
    printf("Query parsed successfully.\n\n");
    
    // Step 2: Read and parse the JSON file
    printf("Reading JSON file: %s\n", json_filename);
    char* json_content = read_file(json_filename);
    if (json_content == NULL) {
        free_query(query_result);
        return 1;
    }
    
    printf("Parsing JSON...\n");
    json_yy_scan_string(json_content);
    
    if (json_yyparse() != 0) {
        fprintf(stderr, "Error: Failed to parse JSON\n");
        free(json_content);
        free_query(query_result);
        return 1;
    }
    
    if (json_result == NULL) {
        fprintf(stderr, "Error: JSON parsing produced no result\n");
        free(json_content);
        free_query(query_result);
        return 1;
    }
    
    printf("JSON parsed successfully.\n\n");
    free(json_content);
    
    // Step 3: Execute the query on the JSON data
    printf("Executing query...\n");
    JsonValue* result = execute_query(query_result, json_result);
    
    if (result == NULL) {
        fprintf(stderr, "Error: Query execution failed\n");
        free_json_value(json_result);
        free_query(query_result);
        return 1;
    }
    
    // Step 4: Print the result
    printf("\nResult:\n");
    print_json_value(result, 0);
    printf("\n");
    
    // Clean up
    free_json_value(json_result);
    free_query(query_result);
    
    return 0;
}
