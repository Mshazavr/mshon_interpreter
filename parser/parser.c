#include "parser.h"
#include "../tokenizer/tokenizer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


// Used for logging
const char *NodeTypeNames[] = {
    "NUMBER",
    "VARIABLE",
    "ARITHMETIC",
    "EQUALITY",
    "INVALID",
    "IF_ELSE_STMT",
    "FUNCTION",
    "DECLARATION",
    "ASSIGNMENT",
    "RETURN_STMT",
    "PRINT_STMT",
    "STMT_SEQUENCE",
};

struct Node INVALID_NODE = { .node_type = INVALID };

void print_indent(int indent_count) {
    for(int i = 0; i < indent_count; ++i) printf(" ");
}

void print_node(struct Node *node, int indent_count) {
    print_indent(indent_count);
    printf("Node type: %s\n", NodeTypeNames[node->node_type]);

    if (node->value) {
        print_indent(indent_count);
        printf("Node value: %s\n", node->value);
    }

    if (node->operators) {
        print_indent(indent_count);
        printf("Node operators: [");
        for (int i = 0; i < node->children_length - 1; ++i) {
            printf("%s, ", TokeTypeNames[node->operators[i]]);
        }
        printf("]\n");
    }
    
    if (node->args) {
        print_indent(indent_count);
        printf("Node args: [");
        for (int i = 0; i < node->args_length; ++i) {
            printf("%s, ", node->args[i]);
        }
        printf("]\n");
    }

    if (node->children) {
        for (int i = 0; i < node->children_length; ++i) {
            print_node(node->children+i, indent_count + 4);
            
            if (i < node->children_length - 1) {
                print_indent(indent_count + 4);
                printf("---\n");
            }
        }
    }
}

void cleanup_node(struct Node *node) {
    free(node->value);
    free(node->args);
    free(node->operators);
    for(int i = 0; i < node->args_length; ++i) {
        free(node->args[i]);
    }
    for(int i = 0; i < node->children_length; ++i) {
        cleanup_node(node->children+i);
    }
}

void cleanup_double_array(char **args, int args_length) {
    for (int i = 0; i < args_length; ++i) free(args[i]);
    free(args);
}

// Expression Parsers 
char peek(struct ParserContext *context, enum TokenType token_type) {
    if (context->token_pos >= context->num_tokens) return 0;
    return context->tokens[context->token_pos].token_type == token_type;
}

char step(struct ParserContext *context, enum TokenType token_type) {
    if (context->token_pos >= context->num_tokens) return 0;
    if (context->tokens[context->token_pos].token_type == token_type) {
        context->token_pos += 1;
        return 1;
    }
    return 0;
}

struct Node parse_number_or_variable(struct ParserContext *context) {
    struct Node node = { .node_type = 0 };
    if (!peek(context, NUMERIC_LITERAL) && !peek(context, IDENTIFIER)) {
        return INVALID_NODE;
    }
    if (peek(context, NUMERIC_LITERAL)) node.node_type = NUMBER;
    if (peek(context, IDENTIFIER)) node.node_type = VARIABLE;

    node.value = malloc(strlen(context->tokens[context->token_pos].token_value) + 1);
    strcpy(node.value, context->tokens[context->token_pos].token_value);

    context->token_pos += 1;
    return node;
}

struct Node parse_bracket_expression(struct ParserContext *context) {
    if (!step(context, ROUND_OPEN)) return INVALID_NODE;
    struct Node child_node = parse_expression(context);
    if (!step(context, ROUND_CLOSE)) return INVALID_NODE;
    return child_node;
}

struct Node parse_expression(struct ParserContext *context) {
    struct Node *children_buffer = malloc(10 * sizeof(struct Node));
    int children_length = 0;
    int children_capacity = 10;

    enum TokenType *operators_buffer = malloc(10 * sizeof(enum TokenType));
    int operators_length = 0;
    int operators_capacity = 10;

    while(1) {
        // Find operands (and operators) until hitting a wall
        
        if (context->token_pos >= context->num_tokens) break;
        
        // Parse the next operand
        struct Node next_node;
        if (peek(context, ROUND_OPEN)) next_node = parse_bracket_expression(context);
        else if (peek(context, NUMERIC_LITERAL) || peek(context, IDENTIFIER)) {
            next_node = parse_number_or_variable(context);
        }
        else break;
        
        // Add the operand to the buffer
        {
            if (children_length == children_capacity) {
                children_capacity *= 2;
                children_buffer = realloc(children_buffer, children_capacity * sizeof(struct Node));
            }
            children_buffer[children_length++] = next_node;
        } 


        // Parse the next operator and add it to the operator buffer
        if (peek(context, PLUS) || peek(context, MINUS) || peek(context, MULT) || peek(context, DIV)) {
            if (operators_capacity == operators_length) {
                operators_capacity *= 2;
                operators_buffer = realloc(operators_buffer, operators_capacity * sizeof(enum TokenType));
            }
            operators_buffer[operators_length++] = context->tokens[context->token_pos].token_type;
            context->token_pos += 1;
        }
        else break;
    }

    if (children_length == 0) {
        free(children_buffer);
        free(operators_buffer);
        return INVALID_NODE;
    }
    else if (children_length == 1) {
        struct Node result = children_buffer[0];
        free(children_buffer);
        free(operators_buffer);
        return result;
    }
    else {
        struct Node result = {
            .node_type = ARITHMETIC,
            .operators = realloc(operators_buffer, operators_length * sizeof(enum TokenType)),
            .children = realloc(children_buffer, children_length * sizeof(struct Node)),
            .children_length = children_length
        };
        return result;
    }
}

// Statement Parsers
struct Node parse_declaration(struct ParserContext *context) {
    // LET IDENTIFER EQUAL <expression> SEMICOLON 

    // LET
    if (!step(context, LET)) return INVALID_NODE;
    
    // LET IDENTIFIER
    if (!peek(context, IDENTIFIER)) return INVALID_NODE;
    struct Node first_child = parse_number_or_variable(context);
    if (first_child.node_type == INVALID) return INVALID_NODE;
    
    // LET IDENTIFIER EQUAL
    if (!step(context, EQUAL)) return INVALID_NODE;

    // LET IDENTIFIER EQUAL <expression>
    struct Node second_child = parse_expression(context);
    if (second_child.node_type == INVALID) return INVALID_NODE;

    // LET IDENTIFIER EQUAL <expression> SEMICOLON
    if (!step(context, SEMICOLON)) return INVALID_NODE;

    struct Node *children = malloc(2 * sizeof(struct Node));
    children[0] = first_child;
    children[1] = second_child;
    struct Node node = {
        .node_type = DECLARATION,
        .children_length = 2,
        .children = children
    };
    return node;
}


struct Node parse_assignment(struct ParserContext *context) {
    // IDENTIFIER EQUAL <expression> SEMICOLON
    
    // IDENTIFIER
    if (!peek(context, IDENTIFIER)) return INVALID_NODE;
    struct Node first_child = parse_number_or_variable(context);
    if (first_child.node_type == INVALID) return INVALID_NODE; 

    // IDENTIFIER EQUAL
    if (!step(context, EQUAL)) return INVALID_NODE;

    // IDENTIFIER EQUAL <expression>
    struct Node second_child = parse_expression(context);
    if (second_child.node_type == INVALID) return INVALID_NODE;

    // IDENTIFIER EQUAL <expression> SEMICOLON
    if (!step(context, SEMICOLON)) return INVALID_NODE;

    struct Node *children = malloc(2 * sizeof(struct Node));
    children[0] = first_child;
    children[1] = second_child;
    struct Node node = {
        .node_type = ASSIGNMENT,
        .children_length = 2,
        .children = children
    };
    return node;
}


struct Node parse_return_stmt(struct ParserContext *context) {
    // RETURN <expression> SEMICOLON

    // RETURN
    if (!step(context, RETURN)) return INVALID_NODE;

    // RETURN <expression>
    struct Node child = parse_expression(context);
    if (child.node_type == INVALID) return INVALID_NODE;

    // RETURN <expression> SEMICOlON
    if (!step(context, SEMICOLON)) return INVALID_NODE;

    struct Node *children = malloc(sizeof(struct Node));
    children[0] = child;
    struct Node node = {
        .node_type = RETURN_STMT,
        .children_length = 1,
        .children = children
    };
    return node;
}


struct Node parse_print_stmt(struct ParserContext *context) {
    // PRINT <expression> SEMICOLON

    // PRINT
    if (!step(context, PRINT)) return INVALID_NODE;

    // PRINT <expression>
    struct Node child = parse_expression(context);
    if (child.node_type == INVALID) return INVALID_NODE;

    // PRINT <expression> SEMICOlON
    if (!step(context, SEMICOLON)) return INVALID_NODE;

    struct Node *children = malloc(sizeof(struct Node));
    children[0] = child;
    struct Node node = {
        .node_type = PRINT_STMT,
        .children_length = 1,
        .children = children
    };
    return node;
}


struct Node parse_if_else_stmt(struct ParserContext *context) {
    // IF ROUND_OPEN <expression> ROUND_CLOSE CURLY_OPEN <stmt_sequence> CURLY_CLOSE
    // ELSE CURLY_OPEN <stmt_sequence> CURLY_CLOSE <---- this line is optional 

    // IF ROUND_OPEN
    if (!step(context, IF) || !step(context, ROUND_OPEN)) return INVALID_NODE;

    // IF ROUND_OPEN <expression>
    struct Node first_child = parse_expression(context);
    if (first_child.node_type == INVALID) return INVALID_NODE;

    // IF ROUND_OPEN <expression> ROUND_CLOSE CURLY_OPEN
    if (!step(context, ROUND_CLOSE) || !step(context, CURLY_OPEN)) return INVALID_NODE;

    // IF ROUND_OPEN <expression> ROUND_CLOSE CURLY_OPEN <stmt_sequence>
    struct Node second_child = parse_stmt_sequence(context);
    if (second_child.node_type == INVALID) return INVALID_NODE;

    // IF ROUND_OPEN <expression> ROUND_CLOSE CURLY_OPEN <stmt_sequence> CURLY_CLOSE
    if (!step(context, CURLY_CLOSE)) return INVALID_NODE;

    // Parse the ELSE block 
    if (peek(context, ELSE)) {
        // ELSE
        context->token_pos += 1;
       
        // ELSE CURLY_OPEN
        if (!step(context, CURLY_OPEN)) return INVALID_NODE;

        // ELSE CURLY_OPEN <stmt_sequence>
        struct Node third_child = parse_stmt_sequence(context);
        if (third_child.node_type == INVALID) return INVALID_NODE;
        
        // ELSE CURLY_OPEN <stmt_sequence> CURLY_CLOSE
        if (!step(context, CURLY_CLOSE)) return INVALID_NODE;

        struct Node *children = malloc(3 * sizeof(struct Node));
        children[0] = first_child;
        children[1] = second_child;
        children[2] = third_child;

        struct Node node = {
            .node_type = IF_ELSE_STMT,
            .children_length = 3,
            .children = children
        };

        return node;
    }
    else {
        struct Node *children = malloc(2 * sizeof(struct Node));
        children[0] = first_child;
        children[1] = second_child;

        struct Node node = {
            .node_type = IF_ELSE_STMT,
            .children_length = 2,
            .children = children
        };

        return node;
    }
}


struct Node parse_function(struct ParserContext *context) {
    // FN IDENTIFIER ROUND_OPEN <comma _separated identifiers> ROUND_CLOSE
    // CURLY_OPEN <stmt_sequence> CURLY_CLOSE 

    int function_name_token_pos = context->token_pos+1;

    // FN IDENTIFIER ROUND_OPEN
    if (!step(context, FN) || !step(context, IDENTIFIER) || !step(context, ROUND_OPEN)) {
        return INVALID_NODE;
    }

    char **args = malloc(10 * sizeof(void*));
    int args_length = 0;
    int args_capacity = 10;
    if (!step(context, ROUND_CLOSE)) {
        while(1) {  
            struct Node next_node = parse_number_or_variable(context);
            
            if (next_node.node_type != VARIABLE) {
                cleanup_double_array(args, args_length);
                return INVALID_NODE;
            }
            
            if (args_length == args_capacity) {
                args_capacity *= 2;
                args = realloc(args, args_capacity * sizeof(void*));
            }
            args[args_length++] = next_node.value; // ownership transfer
            
            if (!step(context, COMMA)) {
                if (!step(context, ROUND_CLOSE)) return INVALID_NODE;
                break;
            }
        }
    }

    // CURLY_OPEN
    if (!step(context, CURLY_OPEN)) return INVALID_NODE;

    // CURLY_OPEN <stmt_sequence>
    struct Node child_node = parse_stmt_sequence(context);
    if (child_node.node_type == INVALID) {
        cleanup_double_array(args, args_length);
        return INVALID_NODE;
    }

    // CURLY_OPEN <stmt_sequence> CURLY_CLOSE
    if(!step(context, CURLY_CLOSE)) {
        cleanup_double_array(args, args_length);
        return INVALID_NODE;
    }

    struct Node *children = malloc(sizeof(struct Node));
    children[0] = child_node; 
    
    char *value = malloc(strlen(context->tokens[function_name_token_pos].token_value)+1);
    value = strcpy(value, context->tokens[function_name_token_pos].token_value);

    struct Node node = {
        .node_type = FUNCTION,
        .value = value,
        .args = args,
        .args_length = args_length,
        .children = children, 
        .children_length = 1 
    };
    return node;
}


struct Node parse_stmt_sequence(struct ParserContext *context) {
    struct Node *children = malloc(10 * sizeof(struct Node));
    int children_length = 0;
    int children_capacity = 10;

    while(1) {
        struct Node next_node;

        if (peek(context, LET)) next_node = parse_declaration(context);
        else if (peek(context, IDENTIFIER)) next_node = parse_assignment(context);
        else if (peek(context, RETURN)) next_node = parse_return_stmt(context);
        else if (peek(context, PRINT)) next_node = parse_print_stmt(context);
        else if (peek(context, IF)) next_node = parse_if_else_stmt(context);
        else if (peek(context, FN)) next_node = parse_function(context);
        else break; 
        
        if (next_node.node_type == INVALID) return INVALID_NODE;

        if (children_length == children_capacity) {
            children_capacity *= 2;
            children = realloc(children, children_capacity * sizeof(struct Node));
        }
        children[children_length++] = next_node;
    }

    struct Node node = {
        .node_type = STMT_SEQUENCE,
        .children_length = children_length, 
        .children = realloc(children, children_capacity * sizeof(struct Node))
    };

    return node;
}

// Entry points
struct Node parse_ast(struct Token *tokens, int num_tokens) {
    struct ParserContext context = {
        .tokens = tokens,
        .num_tokens = num_tokens,
        .token_pos = 0
    };
    struct Node result = parse_stmt_sequence(&context);
    
    if (context.token_pos != num_tokens) {
        cleanup_node(&result);
        result = INVALID_NODE;
    }
    
    return result;
}

int safe_streq(const char *left, const char *right) {
    if (left == NULL) return right == NULL;
    if (right == NULL) return 0;
    return strcmp(left, right) == 0;
}

char ast_equal(struct Node *left, struct Node *right) {
    if (left->node_type != right -> node_type) return 0;
    
    if(!safe_streq(left->value, right->value)) return 0; 
    
    if (left->args_length != right->args_length) return 0;
    for (int i = 0; i < left->args_length; ++i) {
        if (!safe_streq(left->args[i], right->args[i])) return 0;
    }

    if (left->children_length != right->children_length) return 0; 
    for (int i = 0; i < left->children_length; ++i) {
        if (!ast_equal(left->children+i, right->children+i)) return 0;
    }
    
    return 1;
}


/*
TODOs 
- support for empty function and if-else bodies, and empty programs
- descriptive syntax errors attached to invalid nodes
- remove brackets from if else 
- a proper TestCase struct for parser and tokenizer test cases
- support for function call expressions
*/