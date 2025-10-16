/**
 * json.y
 * 
 * Bison parser specification for JSON.
 * UPGRADED VERSION using uthash for object member storage.
 */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_value.h"

/* Forward declarations */
void json_yyerror(const char* s);
int json_yylex(void);

/* Global variable to hold the parsed JSON result */
JsonValue* json_result = NULL;
%}

/* Union to hold different types of values during parsing */
%union {
    double number;
    char* string;
    JsonValue* value;
    JsonObjectMember* object_member;
    JsonArrayElement* array_element;
}

/* Token declarations */
%token LBRACE RBRACE LBRACK RBRACK COMMA COLON
%token TRUE FALSE NULL_TOKEN
%token ERROR
%token <number> NUMBER
%token <string> STRING

/* Non-terminal types */
%type <value> json value object array
%type <object_member> members member
%type <array_element> elements

/* Starting symbol */
%start json

%%

/**
 * Entry point: a JSON document is a single value.
 */
json:
    value                       { json_result = $1; $$ = $1; }
    ;

/**
 * A JSON value can be one of several types.
 */
value:
    object                      { $$ = $1; }
    | array                     { $$ = $1; }
    | STRING                    { $$ = create_json_string($1); free($1); }
    | NUMBER                    { $$ = create_json_number($1); }
    | TRUE                      { $$ = create_json_bool(1); }
    | FALSE                     { $$ = create_json_bool(0); }
    | NULL_TOKEN                { $$ = create_json_null(); }
    ;

/**
 * A JSON object: { members } or {}
 * UPGRADED: Now uses hash table instead of linked list.
 */
object:
    LBRACE RBRACE               {
        $$ = create_json_object();
    }
    | LBRACE members RBRACE     {
        JsonValue* obj = create_json_object();
        
        /* Add all members from linked list to hash table */
        JsonObjectMember* current = $2;
        while (current != NULL) {
            JsonObjectMember* next = current->next;
            current->next = NULL;  // Clear the temporary next pointer
            
            /* Add to hash table using uthash */
            HASH_ADD_KEYPTR(hh, obj->value.object, current->key, 
                           strlen(current->key), current);
            
            current = next;
        }
        
        $$ = obj;
    }
    ;

/**
 * Object members: one or more key-value pairs.
 * Built as temporary linked list during parsing, then added to hash table.
 */
members:
    member                      { $$ = $1; }
    | member COMMA members      {
        $1->next = $3;
        $$ = $1;
    }
    ;

/**
 * A single object member: "key" : value
 */
member:
    STRING COLON value          {
        JsonObjectMember* m = (JsonObjectMember*)malloc(sizeof(JsonObjectMember));
        m->key = strdup($1);
        m->value = $3;
        m->next = NULL;
        free($1);
        $$ = m;
    }
    ;

/**
 * A JSON array: [ elements ] or []
 */
array:
    LBRACK RBRACK               {
        $$ = create_json_array();
    }
    | LBRACK elements RBRACK    {
        JsonValue* arr = create_json_array();
        arr->value.array = $2;
        $$ = arr;
    }
    ;

/**
 * Array elements: one or more values.
 */
elements:
    value                       {
        JsonArrayElement* e = (JsonArrayElement*)malloc(sizeof(JsonArrayElement));
        e->value = $1;
        e->next = NULL;
        $$ = e;
    }
    | value COMMA elements      {
        JsonArrayElement* e = (JsonArrayElement*)malloc(sizeof(JsonArrayElement));
        e->value = $1;
        e->next = $3;
        $$ = e;
    }
    ;

%%

/**
 * Error handler for the JSON parser.
 */
void json_yyerror(const char* s) {
    fprintf(stderr, "JSON Parse Error: %s\n", s);
}
