/**
 * query.y
 * 
 * Bison parser specification for the jqlite query language.
 * UPGRADED VERSION with support for pipes, select filtering, and array slicing.
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
    double number;
    char* string;
    QueryNode* node;
    ConditionExpr* condition;
    ComparisonOp comparison;
}

/* Token declarations */
%token DOT LBRACK RBRACK PIPE LPAREN RPAREN COLON ERROR
%token SELECT
%token EQ NEQ GT LT GTE LTE
%token <number> NUMBER
%token <string> IDENT

/* Non-terminal types */
%type <node> query pipeline operation simple_operation
%type <condition> condition
%type <comparison> comparison_op

/* Operator precedence (lowest to highest) */
%left PIPE
%left DOT

/* Starting symbol */
%start query

%%

/**
 * A query can be a pipeline of operations.
 */
query:
    pipeline                    { query_result = $1; $$ = $1; }
    ;

/**
 * A pipeline is one or more operations connected by pipes.
 */
pipeline:
    operation                   { $$ = $1; }
    | operation PIPE pipeline   {
        /* Create a pipe node */
        QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
        node->type = QUERY_PIPE;
        node->data.pipe.left = $1;
        node->data.pipe.right = $3;
        node->next = NULL;
        $$ = node;
    }
    ;

/**
 * An operation is either a simple operation or a select filter.
 */
operation:
    simple_operation            { $$ = $1; }
    | SELECT LPAREN condition RPAREN {
        /* Select filter operation */
        QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
        node->type = QUERY_SELECT;
        node->data.condition = $3;
        node->next = NULL;
        $$ = node;
    }
    ;

/**
 * Simple operations that can be chained.
 */
simple_operation:
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
    | DOT LBRACK RBRACK         {
        /* Array iteration: .[] */
        QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
        node->type = QUERY_ARRAY_ITER;
        node->next = NULL;
        $$ = node;
    }
    | LBRACK NUMBER RBRACK      {
        /* Array index */
        QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
        node->type = QUERY_INDEX;
        node->data.index = (int)$2;
        node->next = NULL;
        $$ = node;
    }
    | LBRACK NUMBER COLON NUMBER RBRACK {
        /* Array slice: [start:end] */
        QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
        node->type = QUERY_SLICE;
        node->data.slice.start = (int)$2;
        node->data.slice.end = (int)$4;
        node->next = NULL;
        $$ = node;
    }
    | LBRACK COLON NUMBER RBRACK {
        /* Array slice from beginning: [:end] */
        QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
        node->type = QUERY_SLICE;
        node->data.slice.start = 0;
        node->data.slice.end = (int)$3;
        node->next = NULL;
        $$ = node;
    }
    | LBRACK NUMBER COLON RBRACK {
        /* Array slice to end: [start:] */
        QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
        node->type = QUERY_SLICE;
        node->data.slice.start = (int)$2;
        node->data.slice.end = -1;  // -1 means "to the end"
        node->next = NULL;
        $$ = node;
    }
    | simple_operation simple_operation {
        /* Chain operations */
        $1->next = $2;
        $$ = $1;
    }
    ;

/**
 * A condition for select() filtering.
 * Format: .field comparison_op number
 */
condition:
    DOT IDENT comparison_op NUMBER {
        ConditionExpr* cond = (ConditionExpr*)malloc(sizeof(ConditionExpr));
        
        /* Create a sub-query node for the field access */
        QueryNode* field_node = (QueryNode*)malloc(sizeof(QueryNode));
        field_node->type = QUERY_FIELD;
        field_node->data.field = $2;
        field_node->next = NULL;
        
        cond->left = field_node;
        cond->op = $3;
        cond->value = $4;
        $$ = cond;
    }
    ;

/**
 * Comparison operators.
 */
comparison_op:
    GT                          { $$ = CMP_GT; }
    | LT                        { $$ = CMP_LT; }
    | EQ                        { $$ = CMP_EQ; }
    | GTE                       { $$ = CMP_GTE; }
    | LTE                       { $$ = CMP_LTE; }
    | NEQ                       { $$ = CMP_NEQ; }
    ;

%%

/**
 * Error handler for the query parser.
 */
void query_yyerror(const char* s) {
    fprintf(stderr, "Query Parse Error: %s\n", s);
}
