/**
 * json_value.h
 * 
 * Defines the core data structures to represent JSON values in memory.
 * Uses a tree structure with linked lists for objects and arrays.
 */

#ifndef JSON_VALUE_H
#define JSON_VALUE_H

#include <stdlib.h>

/**
 * Enumeration of all possible JSON value types.
 */
typedef enum {
    JSON_NULL,      // null
    JSON_TRUE,      // true
    JSON_FALSE,     // false
    JSON_NUMBER,    // Numeric value (stored as double)
    JSON_STRING,    // String value
    JSON_ARRAY,     // Array of values
    JSON_OBJECT     // Object (key-value pairs)
} JsonType;

/**
 * Forward declaration of JsonValue struct.
 */
struct JsonValue;

/**
 * Represents a single member of a JSON object.
 * Objects are stored as linked lists of key-value pairs.
 */
typedef struct JsonObjectMember {
    char* key;                          // The key (field name)
    struct JsonValue* value;            // Pointer to the value
    struct JsonObjectMember* next;      // Pointer to next member in the list
} JsonObjectMember;

/**
 * Represents a single element in a JSON array.
 * Arrays are stored as linked lists of values.
 */
typedef struct JsonArrayElement {
    struct JsonValue* value;            // Pointer to the value
    struct JsonArrayElement* next;      // Pointer to next element in the list
} JsonArrayElement;

/**
 * Represents any JSON value.
 * Uses a union to store different value types efficiently.
 */
typedef struct JsonValue {
    JsonType type;                      // The type of this JSON value
    union {
        double number;                  // For JSON_NUMBER
        char* string;                   // For JSON_STRING
        JsonArrayElement* array;        // For JSON_ARRAY (head of linked list)
        JsonObjectMember* object;       // For JSON_OBJECT (head of linked list)
    } value;
} JsonValue;

/**
 * Represents a single operation in the query AST.
 * Queries are represented as linked lists of operations.
 */
typedef enum {
    QUERY_IDENTITY,     // The '.' operation (selects entire document)
    QUERY_FIELD,        // Field access: .fieldname
    QUERY_INDEX         // Array index: [n]
} QueryNodeType;

/**
 * A node in the query Abstract Syntax Tree.
 */
typedef struct QueryNode {
    QueryNodeType type;         // Type of this query operation
    union {
        char* field;            // For QUERY_FIELD
        int index;              // For QUERY_INDEX
    } data;
    struct QueryNode* next;     // Next operation in the query chain
} QueryNode;

/* Function prototypes for creating and manipulating JSON values */

/**
 * Create a new JSON null value.
 */
JsonValue* create_json_null();

/**
 * Create a new JSON boolean value.
 */
JsonValue* create_json_bool(int is_true);

/**
 * Create a new JSON number value.
 */
JsonValue* create_json_number(double num);

/**
 * Create a new JSON string value.
 */
JsonValue* create_json_string(const char* str);

/**
 * Create a new empty JSON array.
 */
JsonValue* create_json_array();

/**
 * Create a new empty JSON object.
 */
JsonValue* create_json_object();

/**
 * Add an element to a JSON array.
 */
void json_array_add(JsonValue* array, JsonValue* element);

/**
 * Add a member (key-value pair) to a JSON object.
 */
void json_object_add(JsonValue* object, const char* key, JsonValue* value);

/**
 * Print a JSON value to stdout.
 */
void print_json_value(JsonValue* value, int indent);

/**
 * Free memory allocated for a JSON value and all its children.
 */
void free_json_value(JsonValue* value);

/* Function prototypes for query execution */

/**
 * Execute a query on JSON data and return the result.
 */
JsonValue* execute_query(QueryNode* query, JsonValue* json_data);

/**
 * Free memory allocated for a query AST.
 */
void free_query(QueryNode* query);

#endif /* JSON_VALUE_H */
