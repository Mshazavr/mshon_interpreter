#include "parser.h"
#include "../tokenizer/tokenizer.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


char *NodeTypeNames[] = {
    "NUMBER",
    "VARIABLE",
    "ARITHMETIC",
    "EQUALITY",
    "INVALID"
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


// Parsing functions / FSM nodes 
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

struct Node parse_ast(struct Token *tokens, int num_tokens) {
    int *token_pos = malloc(sizeof(int));
    *token_pos = 0;
    struct Node result = parse_expression(tokens, num_tokens, token_pos);
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