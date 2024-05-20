#ifndef __PARSER__
#define __PARSER__

#include <stdint.h> 
#include <stdlib.h>
#include "tokenizer.h"


// Used for logging 
extern const char *ASTNodeTypeNames[];

enum ASTNodeType {
    // Expressions
    NUMBER,
    VARIABLE,
    ARITHMETIC,
    FUNCTION_CALL,

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

enum OperatorType {
    ADD_OP,
    SUB_OP,
    MULT_OP,
    DIV_OP
};

struct ASTNode_s { 
    enum ASTNodeType node_type;

    // used when node_type is NUMBER, VARIABLE, FUNCTION_CALL, FUNCTION
    char *value;

    // used for function arguments
    char **args; 
    size_t args_length;

    // used for arithmetic expression operators
    // the length is children_length - 1
    enum OperatorType *operators; 
    enum OperatorType *prefix_operator;

    struct ASTNode_s *children;
    size_t children_length;

    const char *error_message;
};
typedef struct ASTNode_s ASTNode;

typedef struct {
    Token const *tokens; 
    int num_tokens;
    int token_pos;  
} ParserContext;

void delete_node(ASTNode *node);

// Expression Parsers
ASTNode parse_number_or_variable(ParserContext *context);
ASTNode parse_function_call(ParserContext *context);
ASTNode parse_bracket_expression(ParserContext *context);
ASTNode parse_expression(ParserContext *context);

// Statement Parsers
ASTNode parse_declaration(ParserContext *context);
ASTNode parse_assignment(ParserContext *context);
ASTNode parse_return_stmt(ParserContext *context);
ASTNode parse_print_stmt(ParserContext *context);
ASTNode parse_if_else_stmt(ParserContext *context);
ASTNode parse_function(ParserContext *context);
ASTNode parse_stmt_sequence(ParserContext *context);

// Entry points
ASTNode parse_ast(Token const *tokens, int num_tokens);
char ast_equal(ASTNode *left, ASTNode *right); 
void print_node(ASTNode *node, size_t indent_count);

#endif