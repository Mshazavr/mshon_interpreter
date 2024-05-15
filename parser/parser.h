#ifndef __PARSER__
#define __PARSER__
 
#include "../tokenizer/tokenizer.h"

enum NodeType {
    // Expressions
    NUMBER,
    VARIABLE,
    ARITHMETIC,
    EQUALITY,

    // Error Management
    INVALID,

    // Statements
    IF_ELSE_STMT,
    FUNCTION,
    DECLARATION,
    ASSIGNMENT,
    RETURN_STMT,
    PRINT_STMT,
    STMT_SEQUENCE,
};

struct Node { 
    enum NodeType node_type;

    // used when node_type is NUMBER, VARIABLE or FUNCTION
    char *value;

    // used for function arguments
    char **args; 
    int args_length;

    // used for arithmetic expression operators
    // the length is children_length - 1
    enum TokenType *operators; 

    struct Node *children;
    int children_length;
};

struct ParserContext {
    struct Token *tokens; 
    int num_tokens;
    int token_pos;  
};


// Expression Parsers
struct Node parse_expression(struct ParserContext *context);
struct Node parse_number_or_variable(struct ParserContext *context);
struct Node parse_bracket_expression(struct ParserContext *context);

// Statement Parsers
struct Node parse_declaration(struct ParserContext *context);
struct Node parse_assignment(struct ParserContext *context);
struct Node parse_return_stmt(struct ParserContext *context);
struct Node parse_print_stmt(struct ParserContext *context);
struct Node parse_if_else_stmt(struct ParserContext *context);
struct Node parse_function(struct ParserContext *context);
struct Node parse_stmt_sequence(struct ParserContext *context);

// Entry points
struct Node parse_ast(struct Token *tokens, int num_tokens);
char ast_equal(struct Node *left, struct Node *right); 
void print_node(struct Node *node, int indent_count);

#endif