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
    IF_STMT,
    ELSE_STMT,
    FUNCTION,
    DECLARATION,
    ASSIGNMENT,
    RETURN_STMT,
    PRINT_STMT
};

struct Node { 
    enum NodeType node_type;
    char *value;

    // used for function arguments
    char **args; 
    int args_length;

    // used for arithmetic expression operands
    enum TokenType *operators; 

    struct Node *children;
    int children_length;
};

void print_node(struct Node *node, int indent_count);

// Parsing functions / FSM nodes 
struct Node parse_expression(struct Token *tokens, int num_tokens, int *token_pos);
struct Node parse_number_or_variable(struct Token *tokens, int num_tokens, int *token_pos);
struct Node parse_bracket_expression(struct Token *tokens, int num_tokens, int *token_pos);
struct Node parse_ast(struct Token *tokens, int num_tokens);

char ast_equal(struct Node *left, struct Node *right);

#endif