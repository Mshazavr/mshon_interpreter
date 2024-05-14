#include "parser.h"
#include "../tokenizer/tokenizer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


// Used for logging
char *NodeTypeNames[] = {
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
            printf("%d, ", node->operators[i]);
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

void cleanup_node(struct Node node) {
    return;
}

void cleanup_double_array(char **args, int args_length) {
    for (int i = 0; i < args_length; ++i) free(args[i]);
    free(args);
}

// Expression Parsers 
struct Node parse_number_or_variable(struct Token *tokens, int num_tokens, int *token_pos) {
    struct Node node = { .node_type = 0 };
    if (
        *token_pos >= num_tokens || 
        (
            tokens[*token_pos].token_type != NUMERIC_LITERAL && 
            tokens[*token_pos].token_type != IDENTIFIER
        )
    ) return INVALID_NODE;
    if (tokens[*token_pos].token_type == NUMERIC_LITERAL) node.node_type = NUMBER;
    if (tokens[*token_pos].token_type == IDENTIFIER) node.node_type = VARIABLE;
    node.value = malloc(strlen(tokens[*token_pos].token_value) + 1);
    strcpy(node.value, tokens[*token_pos].token_value);

    *token_pos += 1;
    return node;
}

struct Node parse_bracket_expression(struct Token *tokens, int num_tokens, int *token_pos) {
    if (*token_pos >= num_tokens || tokens[*token_pos].token_type != ROUND_OPEN) {
        return INVALID_NODE;
    }
    *token_pos += 1;
    
    struct Node child_node = parse_expression(tokens, num_tokens, token_pos);
    
    if (*token_pos >= num_tokens || tokens[*token_pos].token_type != ROUND_CLOSE) {
        return INVALID_NODE;
    }
    *token_pos += 1;

    return child_node;
}

struct Node parse_expression(struct Token *tokens, int num_tokens, int *token_pos) {
    struct Node *children_buffer = malloc(10 * sizeof(struct Node));
    int children_length = 0;
    int children_capacity = 10;

    enum TokenType *operators_buffer = malloc(10 * sizeof(enum TokenType));
    int operators_length = 0;
    int operators_capacity = 10;

    while(1) {
        // Find operands (and operators) until hitting a wall
        
        if (*token_pos >= num_tokens) break;
        
        // Parse the next operand
        struct Node next_node;
        if (tokens[*token_pos].token_type == ROUND_OPEN) {
            next_node = parse_bracket_expression(tokens, num_tokens, token_pos);
        }
        else if (
            tokens[*token_pos].token_type == NUMERIC_LITERAL || 
            tokens[*token_pos].token_type == IDENTIFIER
        ) {
            next_node = parse_number_or_variable(tokens, num_tokens, token_pos);
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
        if (
            tokens[*token_pos].token_type == PLUS ||
            tokens[*token_pos].token_type == MINUS ||
            tokens[*token_pos].token_type == MULT ||
            tokens[*token_pos].token_type == DIV
        ) {
            if (operators_capacity == operators_length) {
                operators_capacity *= 2;
                operators_buffer = realloc(operators_buffer, operators_capacity * sizeof(enum TokenType));
            }
            operators_buffer[operators_length++] = tokens[*token_pos].token_type;
            *token_pos += 1;
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
struct Node parse_declaration(struct Token *tokens, int num_tokens, int *token_pos) {
    // LET IDENTIFER EQUAL <statement> SEMICOLON 
    
    if (num_tokens - *token_pos < 3) return INVALID_NODE;
    if (
        tokens[*token_pos].token_type != LET || 
        tokens[*token_pos+1].token_type != IDENTIFIER ||
        tokens[*token_pos+2].token_type != EQUAL
    ) return INVALID_NODE;

    *token_pos += 1; // skipping LET
    struct Node first_child = parse_number_or_variable(tokens, num_tokens, token_pos);
    
    *token_pos += 1; // skipping EQUAL
    struct Node second_child = parse_expression(tokens, num_tokens, token_pos);

    if (first_child.node_type == INVALID ||  second_child.node_type == INVALID) {
        return INVALID_NODE;
    }

    if (*token_pos >= num_tokens || tokens[*token_pos].token_type != SEMICOLON) {
        return INVALID_NODE;
    }

    *token_pos += 1; // skipping SEMICOLON
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


struct Node parse_assignment(struct Token *tokens, int num_tokens, int *token_pos) {
    // IDENTIFER EQUAL <statement> SEMICOLON 
    
    if (num_tokens - *token_pos < 2) return INVALID_NODE;
    if (
        tokens[*token_pos].token_type != IDENTIFIER ||
        tokens[*token_pos+1].token_type != EQUAL
    ) return INVALID_NODE;

    struct Node first_child = parse_number_or_variable(tokens, num_tokens, token_pos);
    
    *token_pos += 1; // skipping EQUAL
    struct Node second_child = parse_expression(tokens, num_tokens, token_pos);

    if (first_child.node_type == INVALID ||  second_child.node_type == INVALID) {
        return INVALID_NODE;
    }

    if (*token_pos >= num_tokens || tokens[*token_pos].token_type != SEMICOLON) {
        return INVALID_NODE;
    }

    *token_pos += 1; // skipping SEMICOLON
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


struct Node parse_return_stmt(struct Token *tokens, int num_tokens, int *token_pos) {
    // RETURN <expression> ;
    if (*token_pos >= num_tokens || tokens[*token_pos].token_type != RETURN) {
        return INVALID_NODE;
    }

    *token_pos += 1; // skipping RETURN 
    struct Node child = parse_expression(tokens, num_tokens, token_pos);

    if (*token_pos >= num_tokens || tokens[*token_pos].token_type != SEMICOLON) {
        return INVALID_NODE;
    }

    *token_pos += 1; // skipping SEMICOLON
    struct Node *children = malloc(sizeof(struct Node));
    children[0] = child;
    struct Node node = {
        .node_type = RETURN_STMT,
        .children_length = 1,
        .children = children
    };
    return node;
}


struct Node parse_print_stmt(struct Token *tokens, int num_tokens, int *token_pos) {
    // PRINT <expression> ;
    if (*token_pos >= num_tokens || tokens[*token_pos].token_type != PRINT) {
        return INVALID_NODE;
    }

    *token_pos += 1; // skipping RETURN 
    struct Node child = parse_expression(tokens, num_tokens, token_pos);
    
    if (*token_pos >= num_tokens || tokens[*token_pos].token_type != SEMICOLON) {
        return INVALID_NODE;
    }

    *token_pos += 1; // skipping SEMICOLON
    struct Node *children = malloc(sizeof(struct Node));
    children[0] = child;
    struct Node node = {
        .node_type = PRINT_STMT,
        .children_length = 1,
        .children = children
    };
    return node;
}


struct Node parse_if_else_stmt(struct Token *tokens, int num_tokens, int *token_pos) {
    // IF ROUND_OPEN <expression> ROUND_CLOSE CURLY_OPEN <stmt_sequence> CURLY_CLOSE
    // ELSE CURLY_OPEN <stmt_sequence> CURLY_CLOSE <---- this line is optional 

    if (
        num_tokens - *token_pos < 2 || 
        tokens[*token_pos].token_type != IF ||
        tokens[*token_pos+1].token_type != ROUND_OPEN
    ) return INVALID_NODE;

    *token_pos += 2; // skipping IF ROUND_OPEN 
    struct Node first_child = parse_expression(tokens, num_tokens, token_pos);
    if (first_child.node_type == INVALID) return INVALID_NODE;

    if (
        num_tokens - *token_pos < 2 || 
        tokens[*token_pos].token_type != ROUND_CLOSE ||
        tokens[*token_pos+1].token_type != CURLY_OPEN
    ) return INVALID_NODE; 

    *token_pos += 2; // skipping ROUND_CLOSE CURLY_OPEN 
    struct Node second_child = parse_stmt_sequence(tokens, num_tokens, token_pos);
    if (second_child.node_type == INVALID) return INVALID_NODE;

    if (*token_pos >= num_tokens || tokens[*token_pos].token_type != CURLY_CLOSE) {
        return INVALID_NODE;
    }
    *token_pos += 1; // skipping CURLY_CLOSE 

    // Parse the ELSE block 
    if (*token_pos < num_tokens && tokens[*token_pos].token_type == ELSE) {
        *token_pos += 1; // skipping ELSE
        if (*token_pos >= num_tokens || tokens[*token_pos].token_type != CURLY_OPEN) {
            return INVALID_NODE;
        }

        *token_pos += 1; // skipping CURLY_OPEN 
        struct Node third_child = parse_stmt_sequence(tokens, num_tokens, token_pos);
        if (third_child.node_type == INVALID) return INVALID_NODE;
        
        if (*token_pos >= num_tokens || tokens[*token_pos].token_type != CURLY_CLOSE) {
            return INVALID_NODE;
        }
        *token_pos += 1; // skipping CURLY_CLOSE 

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


struct Node parse_function(struct Token *tokens, int num_tokens, int *token_pos) {
    // FN IDENTIFIER ROUND_OPEN <comma _separated identifiers> ROUND_CLOSE
    // CURLY_OPEN <stmt_sequence> CURLY_CLOSE 

    if (
        num_tokens - *token_pos < 3 || 
        tokens[*token_pos].token_type != FN || 
        tokens[*token_pos+1].token_type != IDENTIFIER || 
        tokens[*token_pos+2].token_type != ROUND_OPEN 
    ) return INVALID_NODE;

    int function_name_token_pos = *token_pos+1;

    // parse the arguments 
    *token_pos += 3; // skipping FN IDENTIFIER ROUND_OPEN
    if (*token_pos >= num_tokens) return INVALID_NODE;
    char **args = malloc(10 * sizeof(void*));
    int args_length = 0;
    int args_capacity = 10;
    if (tokens[*token_pos].token_type == ROUND_CLOSE) {
        *token_pos += 1;
    }
    else {
        while(1) {  
            struct Node next_node = parse_number_or_variable(tokens, num_tokens, token_pos);
            
            if (next_node.node_type != VARIABLE) {
                cleanup_double_array(args, args_length);
                return INVALID_NODE;
            }
            
            if (args_length == args_capacity) {
                args_capacity *= 2;
                args = realloc(args, args_capacity * sizeof(void*));
            }
            args[args_length++] = next_node.value; // ownership transfer
            
            if (*token_pos < num_tokens && tokens[*token_pos].token_type == COMMA) { // TODO peek function
                *token_pos += 1; 
            }
            else if (*token_pos < num_tokens && tokens[*token_pos].token_type == ROUND_CLOSE) {
                *token_pos += 1;
                break;
            }
            else return INVALID_NODE;
        }
    }

    if(*token_pos == num_tokens || tokens[*token_pos].token_type != CURLY_OPEN) {
        cleanup_double_array(args, args_length);
        return INVALID_NODE;
    }
    *token_pos += 1; // skipping CURLY_OPEN

    struct Node child_node = parse_stmt_sequence(tokens, num_tokens, token_pos);
    if (child_node.node_type == INVALID) {
        cleanup_double_array(args, args_length);
        return INVALID_NODE;
    }

    //if(!advance(tokens, num_tokens, token_pos, (struct Token[]){})) TODO 
    if(*token_pos == num_tokens || tokens[*token_pos].token_type != CURLY_CLOSE) {
        cleanup_double_array(args, args_length);
        return INVALID_NODE;
    }
    *token_pos += 1; // skipping CURLY_CLOSE 

    struct Node *children = malloc(sizeof(struct Node));
    children[0] = child_node; 
    
    char *value = malloc(strlen(tokens[function_name_token_pos].token_value)+1);
    value = strcpy(value, tokens[function_name_token_pos].token_value);

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


struct Node parse_stmt_sequence(struct Token *tokens, int num_tokens, int *token_pos) {
    struct Node *children = malloc(10 * sizeof(struct Node));
    int children_length = 0;
    int children_capacity = 10;

    while(1) {
        struct Node next_node;

        if (*token_pos >= num_tokens) break;
        if (tokens[*token_pos].token_type == LET) next_node = parse_declaration(tokens, num_tokens, token_pos);
        else if (tokens[*token_pos].token_type == IDENTIFIER) next_node = parse_assignment(tokens, num_tokens, token_pos);
        else if (tokens[*token_pos].token_type == RETURN) next_node = parse_return_stmt(tokens, num_tokens, token_pos);
        else if (tokens[*token_pos].token_type == PRINT) next_node = parse_print_stmt(tokens, num_tokens, token_pos);
        else if (tokens[*token_pos].token_type == IF) next_node = parse_if_else_stmt(tokens, num_tokens, token_pos);
        else if (tokens[*token_pos].token_type == FN) next_node = parse_function(tokens, num_tokens, token_pos);
        else break; 
        
        if (next_node.node_type == INVALID) {
            return INVALID_NODE;
        }

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
    int *token_pos = malloc(sizeof(int));
    *token_pos = 0;
    struct Node result = parse_stmt_sequence(tokens, num_tokens, token_pos);\
    
    if (*token_pos != num_tokens) {
        cleanup_node(result);
        result = INVALID_NODE;
    }

    free(token_pos);
    
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
- add step function for stepping over single char and return 1 or 0 status code 
- make a struct for storing parser context (tokens, num_tokens, token_pos)
- support for empty function and if-else bodies, and empty programs
- implement cleanup_node
- description syntax errors attached to invalid nodes
- remove brackets from if else 
- add back tokentype names for logging
- add peek function (similar to step function)
- a proper TestCase struct for parser and tokenizer test cases
*/