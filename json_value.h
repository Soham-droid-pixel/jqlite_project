/**
 * json_value.h
 * 
 * Defines the core data structures to represent JSON values in memory.
 * UPGRADED VERSION with hash table support for objects, pipe operations,
 * filtering, and array slicing.
 */

#ifndef JSON_VALUE_H
#define JSON_VALUE_H

#include <stdlib.h>
#include "uthash.h"  // Hash table library for fast object lookups

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
 * NOW USES UTHASH for O(1) lookups instead of linked list.
 * Note: 'next' field is kept for temporary use during parsing.
 */
typedef struct JsonObjectMember {
    char* key;                          // The key (field name) - also the hash key
    struct JsonValue* value;            // Pointer to the value
    UT_hash_handle hh;                  // Makes this structure hashable by uthash
    struct JsonObjectMember* next;      // Temporary: used during parsing, then discarded
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
        JsonObjectMember* object;       // For JSON_OBJECT (head of hash table)
    } value;
} JsonValue;

/**
 * Enumeration for comparison operators in select() expressions.
 */
typedef enum {
    CMP_GT,         // Greater than: >
    CMP_LT,         // Less than: <
    CMP_EQ,         // Equal: ==
    CMP_GTE,        // Greater than or equal: >=
    CMP_LTE,        // Less than or equal: <=
    CMP_NEQ         // Not equal: !=
} ComparisonOp;

/**
 * Represents a condition expression used in select() filtering.
 */
typedef struct ConditionExpr {
    struct QueryNode* left;             // Left side of comparison (usually a field access)
    ComparisonOp op;                    // Comparison operator
    double value;                       // Right side value (only numbers supported for now)
} ConditionExpr;

/**
 * Represents a single operation in the query AST.
 * UPGRADED to support pipes, filtering, and slicing.
 */
typedef enum {
    QUERY_IDENTITY,     // The '.' operation (selects entire document)
    QUERY_FIELD,        // Field access: .fieldname
    QUERY_INDEX,        // Array index: [n]
    QUERY_PIPE,         // Pipe operator: |
    QUERY_SELECT,       // Filter with select(): select(condition)
    QUERY_SLICE,        // Array slice: [start:end]
    QUERY_ARRAY_ITER    // Array iteration: .[]
} QueryNodeType;

/**
 * A node in the query Abstract Syntax Tree.
 * UPGRADED to support new operations.
 */
typedef struct QueryNode {
    QueryNodeType type;         // Type of this query operation
    union {
        char* field;            // For QUERY_FIELD
        int index;              // For QUERY_INDEX
        struct {
            int start;          // For QUERY_SLICE
            int end;
        } slice;
        struct {
            struct QueryNode* left;   // For QUERY_PIPE
            struct QueryNode* right;
        } pipe;
        ConditionExpr* condition;     // For QUERY_SELECT
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
 * Add a member (key-value pair) to a JSON object using hash table.
 */
void json_object_add(JsonValue* object, const char* key, JsonValue* value);

/**
 * Find a member in a JSON object by key (O(1) with hash table).
 */
JsonValue* json_object_get(JsonValue* object, const char* key);

/**
 * Print a JSON value to stdout.
 */
void print_json_value(JsonValue* value, int indent);

/**
 * Free memory allocated for a JSON value and all its children.
 */
void free_json_value(JsonValue* value);

/**
 * Clone a JSON value (deep copy).
 */
JsonValue* clone_json_value(JsonValue* value);

/* Function prototypes for query execution */

/**
 * Execute a query on JSON data and return the result.
 */
JsonValue* execute_query(QueryNode* query, JsonValue* json_data);

/**
 * Free memory allocated for a query AST.
 */
void free_query(QueryNode* query);

/**
 * Create a new query node for pipe operation.
 */
QueryNode* create_pipe_node(QueryNode* left, QueryNode* right);

/**
 * Create a new query node for select operation.
 */
QueryNode* create_select_node(ConditionExpr* condition);

/**
 * Create a new query node for slice operation.
 */
QueryNode* create_slice_node(int start, int end);

/**
 * Create a new query node for array iteration.
 */
QueryNode* create_array_iter_node();

#endif /* JSON_VALUE_H */
