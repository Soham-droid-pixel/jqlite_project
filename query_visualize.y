/**
 * query.y
 * 
 * Bison parser specification for the jqlite query language.
 * UPGRADED VERSION with Compiler Visualization support.
 */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_value.h"

/* Forward declarations */
void query_yyerror(const char* s);
int query_yylex(void);

/* External visualization mode flag */
extern int g_visualize_mode;
static int parse_step_count = 0;

/* Helper function to log parse steps */
void log_parse_step(const char* rule, const char* ast_node) {
    if (g_visualize_mode) {
        /* First parse step: close tokens array and start parseSteps array */
        if (parse_step_count == 0) {
            printf("],\"parseSteps\":[");
        } else {
            printf(",");
        }
        printf("{\"rule\":\"%s\",\"astNode\":\"%s\"}", rule, ast_node);
        parse_step_count++;
    }
}

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
    pipeline                    { 
        log_parse_step("query: pipeline", "QUERY_ROOT");
        query_result = $1; 
        $$ = $1; 
    }
    ;

/**
 * A pipeline is one or more operations connected by pipes.
 */
pipeline:
    operation                   { 
        log_parse_step("pipeline: operation", "SINGLE_OPERATION");
        $$ = $1; 
    }
    | operation PIPE pipeline   {
        log_parse_step("pipeline: operation PIPE pipeline", "PIPE_NODE");
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
    simple_operation            { 
        log_parse_step("operation: simple_operation", "SIMPLE_OP");
        $$ = $1; 
    }
    | SELECT LPAREN condition RPAREN {
        log_parse_step("operation: SELECT ( condition )", "SELECT_NODE");
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
        log_parse_step("simple_operation: DOT", "IDENTITY_NODE");
        /* Identity operation */
        QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
        node->type = QUERY_IDENTITY;
        node->next = NULL;
        $$ = node;
    }
    | DOT IDENT                 {
        char rule[256];
        snprintf(rule, sizeof(rule), "simple_operation: DOT IDENT(%s)", $2);
        log_parse_step(rule, "FIELD_NODE");
        
        /* Field access */
        QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
        node->type = QUERY_FIELD;
        node->data.field = $2;
        node->next = NULL;
        $$ = node;
    }
    | DOT LBRACK RBRACK         {
        log_parse_step("simple_operation: DOT [ ]", "ARRAY_ITER_NODE");
        /* Array iteration: .[] */
        QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
        node->type = QUERY_ARRAY_ITER;
        node->next = NULL;
        $$ = node;
    }
    | LBRACK NUMBER RBRACK      {
        char rule[256];
        snprintf(rule, sizeof(rule), "simple_operation: [ NUMBER(%g) ]", $2);
        log_parse_step(rule, "INDEX_NODE");
        
        /* Array index */
        QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
        node->type = QUERY_INDEX;
        node->data.index = (int)$2;
        node->next = NULL;
        $$ = node;
    }
    | LBRACK NUMBER COLON NUMBER RBRACK {
        char rule[256];
        snprintf(rule, sizeof(rule), "simple_operation: [ %g : %g ]", $2, $4);
        log_parse_step(rule, "SLICE_NODE");
        
        /* Array slice: [start:end] */
        QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
        node->type = QUERY_SLICE;
        node->data.slice.start = (int)$2;
        node->data.slice.end = (int)$4;
        node->next = NULL;
        $$ = node;
    }
    | LBRACK COLON NUMBER RBRACK {
        char rule[256];
        snprintf(rule, sizeof(rule), "simple_operation: [ : %g ]", $3);
        log_parse_step(rule, "SLICE_NODE");
        
        /* Array slice: [:end] */
        QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
        node->type = QUERY_SLICE;
        node->data.slice.start = 0;
        node->data.slice.end = (int)$3;
        node->next = NULL;
        $$ = node;
    }
    | LBRACK NUMBER COLON RBRACK {
        char rule[256];
        snprintf(rule, sizeof(rule), "simple_operation: [ %g : ]", $2);
        log_parse_step(rule, "SLICE_NODE");
        
        /* Array slice: [start:] */
        QueryNode* node = (QueryNode*)malloc(sizeof(QueryNode));
        node->type = QUERY_SLICE;
        node->data.slice.start = (int)$2;
        node->data.slice.end = -1;
        node->next = NULL;
        $$ = node;
    }
    ;

/**
 * Condition expressions for select() filtering.
 */
condition:
    DOT IDENT comparison_op NUMBER {
        log_parse_step("condition: field comparison number", "CONDITION_EXPR");
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
 * Comparison operators for conditions.
 */
comparison_op:
    GT      { $$ = CMP_GT; }
    | LT    { $$ = CMP_LT; }
    | EQ    { $$ = CMP_EQ; }
    | GTE   { $$ = CMP_GTE; }
    | LTE   { $$ = CMP_LTE; }
    | NEQ   { $$ = CMP_NEQ; }
    ;

%%

/**
 * Error handler for the query parser.
 */
void query_yyerror(const char* s) {
    if (!g_visualize_mode) {
        fprintf(stderr, "Query Parse Error: %s\n", s);
    }
}
