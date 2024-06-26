
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "parser.h"
#include "tokenizer.h"


// Used for logging
const char *ASTNodeTypeNames[] = {
    "NUMBER",
    "VARIABLE",
    "ARITHMETIC",
    "FUNCTION_CALL",
    "INVALID",
    "IF_ELSE_STMT",
    "FUNCTION",
    "DECLARATION",
    "ASSIGNMENT",
    "RETURN_STMT",
    "PRINT_STMT",
    "STMT_SEQUENCE",
}; 

ASTNode get_invalid_node(const enum TokenType expected_type, const ParserContext *context) {
    char *error_message;
    if (context->token_pos < context->num_tokens) {
        size_t num_bytes = (
            38 + 1 +
            strlen(TokeTypeNames[expected_type]) + 
            strlen(TokeTypeNames[context->tokens[context->token_pos].token_type])
        );

        if (context->tokens[context->token_pos].token_value) {
            num_bytes += strlen(context->tokens[context->token_pos].token_value) + 2;
            error_message = malloc(num_bytes);
            sprintf(
                error_message, 
                "Syntex error: Expected %s. Instead got: %s[%s]", 
                TokeTypeNames[expected_type],
                TokeTypeNames[context->tokens[context->token_pos].token_type],
                context->tokens[context->token_pos].token_value
            );
        }
        else {
            error_message = malloc(num_bytes);
            sprintf(
                error_message, 
                "Syntex error: Expected %s. Instead got: %s", 
                TokeTypeNames[expected_type],
                TokeTypeNames[context->tokens[context->token_pos].token_type]
            );  
        }   
    }
    else {
        error_message = malloc(50 + strlen(TokeTypeNames[expected_type]) + 1);

        sprintf(
            error_message, 
            "Syntex error: Expected %s. Instead ran out of tokens", 
            TokeTypeNames[expected_type]
        );
    }
    
    return (ASTNode){.node_type=INVALID, .error_message=error_message};
}

void delete_node(ASTNode *node) {
    free(node->prefix_operator);

    free(node->value);

    for(size_t i=0; i < node->args_length; ++i) free(node->args[i]);
    free(node->args);

    free(node->operators);
    
    for(size_t i=0; i < node->children_length; ++i) {
        delete_node(node->children+i);
    }
}


void print_indent(size_t indent_count) {
    for(size_t i = 0; i < indent_count; ++i) printf(" ");
}

void print_node(ASTNode *node, size_t indent_count) {
    print_indent(indent_count);
    printf("Node type: %s\n", ASTNodeTypeNames[node->node_type]);

    if (node->value) {
        print_indent(indent_count);
        printf("Node value: %s\n", node->value);
    }

    if (node->prefix_operator) {
        print_indent(indent_count);
        printf("Prefix Operator: %s\n", TokeTypeNames[*node->prefix_operator]);
    }

    if (node->operators) {
        print_indent(indent_count);
        printf("Node operators: [");
        for (size_t i = 0; i < node->children_length - 1; ++i) {
            printf("%s, ", TokeTypeNames[node->operators[i]]);
        }
        printf("]\n");
    }
    
    if (node->args) {
        print_indent(indent_count);
        printf("Node args: [");
        for (size_t i = 0; i < node->args_length; ++i) {
            printf("%s, ", node->args[i]);
        }
        printf("]\n");
    }

    if (node->children) {
        for (size_t i = 0; i < node->children_length; ++i) {
            print_node(node->children+i, indent_count + 4);
            
            if (i < node->children_length - 1) {
                print_indent(indent_count + 4);
                printf("---\n");
            }
        }
    }
}

void cleanup_node(ASTNode *node) {
    free(node->value);
    free(node->args);
    free(node->operators);
    for(size_t i = 0; i < node->args_length; ++i) {
        free(node->args[i]);
    }
    for(size_t i = 0; i < node->children_length; ++i) {
        cleanup_node(node->children+i);
    }
}

void cleanup_double_array(char **args, size_t args_length) {
    for (size_t i = 0; i < args_length; ++i) free(args[i]);
    free(args);
}

// Expression Parsers 
char peek(ParserContext *context, enum TokenType token_type) {
    if (context->token_pos >= context->num_tokens) return 0;
    return context->tokens[context->token_pos].token_type == token_type;
}

char step(ParserContext *context, enum TokenType token_type) {
    if (context->token_pos >= context->num_tokens) return 0;
    if (context->tokens[context->token_pos].token_type == token_type) {
        context->token_pos += 1;
        return 1;
    }
    return 0;
}

ASTNode parse_number_or_variable(ParserContext *context) {
    ASTNode node = { .node_type = 0 };
    if (!peek(context, NUMERIC_LITERAL) && !peek(context, IDENTIFIER)) {
        return get_invalid_node(IDENTIFIER, context);
    }
    if (peek(context, NUMERIC_LITERAL)) node.node_type = NUMBER;
    if (peek(context, IDENTIFIER)) node.node_type = VARIABLE;

    node.value = strdup(context->tokens[context->token_pos].token_value);

    context->token_pos += 1;
    return node;
}

ASTNode parse_function_call(ParserContext *context) {
    // IDENTIFIER ROUND_OPEN <comma separated expressions> ROUND_CLOSE

    // IDENTIFIER
    if (!peek(context, IDENTIFIER)) return get_invalid_node(IDENTIFIER, context);
    char *value = strdup(context->tokens[context->token_pos].token_value);
    context->token_pos += 1;

    // IDENTIFIER ROUND_OPEN
    if (!step(context, ROUND_OPEN)) {
        free(value);
        return get_invalid_node(ROUND_OPEN, context);
    }

    // IDENTIFIER ROUND_OPEN <comma separated expressions>
    ASTNode *children = malloc(10 * sizeof(ASTNode));
    size_t children_length = 0;
    size_t children_capacity = 10;
    while(1) {
        if (peek(context, ROUND_CLOSE)) break;
        ASTNode next_node = parse_expression(context);
        if (next_node.node_type == INVALID) {
            for (size_t i = 0; i < children_length; ++i) cleanup_node(children+i);
            free(children);
            free(value);
            return next_node;
        }

        if (children_length == children_capacity) {
            children_capacity *= 2;
            children = realloc(children, children_capacity * sizeof(ASTNode));
        }
        children[children_length++] = next_node; 
        
        if (peek(context, ROUND_CLOSE)) break;
        if (!step(context, COMMA)) {
            for (size_t i = 0; i < children_length; ++i) cleanup_node(children+i);
            free(children);
            free(value);
            return get_invalid_node(ROUND_CLOSE, context);
        }
    }

    // IDENTIFIER ROUND_OPEN <comma separated expressions> ROUND_CLOSE
    if (!step(context, ROUND_CLOSE)) {
        for (size_t i = 0; i < children_length; ++i) cleanup_node(children+i);
        free(children);
        free(value);
        return get_invalid_node(ROUND_CLOSE, context);
    }

    ASTNode node = {
        .node_type = FUNCTION_CALL,
        .value = value,
        .children_length = children_length,
        .children = realloc(children, children_length * sizeof(ASTNode))
    };

    return node;
}

ASTNode parse_bracket_expression(ParserContext *context) {
    if (!step(context, ROUND_OPEN)) return get_invalid_node(ROUND_OPEN, context);
    ASTNode child_node = parse_expression(context);
    if (!step(context, ROUND_CLOSE)) return get_invalid_node(ROUND_CLOSE, context);
    return child_node;
}

ASTNode parse_expression(ParserContext *context) {
    ASTNode *children_buffer = malloc(10 * sizeof(ASTNode));
    size_t children_length = 0;
    size_t children_capacity = 10;

    enum OperatorType *operators_buffer = malloc(10 * sizeof(enum OperatorType));
    size_t operators_length = 0;
    size_t operators_capacity = 10;
    enum OperatorType *prefix_operator = NULL;
    if (peek(context, MINUS)) {
        prefix_operator = malloc(sizeof(enum OperatorType));
        *prefix_operator = SUB_OP;
        context->token_pos += 1;
    }

    while(1) {
        // Find operands (and operators) until hitting a wall
        
        if (context->token_pos >= context->num_tokens) break;
        
        // Parse the next operand
        ASTNode next_node;
        if (peek(context, ROUND_OPEN)) next_node = parse_bracket_expression(context);
        else if (peek(context, NUMERIC_LITERAL)) next_node = parse_number_or_variable(context);
        else if (peek(context, IDENTIFIER)) {
            context->token_pos+=1;
            if(peek(context, ROUND_OPEN)) {
                context->token_pos-=1;
                next_node = parse_function_call(context);
            }
            else {
                context->token_pos-=1;
                next_node = parse_number_or_variable(context);
            }
        }
        else break;

        if (next_node.node_type == INVALID) {
            for (size_t i = 0; i < children_length; ++i) cleanup_node(children_buffer+i);
            free(children_buffer);
            free(operators_buffer);
            if (prefix_operator != NULL) free(prefix_operator); 
            return next_node;
        }
        
        // Add the operand to the buffer
        {
            if (children_length == children_capacity) {
                children_capacity *= 2;
                children_buffer = realloc(children_buffer, children_capacity * sizeof(ASTNode));
            }
            children_buffer[children_length++] = next_node;
        } 


        // Parse the next operator and add it to the operator buffer
        if (peek(context, PLUS) || peek(context, MINUS) || peek(context, MULT) || peek(context, DIV)) {
            if (operators_capacity == operators_length) {
                operators_capacity *= 2;
                operators_buffer = realloc(operators_buffer, operators_capacity * sizeof(enum OperatorType));
            }
            if (peek(context, PLUS)) operators_buffer[operators_length++] = ADD_OP;
            else if (peek(context, MINUS)) operators_buffer[operators_length++] = SUB_OP;
            else if (peek(context, MULT)) operators_buffer[operators_length++] = MULT_OP;
            else operators_buffer[operators_length++] = DIV_OP;
            context->token_pos += 1;
        }
        else break;
    }

    if (children_length == 0) {
        for (size_t i = 0; i < children_length; ++i) cleanup_node(children_buffer+i);
        free(children_buffer);
        free(operators_buffer);
        if (prefix_operator != NULL) free(prefix_operator);
        return get_invalid_node(IDENTIFIER, context); // >:)
    }
    else if (children_length == 1) {
        ASTNode result = children_buffer[0];
        result.prefix_operator = prefix_operator;
        free(children_buffer);
        free(operators_buffer);
        return result;
    }
    else {
        ASTNode result = {
            .node_type = ARITHMETIC,
            .operators = realloc(operators_buffer, operators_length * sizeof(enum OperatorType)),
            .prefix_operator = prefix_operator,
            .children = realloc(children_buffer, children_length * sizeof(ASTNode)),
            .children_length = children_length
        };
        return result;
    }
}

// Statement Parsers
ASTNode parse_declaration(ParserContext *context) {
    // LET IDENTIFER EQUAL <expression> SEMICOLON 

    // LET
    if (!step(context, LET)) return get_invalid_node(LET, context);
    
    // LET IDENTIFIER
    if (!peek(context, IDENTIFIER)) return get_invalid_node(IDENTIFIER, context);
    ASTNode first_child = parse_number_or_variable(context);
    if (first_child.node_type == INVALID) return first_child;
    
    // LET IDENTIFIER EQUAL
    if (!step(context, EQUAL)) return get_invalid_node(EQUAL, context);

    // LET IDENTIFIER EQUAL <expression>
    ASTNode second_child = parse_expression(context);
    if (second_child.node_type == INVALID) return second_child;

    // LET IDENTIFIER EQUAL <expression> SEMICOLON
    if (!step(context, SEMICOLON)) return get_invalid_node(SEMICOLON, context);
    
    ASTNode *children = malloc(2 * sizeof(ASTNode));
    children[0] = first_child;
    children[1] = second_child;
    ASTNode node = {
        .node_type = DECLARATION,
        .children_length = 2,
        .children = children
    };
    return node;
}


ASTNode parse_assignment(ParserContext *context) {
    // IDENTIFIER EQUAL <expression> SEMICOLON
    
    // IDENTIFIER
    if (!peek(context, IDENTIFIER)) return get_invalid_node(IDENTIFIER, context);
    ASTNode first_child = parse_number_or_variable(context);
    if (first_child.node_type == INVALID) return first_child; 

    // IDENTIFIER EQUAL
    if (!step(context, EQUAL)) return get_invalid_node(EQUAL, context);

    // IDENTIFIER EQUAL <expression>
    ASTNode second_child = parse_expression(context);
    if (second_child.node_type == INVALID) return second_child;

    // IDENTIFIER EQUAL <expression> SEMICOLON
    if (!step(context, SEMICOLON)) return get_invalid_node(SEMICOLON, context);

    ASTNode *children = malloc(2 * sizeof(ASTNode));
    children[0] = first_child;
    children[1] = second_child;
    ASTNode node = {
        .node_type = ASSIGNMENT,
        .children_length = 2,
        .children = children
    };
    return node;
}


ASTNode parse_return_stmt(ParserContext *context) {
    // RETURN <expression> SEMICOLON

    // RETURN
    if (!step(context, RETURN)) return get_invalid_node(RETURN, context);

    // RETURN <expression>
    ASTNode child = parse_expression(context);
    if (child.node_type == INVALID) return child;

    // RETURN <expression> SEMICOlON
    if (!step(context, SEMICOLON)) return get_invalid_node(SEMICOLON, context);

    ASTNode *children = malloc(sizeof(ASTNode));
    children[0] = child;
    ASTNode node = {
        .node_type = RETURN_STMT,
        .children_length = 1,
        .children = children
    };
    return node;
}


ASTNode parse_print_stmt(ParserContext *context) {
    // PRINT <expression> SEMICOLON

    // PRINT
    if (!step(context, PRINT)) return get_invalid_node(PRINT, context);

    // PRINT <expression>
    ASTNode child = parse_expression(context);
    if (child.node_type == INVALID) return child;

    // PRINT <expression> SEMICOlON
    if (!step(context, SEMICOLON)) return get_invalid_node(SEMICOLON, context);

    ASTNode *children = malloc(sizeof(ASTNode));
    children[0] = child;
    ASTNode node = {
        .node_type = PRINT_STMT,
        .children_length = 1,
        .children = children
    };
    return node;
}


ASTNode parse_if_else_stmt(ParserContext *context) {
    // IF <expression> CURLY_OPEN <stmt_sequence> CURLY_CLOSE
    // ELSE CURLY_OPEN <stmt_sequence> CURLY_CLOSE <---- this line is optional 

    // IF
    if (!step(context, IF)) return get_invalid_node(IF, context);

    // IF <expression>
    ASTNode first_child = parse_expression(context);
    if (first_child.node_type == INVALID) return first_child;

    // IF <expression> CURLY_OPEN
    if (!step(context, CURLY_OPEN)) return get_invalid_node(CURLY_OPEN, context);

    // IF <expression> CURLY_OPEN <stmt_sequence>
    ASTNode second_child = parse_stmt_sequence(context);
    if (second_child.node_type == INVALID) return second_child;

    // IF <expression> CURLY_OPEN <stmt_sequence> CURLY_CLOSE
    if (!step(context, CURLY_CLOSE)) return get_invalid_node(CURLY_CLOSE, context);

    // Parse the ELSE block 
    if (peek(context, ELSE)) {
        // ELSE
        context->token_pos += 1;
       
        // ELSE CURLY_OPEN
        if (!step(context, CURLY_OPEN)) return get_invalid_node(CURLY_OPEN, context);

        // ELSE CURLY_OPEN <stmt_sequence>
        ASTNode third_child = parse_stmt_sequence(context);
        if (third_child.node_type == INVALID) return third_child;
        
        // ELSE CURLY_OPEN <stmt_sequence> CURLY_CLOSE
        if (!step(context, CURLY_CLOSE)) return get_invalid_node(CURLY_CLOSE, context);

        ASTNode *children = malloc(3 * sizeof(ASTNode));
        children[0] = first_child;
        children[1] = second_child;
        children[2] = third_child;

        ASTNode node = {
            .node_type = IF_ELSE_STMT,
            .children_length = 3,
            .children = children
        };

        return node;
    }
    else {
        ASTNode *children = malloc(2 * sizeof(ASTNode));
        children[0] = first_child;
        children[1] = second_child;

        ASTNode node = {
            .node_type = IF_ELSE_STMT,
            .children_length = 2,
            .children = children
        };

        return node;
    }
}


ASTNode parse_function(ParserContext *context) {
    // FN IDENTIFIER ROUND_OPEN <comma _separated identifiers> ROUND_CLOSE
    // CURLY_OPEN <stmt_sequence> CURLY_CLOSE 

    int function_name_token_pos = context->token_pos+1;

    // FN IDENTIFIER ROUND_OPEN
    if (!step(context, FN)) return get_invalid_node(FN, context);
    if (!step(context, IDENTIFIER)) return get_invalid_node(IDENTIFIER, context);
    if (!step(context, ROUND_OPEN)) return get_invalid_node(ROUND_OPEN, context);

    char **args = malloc(10 * sizeof(void*));
    int args_length = 0;
    int args_capacity = 10;
    if (!step(context, ROUND_CLOSE)) {
        while(1) {  
            ASTNode next_node = parse_number_or_variable(context);
            
            if (next_node.node_type != VARIABLE) {
                cleanup_double_array(args, args_length);
                return get_invalid_node(IDENTIFIER, context);
            }
            
            if (args_length == args_capacity) {
                args_capacity *= 2;
                args = realloc(args, args_capacity * sizeof(void*));
            }
            args[args_length++] = next_node.value; // ownership transfer
            
            if (!step(context, COMMA)) {
                if (!step(context, ROUND_CLOSE)) {
                    cleanup_double_array(args, args_length);
                    return get_invalid_node(ROUND_CLOSE, context);
                }
                break;
            }
        }
    }

    // CURLY_OPEN
    if (!step(context, CURLY_OPEN)) return get_invalid_node(CURLY_OPEN, context);

    // CURLY_OPEN <stmt_sequence>
    ASTNode child_node = parse_stmt_sequence(context);
    if (child_node.node_type == INVALID) {
        cleanup_double_array(args, args_length);
        return child_node;
    }

    // CURLY_OPEN <stmt_sequence> CURLY_CLOSE
    if(!step(context, CURLY_CLOSE)) {
        cleanup_double_array(args, args_length);
        return get_invalid_node(CURLY_CLOSE, context);
    }

    ASTNode *children = malloc(sizeof(ASTNode));
    children[0] = child_node; 
    
    char *value = strdup(context->tokens[function_name_token_pos].token_value);

    ASTNode node = {
        .node_type = FUNCTION,
        .value = value,
        .args = realloc(args, args_length * sizeof(void*)),
        .args_length = args_length,
        .children = children, 
        .children_length = 1 
    };
    return node;
}


ASTNode parse_stmt_sequence(ParserContext *context) {
    ASTNode *children = malloc(10 * sizeof(ASTNode));
    size_t children_length = 0;
    size_t children_capacity = 10;

    while(1) {
        ASTNode next_node;

        if (peek(context, LET)) next_node = parse_declaration(context);
        else if (peek(context, IDENTIFIER)) next_node = parse_assignment(context);
        else if (peek(context, RETURN)) next_node = parse_return_stmt(context);
        else if (peek(context, PRINT)) next_node = parse_print_stmt(context);
        else if (peek(context, IF)) next_node = parse_if_else_stmt(context);
        else if (peek(context, FN)) next_node = parse_function(context);
        else break; 
        
        if (next_node.node_type == INVALID) return next_node;

        if (children_length == children_capacity) {
            children_capacity *= 2;
            children = realloc(children, children_capacity * sizeof(ASTNode));
        }
        children[children_length++] = next_node;
    }

    ASTNode node = {
        .node_type = STMT_SEQUENCE,
        .children_length = children_length, 
        .children = realloc(children, children_length * sizeof(ASTNode))
    };

    return node;
}

// Entry points
ASTNode parse_ast(Token const *tokens, int num_tokens) {
    ParserContext context = {
        .tokens = tokens,
        .num_tokens = num_tokens,
        .token_pos = 0
    };
    ASTNode result = parse_stmt_sequence(&context);
    
    if (context.token_pos != num_tokens) {
        if (result.node_type == INVALID) return result;
        else {
            cleanup_node(&result);
            result = get_invalid_node(IDENTIFIER, &context); // >>:)
        }
    }
    
    return result;
}

int safe_streq(const char *left, const char *right) {
    if (left == NULL) return right == NULL;
    if (right == NULL) return 0;
    return strcmp(left, right) == 0;
}

char ast_equal(ASTNode *left, ASTNode *right) {
    if (left->node_type != right -> node_type) return 0;
    
    if(!safe_streq(left->value, right->value)) return 0; 
    
    if (left->args_length != right->args_length) return 0;
    for (size_t i = 0; i < left->args_length; ++i) {
        if (!safe_streq(left->args[i], right->args[i])) return 0;
    }

    if (left->children_length != right->children_length) return 0; 
    for (size_t i = 0; i < left->children_length; ++i) {
        if (!ast_equal(left->children+i, right->children+i)) return 0;
    }
    
    return 1;
}
