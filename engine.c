/**
 * engine.c
 * 
 * Implementation of the query execution engine and helper functions
 * for creating and manipulating JSON values.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_value.h"

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
 * Add a member (key-value pair) to a JSON object.
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
    new_member->next = object->value.object;
    object->value.object = new_member;
}

/**
 * Print a JSON value to stdout with proper formatting.
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
                JsonObjectMember* member = value->value.object;
                int first = 1;
                while (member != NULL) {
                    if (!first) printf(",\n");
                    for (i = 0; i < indent + 2; i++) printf(" ");
                    printf("\"%s\": ", member->key);
                    print_json_value(member->value, indent + 2);
                    member = member->next;
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
 * Free memory allocated for a JSON value and all its children.
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
                JsonObjectMember* member = value->value.object;
                while (member != NULL) {
                    JsonObjectMember* next = member->next;
                    free(member->key);
                    free_json_value(member->value);
                    free(member);
                    member = next;
                }
            }
            break;
            
        default:
            break;
    }
    
    free(value);
}

/**
 * Execute a query on JSON data and return the result.
 * 
 * This function walks through the query AST (linked list of operations)
 * and applies each operation to the JSON data structure.
 * 
 * @param query The query AST (linked list of QueryNode)
 * @param json_data The JSON data to query
 * @return The result of the query (a pointer to a JsonValue)
 */
JsonValue* execute_query(QueryNode* query, JsonValue* json_data) {
    if (json_data == NULL) {
        fprintf(stderr, "Error: Cannot execute query on NULL JSON data\n");
        return NULL;
    }
    
    JsonValue* current = json_data;
    QueryNode* op = query;
    
    // Walk through each operation in the query
    while (op != NULL) {
        if (current == NULL) {
            fprintf(stderr, "Error: Query operation on null value\n");
            return NULL;
        }
        
        switch (op->type) {
            case QUERY_IDENTITY:
                // Identity operation: return current value as-is
                // This just continues to the next operation
                break;
                
            case QUERY_FIELD:
                // Field access: look up a key in an object
                if (current->type != JSON_OBJECT) {
                    fprintf(stderr, "Error: Cannot access field '%s' on non-object\n", 
                            op->data.field);
                    return NULL;
                }
                
                // Search for the field in the object
                {
                    JsonObjectMember* member = current->value.object;
                    int found = 0;
                    while (member != NULL) {
                        if (strcmp(member->key, op->data.field) == 0) {
                            current = member->value;
                            found = 1;
                            break;
                        }
                        member = member->next;
                    }
                    
                    if (!found) {
                        fprintf(stderr, "Error: Field '%s' not found in object\n", 
                                op->data.field);
                        return NULL;
                    }
                }
                break;
                
            case QUERY_INDEX:
                // Array index: access an element by index
                if (current->type != JSON_ARRAY) {
                    fprintf(stderr, "Error: Cannot index non-array with [%d]\n", 
                            op->data.index);
                    return NULL;
                }
                
                // Find the element at the specified index
                {
                    JsonArrayElement* elem = current->value.array;
                    int idx = 0;
                    int found = 0;
                    
                    while (elem != NULL) {
                        if (idx == op->data.index) {
                            current = elem->value;
                            found = 1;
                            break;
                        }
                        elem = elem->next;
                        idx++;
                    }
                    
                    if (!found) {
                        fprintf(stderr, "Error: Array index %d out of bounds\n", 
                                op->data.index);
                        return NULL;
                    }
                }
                break;
        }
        
        op = op->next;
    }
    
    return current;
}

/**
 * Free memory allocated for a query AST.
 * 
 * @param query The query AST to free
 */
void free_query(QueryNode* query) {
    while (query != NULL) {
        QueryNode* next = query->next;
        if (query->type == QUERY_FIELD) {
            free(query->data.field);
        }
        free(query);
        query = next;
    }
}
