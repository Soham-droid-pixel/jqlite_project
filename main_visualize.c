/**
 * main.c
 * 
 * Entry point for the jqlite program with Compiler Visualization support.
 * Handles command-line arguments, orchestrates parsing of query and JSON,
 * executes the query, and prints the result.
 * 
 * NEW: Supports --visualize flag for educational compiler exploration.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_value.h"

/* Global visualization mode flag */
int g_visualize_mode = 0;

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
        if (!g_visualize_mode) {
            fprintf(stderr, "Error: Could not open file '%s'\n", filename);
        }
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate buffer and read file
    char* buffer = (char*)malloc(size + 1);
    if (buffer == NULL) {
        if (!g_visualize_mode) {
            fprintf(stderr, "Error: Memory allocation failed\n");
        }
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
 * Usage: jqlite [--visualize] '<query>' <json_file>
 */
int main(int argc, char** argv) {
    int arg_offset = 1;
    
    // Check for --visualize flag
    if (argc > 1 && strcmp(argv[1], "--visualize") == 0) {
        g_visualize_mode = 1;
        arg_offset = 2;
        
        // Start the visualization JSON output
        printf("{");
        printf("\"tokens\":[");
    }
    
    // Check command-line arguments
    if (argc < arg_offset + 2) {
        if (!g_visualize_mode) {
            fprintf(stderr, "Usage: %s [--visualize] '<query>' <json_file>\n", argv[0]);
            fprintf(stderr, "Example: %s '.posts[0].title' data.json\n", argv[0]);
        }
        return 1;
    }
    
    const char* query_string = argv[arg_offset];
    const char* json_filename = argv[arg_offset + 1];
    
    // Step 1: Parse the query string
    if (!g_visualize_mode) {
        printf("Parsing query: %s\n", query_string);
    }
    
    query_yy_scan_string(query_string);
    
    if (query_yyparse() != 0) {
        if (g_visualize_mode) {
            printf("],\"parseSteps\":[],\"executionTrace\":[],\"error\":\"Failed to parse query\"}");
        } else {
            fprintf(stderr, "Error: Failed to parse query\n");
        }
        return 1;
    }
    
    if (query_result == NULL) {
        if (g_visualize_mode) {
            printf("],\"parseSteps\":[],\"executionTrace\":[],\"error\":\"Query parsing produced no result\"}");
        } else {
            fprintf(stderr, "Error: Query parsing produced no result\n");
        }
        return 1;
    }
    
    if (!g_visualize_mode) {
        printf("Query parsed successfully.\n\n");
    }
    
    // Note: tokens array is closed and parseSteps array is opened by first log_parse_step() call
    
    // Step 2: Read and parse the JSON file
    if (!g_visualize_mode) {
        printf("Reading JSON file: %s\n", json_filename);
    }
    
    char* json_content = read_file(json_filename);
    if (json_content == NULL) {
        if (g_visualize_mode) {
            printf("],\"executionTrace\":[],\"error\":\"Could not read JSON file\"}");
        }
        free_query(query_result);
        return 1;
    }
    
    if (!g_visualize_mode) {
        printf("Parsing JSON...\n");
    }
    
    json_yy_scan_string(json_content);
    
    if (json_yyparse() != 0) {
        if (g_visualize_mode) {
            printf("],\"executionTrace\":[],\"error\":\"Failed to parse JSON\"}");
        } else {
            fprintf(stderr, "Error: Failed to parse JSON\n");
        }
        free(json_content);
        free_query(query_result);
        return 1;
    }
    
    if (json_result == NULL) {
        if (g_visualize_mode) {
            printf("],\"executionTrace\":[],\"error\":\"JSON parsing produced no result\"}");
        } else {
            fprintf(stderr, "Error: JSON parsing produced no result\n");
        }
        free(json_content);
        free_query(query_result);
        return 1;
    }
    
    if (!g_visualize_mode) {
        printf("JSON parsed successfully.\n\n");
    }
    free(json_content);
    
    // Close parseSteps and start executionTrace in visualize mode
    if (g_visualize_mode) {
        printf("],\"executionTrace\":[");
    }
    
    // Step 3: Execute the query on the JSON data
    if (!g_visualize_mode) {
        printf("Executing query...\n");
    }
    
    JsonValue* result = execute_query(query_result, json_result);
    
    if (result == NULL) {
        if (g_visualize_mode) {
            printf("],\"error\":\"Query execution failed\"}");
        } else {
            fprintf(stderr, "Error: Query execution failed\n");
        }
        free_json_value(json_result);
        free_query(query_result);
        return 1;
    }
    
    // Step 4: Print the result
    if (g_visualize_mode) {
        // Close executionTrace array
        printf("],\"finalResult\":");
        print_json_value(result, 0);
        printf("}");
    } else {
        printf("\nResult:\n");
        print_json_value(result, 0);
        printf("\n");
    }
    
    // Clean up
    free_json_value(json_result);
    free_query(query_result);
    
    return 0;
}
