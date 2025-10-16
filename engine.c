/**
 * engine.c
 * 
 * UPGRADED VERSION: Implementation of the query execution engine with:
 * - Hash table support for fast object lookups (uthash)
 * - Pipe operator support
 * - select() filtering
 * - Array slicing
 * - Array iteration
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_value.h"

/* Forward declarations for internal functions */
static JsonValue* execute_query_internal(QueryNode* query, JsonValue* json_data);
static int evaluate_condition(ConditionExpr* condition, JsonValue* item);
static JsonValue* clone_json_value_internal(JsonValue* value);

/**
 * Create a new JSON null value.
 */
JsonValue* create_json_null() {
    JsonValue* val = (JsonValue*)malloc(sizeof(JsonValue));
    val->type = JSON_NULL;
    return val;
}

/**
 * Create a new JSON boolean value.
 * 
 * @param is_true 1 for true, 0 for false
 */
JsonValue* create_json_bool(int is_true) {
    JsonValue* val = (JsonValue*)malloc(sizeof(JsonValue));
    val->type = is_true ? JSON_TRUE : JSON_FALSE;
    return val;
}

/**
 * Create a new JSON number value.
 * 
 * @param num The numeric value
 */
JsonValue* create_json_number(double num) {
    JsonValue* val = (JsonValue*)malloc(sizeof(JsonValue));
    val->type = JSON_NUMBER;
    val->value.number = num;
    return val;
}

/**
 * Create a new JSON string value.
 * 
 * @param str The string value (will be copied)
 */
JsonValue* create_json_string(const char* str) {
    JsonValue* val = (JsonValue*)malloc(sizeof(JsonValue));
    val->type = JSON_STRING;
    val->value.string = strdup(str);
    return val;
}

/**
 * Create a new empty JSON array.
 */
JsonValue* create_json_array() {
    JsonValue* val = (JsonValue*)malloc(sizeof(JsonValue));
    val->type = JSON_ARRAY;
    val->value.array = NULL;
    return val;
}

/**
 * Create a new empty JSON object.
 */
JsonValue* create_json_object() {
    JsonValue* val = (JsonValue*)malloc(sizeof(JsonValue));
    val->type = JSON_OBJECT;
    val->value.object = NULL;
    return val;
}

/**
 * Add an element to a JSON array.
 * Elements are appended to the end of the array.
 * 
 * @param array The JSON array to add to
 * @param element The value to add
 */
void json_array_add(JsonValue* array, JsonValue* element) {
    if (array->type != JSON_ARRAY) return;
    
    JsonArrayElement* new_elem = (JsonArrayElement*)malloc(sizeof(JsonArrayElement));
    new_elem->value = element;
    new_elem->next = NULL;
    
    if (array->value.array == NULL) {
        array->value.array = new_elem;
    } else {
        JsonArrayElement* curr = array->value.array;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = new_elem;
    }
}

/**
 * Add a member (key-value pair) to a JSON object using hash table.
 * UPGRADED: Uses uthash for O(1) insertion.
 * 
 * @param object The JSON object to add to
 * @param key The key (field name)
 * @param value The value
 */
void json_object_add(JsonValue* object, const char* key, JsonValue* value) {
    if (object->type != JSON_OBJECT) return;
    
    JsonObjectMember* new_member = (JsonObjectMember*)malloc(sizeof(JsonObjectMember));
    new_member->key = strdup(key);
    new_member->value = value;
    
    /* Add to hash table using uthash */
    HASH_ADD_KEYPTR(hh, object->value.object, new_member->key, 
                   strlen(new_member->key), new_member);
}

/**
 * Find a member in a JSON object by key.
 * UPGRADED: Uses uthash for O(1) lookup.
 * 
 * @param object The JSON object to search
 * @param key The key to find
 * @return The value associated with the key, or NULL if not found
 */
JsonValue* json_object_get(JsonValue* object, const char* key) {
    if (object->type != JSON_OBJECT) return NULL;
    
    JsonObjectMember* member;
    HASH_FIND_STR(object->value.object, key, member);
    
    return member ? member->value : NULL;
}

/**
 * Print a JSON value to stdout with proper formatting.
 * UPGRADED: Works with hash table structure.
 * 
 * @param value The JSON value to print
 * @param indent Current indentation level (for pretty printing)
 */
void print_json_value(JsonValue* value, int indent) {
    if (value == NULL) {
        printf("null");
        return;
    }
    
    int i;
    switch (value->type) {
        case JSON_NULL:
            printf("null");
            break;
            
        case JSON_TRUE:
            printf("true");
            break;
            
        case JSON_FALSE:
            printf("false");
            break;
            
        case JSON_NUMBER:
            // Print integers without decimal point
            if (value->value.number == (int)value->value.number) {
                printf("%d", (int)value->value.number);
            } else {
                printf("%g", value->value.number);
            }
            break;
            
        case JSON_STRING:
            printf("\"%s\"", value->value.string);
            break;
            
        case JSON_ARRAY:
            printf("[\n");
            {
                JsonArrayElement* elem = value->value.array;
                int first = 1;
                while (elem != NULL) {
                    if (!first) printf(",\n");
                    for (i = 0; i < indent + 2; i++) printf(" ");
                    print_json_value(elem->value, indent + 2);
                    elem = elem->next;
                    first = 0;
                }
            }
            printf("\n");
            for (i = 0; i < indent; i++) printf(" ");
            printf("]");
            break;
            
        case JSON_OBJECT:
            printf("{\n");
            {
                JsonObjectMember* member;
                int first = 1;
                
                /* Iterate through hash table */
                for (member = value->value.object; member != NULL; member = member->hh.next) {
                    if (!first) printf(",\n");
                    for (i = 0; i < indent + 2; i++) printf(" ");
                    printf("\"%s\": ", member->key);
                    print_json_value(member->value, indent + 2);
                    first = 0;
                }
            }
            printf("\n");
            for (i = 0; i < indent; i++) printf(" ");
            printf("}");
            break;
    }
}

/**
 * Clone a JSON value (deep copy).
 * 
 * @param value The value to clone
 * @return A new deep copy of the value
 */
JsonValue* clone_json_value(JsonValue* value) {
    return clone_json_value_internal(value);
}

/**
 * Internal helper for cloning JSON values.
 */
static JsonValue* clone_json_value_internal(JsonValue* value) {
    if (value == NULL) return NULL;
    
    switch (value->type) {
        case JSON_NULL:
            return create_json_null();
            
        case JSON_TRUE:
            return create_json_bool(1);
            
        case JSON_FALSE:
            return create_json_bool(0);
            
        case JSON_NUMBER:
            return create_json_number(value->value.number);
            
        case JSON_STRING:
            return create_json_string(value->value.string);
            
        case JSON_ARRAY: {
            JsonValue* new_array = create_json_array();
            JsonArrayElement* elem = value->value.array;
            while (elem != NULL) {
                json_array_add(new_array, clone_json_value_internal(elem->value));
                elem = elem->next;
            }
            return new_array;
        }
        
        case JSON_OBJECT: {
            JsonValue* new_object = create_json_object();
            JsonObjectMember* member;
            
            /* Iterate through hash table */
            for (member = value->value.object; member != NULL; member = member->hh.next) {
                json_object_add(new_object, member->key, 
                              clone_json_value_internal(member->value));
            }
            return new_object;
        }
    }
    
    return NULL;
}

/**
 * Free memory allocated for a JSON value and all its children.
 * UPGRADED: Properly frees hash table.
 * 
 * @param value The JSON value to free
 */
void free_json_value(JsonValue* value) {
    if (value == NULL) return;
    
    switch (value->type) {
        case JSON_STRING:
            free(value->value.string);
            break;
            
        case JSON_ARRAY:
            {
                JsonArrayElement* elem = value->value.array;
                while (elem != NULL) {
                    JsonArrayElement* next = elem->next;
                    free_json_value(elem->value);
                    free(elem);
                    elem = next;
                }
            }
            break;
            
        case JSON_OBJECT:
            {
                JsonObjectMember* member, *tmp;
                
                /* Delete all items from hash table */
                HASH_ITER(hh, value->value.object, member, tmp) {
                    HASH_DEL(value->value.object, member);
                    free(member->key);
                    free_json_value(member->value);
                    free(member);
                }
            }
            break;
            
        default:
            break;
    }
    
    free(value);
}

/**
 * Evaluate a condition expression on a JSON value.
 * Used by select() filtering.
 * 
 * @param condition The condition to evaluate
 * @param item The JSON value to test
 * @return 1 if condition is true, 0 otherwise
 */
static int evaluate_condition(ConditionExpr* condition, JsonValue* item) {
    if (condition == NULL || item == NULL) return 0;
    
    /* Execute the left-hand side query (usually a field access) */
    JsonValue* result = execute_query_internal(condition->left, item);
    if (result == NULL) return 0;
    
    /* Only numbers are supported in comparisons for now */
    if (result->type != JSON_NUMBER) {
        return 0;
    }
    
    double left_val = result->value.number;
    double right_val = condition->value;
    
    /* Perform comparison */
    switch (condition->op) {
        case CMP_GT:  return left_val > right_val;
        case CMP_LT:  return left_val < right_val;
        case CMP_EQ:  return left_val == right_val;
        case CMP_GTE: return left_val >= right_val;
        case CMP_LTE: return left_val <= right_val;
        case CMP_NEQ: return left_val != right_val;
        default:      return 0;
    }
}

/**
 * Internal query execution function (recursive for pipes).
 * 
 * @param query The query AST
 * @param json_data The JSON data to query
 * @return The result of the query
 */
static JsonValue* execute_query_internal(QueryNode* query, JsonValue* json_data) {
    if (json_data == NULL) {
        return NULL;
    }
    
    if (query == NULL) {
        return json_data;
    }
    
    switch (query->type) {
        case QUERY_IDENTITY:
            /* Identity: return current value and continue */
            return execute_query_internal(query->next, json_data);
            
        case QUERY_FIELD: {
            /* Field access: look up a key in an object */
            if (json_data->type != JSON_OBJECT) {
                fprintf(stderr, "Error: Cannot access field '%s' on non-object\n", 
                        query->data.field);
                return NULL;
            }
            
            /* Use hash table lookup (O(1)) */
            JsonValue* result = json_object_get(json_data, query->data.field);
            if (result == NULL) {
                fprintf(stderr, "Error: Field '%s' not found in object\n", 
                        query->data.field);
                return NULL;
            }
            
            return execute_query_internal(query->next, result);
        }
        
        case QUERY_INDEX: {
            /* Array index: access an element by index */
            if (json_data->type != JSON_ARRAY) {
                fprintf(stderr, "Error: Cannot index non-array with [%d]\n", 
                        query->data.index);
                return NULL;
            }
            
            JsonArrayElement* elem = json_data->value.array;
            int idx = 0;
            
            while (elem != NULL) {
                if (idx == query->data.index) {
                    return execute_query_internal(query->next, elem->value);
                }
                elem = elem->next;
                idx++;
            }
            
            fprintf(stderr, "Error: Array index %d out of bounds\n", query->data.index);
            return NULL;
        }
        
        case QUERY_SLICE: {
            /* Array slice: return a sub-array */
            if (json_data->type != JSON_ARRAY) {
                fprintf(stderr, "Error: Cannot slice non-array\n");
                return NULL;
            }
            
            JsonValue* result_array = create_json_array();
            JsonArrayElement* elem = json_data->value.array;
            int idx = 0;
            int start = query->data.slice.start;
            int end = query->data.slice.end;
            
            /* Count array length if end is -1 (slice to end) */
            if (end == -1) {
                JsonArrayElement* temp = elem;
                end = 0;
                while (temp != NULL) {
                    end++;
                    temp = temp->next;
                }
            }
            
            /* Collect elements in range [start, end) */
            while (elem != NULL && idx < end) {
                if (idx >= start) {
                    json_array_add(result_array, clone_json_value_internal(elem->value));
                }
                elem = elem->next;
                idx++;
            }
            
            return execute_query_internal(query->next, result_array);
        }
        
        case QUERY_ARRAY_ITER: {
            /* Array iteration: .[] - return array as-is for further processing */
            if (json_data->type != JSON_ARRAY) {
                fprintf(stderr, "Error: Cannot iterate over non-array\n");
                return NULL;
            }
            
            /* If there's a next operation, apply it to each element */
            if (query->next != NULL) {
                JsonValue* result_array = create_json_array();
                JsonArrayElement* elem = json_data->value.array;
                
                while (elem != NULL) {
                    JsonValue* item_result = execute_query_internal(query->next, elem->value);
                    if (item_result != NULL) {
                        json_array_add(result_array, item_result);
                    }
                    elem = elem->next;
                }
                
                return result_array;
            }
            
            return json_data;
        }
        
        case QUERY_SELECT: {
            /* Filter array elements with select() */
            if (json_data->type != JSON_ARRAY) {
                fprintf(stderr, "Error: select() can only be applied to arrays\n");
                return NULL;
            }
            
            JsonValue* result_array = create_json_array();
            JsonArrayElement* elem = json_data->value.array;
            
            while (elem != NULL) {
                if (evaluate_condition(query->data.condition, elem->value)) {
                    json_array_add(result_array, clone_json_value_internal(elem->value));
                }
                elem = elem->next;
            }
            
            return execute_query_internal(query->next, result_array);
        }
        
        case QUERY_PIPE: {
            /* Pipe: execute left side, then feed result to right side */
            JsonValue* left_result = execute_query_internal(query->data.pipe.left, json_data);
            if (left_result == NULL) {
                return NULL;
            }
            
            JsonValue* final_result = execute_query_internal(query->data.pipe.right, left_result);
            
            /* Continue with any remaining operations */
            if (query->next != NULL) {
                return execute_query_internal(query->next, final_result);
            }
            
            return final_result;
        }
        
        default:
            fprintf(stderr, "Error: Unknown query operation type\n");
            return NULL;
    }
}

/**
 * Execute a query on JSON data and return the result.
 * UPGRADED: Now supports pipes, filtering, and slicing.
 * 
 * @param query The query AST
 * @param json_data The JSON data to query
 * @return The result of the query
 */
JsonValue* execute_query(QueryNode* query, JsonValue* json_data) {
    if (json_data == NULL) {
        fprintf(stderr, "Error: Cannot execute query on NULL JSON data\n");
        return NULL;
    }
    
    return execute_query_internal(query, json_data);
}

/**
 * Free memory allocated for a query AST.
 * UPGRADED: Handles new query node types.
 * 
 * @param query The query AST to free
 */
void free_query(QueryNode* query) {
    if (query == NULL) return;
    
    QueryNode* next = query->next;
    
    switch (query->type) {
        case QUERY_FIELD:
            free(query->data.field);
            break;
            
        case QUERY_PIPE:
            free_query(query->data.pipe.left);
            free_query(query->data.pipe.right);
            break;
            
        case QUERY_SELECT:
            if (query->data.condition) {
                free_query(query->data.condition->left);
                free(query->data.condition);
            }
            break;
            
        default:
            break;
    }
    
    free(query);
    free_query(next);
}

/**
 * Create a new query node for pipe operation.
 */
QueryNode* create_pipe_node(QueryNode* left, QueryNode* right) {
    QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
    node->type = QUERY_PIPE;
    node->data.pipe.left = left;
    node->data.pipe.right = right;
    node->next = NULL;
    return node;
}

/**
 * Create a new query node for select operation.
 */
QueryNode* create_select_node(ConditionExpr* condition) {
    QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
    node->type = QUERY_SELECT;
    node->data.condition = condition;
    node->next = NULL;
    return node;
}

/**
 * Create a new query node for slice operation.
 */
QueryNode* create_slice_node(int start, int end) {
    QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
    node->type = QUERY_SLICE;
    node->data.slice.start = start;
    node->data.slice.end = end;
    node->next = NULL;
    return node;
}

/**
 * Create a new query node for array iteration.
 */
QueryNode* create_array_iter_node() {
    QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
    node->type = QUERY_ARRAY_ITER;
    node->next = NULL;
    return node;
}
