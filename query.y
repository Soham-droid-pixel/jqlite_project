/**
 * query.y
 * 
 * Bison parser specification for the jqlite query language.
 * Builds an Abstract Syntax Tree (linked list of QueryNode structs)
 * representing the query operations.
 */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_value.h"

/* Forward declarations */
void query_yyerror(const char* s);
int query_yylex(void);

/* Global variable to hold the parsed query result */
QueryNode* query_result = NULL;
%}

/* Union to hold different types of values during parsing */
%union {
    int number;
    char* string;
    QueryNode* node;
}

/* Token declarations */
%token DOT LBRACK RBRACK ERROR
%token <number> INDEX
%token <string> IDENT

/* Non-terminal types */
%type <node> query operations operation

/* Starting symbol */
%start query

%%

/**
 * A query is a sequence of operations.
 */
query:
    operations                  { query_result = $1; $$ = $1; }
    ;

/**
 * Operations are chained together.
 */
operations:
    operation                   { $$ = $1; }
    | operation operations      {
        $1->next = $2;
        $$ = $1;
    }
    ;

/**
 * A single operation can be:
 * - Identity: .
 * - Field access: .fieldname
 * - Array index: [n]
 */
operation:
    DOT                         {
        /* Identity operation */
        QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
        node->type = QUERY_IDENTITY;
        node->next = NULL;
        $$ = node;
    }
    | DOT IDENT                 {
        /* Field access */
        QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
        node->type = QUERY_FIELD;
        node->data.field = $2;
        node->next = NULL;
        $$ = node;
    }
    | LBRACK INDEX RBRACK       {
        /* Array index */
        QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
        node->type = QUERY_INDEX;
        node->data.index = $2;
        node->next = NULL;
        $$ = node;
    }
    ;

%%

/**
 * Error handler for the query parser.
 */
void query_yyerror(const char* s) {
    fprintf(stderr, "Query Parse Error: %s\n", s);
}
