#include "evaluator.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stack.h"
#include "hash_table.h"
#include "parser.h"

char *undefined_identifier_message(char const *identifier) {
    char *error_message = malloc(25+strlen(identifier)+1);
    if (error_message != NULL)
        sprintf(error_message, "Undeclared Identifier: %s", identifier);
    return error_message;
}

char *not_callable_message(char const *identifier) {
    char *error_message = malloc(25+strlen(identifier)+1);
    if (error_message != NULL)
        sprintf(error_message, "Variable is not callable: %s", identifier);
    return error_message;
}

char *unexpected_arguments_message(char const *identifier) {
    char *error_message = malloc(42+strlen(identifier)+1);
    if (error_message != NULL)
        sprintf(error_message, "Incorrect arguments supplied to function: %s", identifier);
    return error_message;
}

char *variable_exists_message(char const *identifier) {
    char *error_message = malloc(25+strlen(identifier)+1);
    if (error_message != NULL)
        sprintf(error_message, "Variable Already Exists: %s", identifier);
    return error_message;
}

int32_t char_to_int(char const *num) {
    int result = 0;
    for(char const *p=num; *p; ++p) {
        result = result * 10 + (*p) - '0';
    }
    return result;
}

char const *search_identifier_value(EvaluatorContext *context, char const *identifer) {
    for (size_t i=0; i < context->stack_frames.length; ++i) {
        HashTable const *frame = stack_at(&context->stack_frames, i);
        char * const *value = hash_table_get(frame, identifer);
        if (value == NULL) continue;
        return *value;
    }
    return NULL;
}

char allocate_stack_frame(
    EvaluatorContext *context, 
    char **arg_names, 
    uint32_t *arg_values, 
    size_t args_length
) { 
    HashTable frame = init_hash_table(_INITIAL_IDENTIFIER_TABLE_CAPACITY, sizeof(char*));
    HashTable *new_frame = malloc(sizeof(HashTable));
    if (frame.rows == NULL || new_frame == NULL) return 0;
    memcpy(new_frame, &frame, sizeof(HashTable));

    for (size_t i=0; i < args_length; ++i) {
        uint32_t *arg = malloc(4);
        *arg = arg_values[i];
        char error = hash_table_set(new_frame, arg_names[i], &arg);
        if (error) {
            clean_hash_table(new_frame);
            free(new_frame);
            return error;
        }
    }

    char error = !stack_push(&context->stack_frames, new_frame);
    return error;
}




void evaluate_number(ASTNode const *node, EvaluatorContext *context);
void evaluate_variable(ASTNode const *node, EvaluatorContext *context);
void evaluate_arithmetic(ASTNode const *node, EvaluatorContext *context);
void evaluate_function_call(ASTNode const *node, EvaluatorContext *context);
void evaluate_expression_node(ASTNode const *node, EvaluatorContext *context);

void evaluate_declaration(ASTNode const *node, EvaluatorContext *context);
void evaluate_assignment(ASTNode const *node, EvaluatorContext *context);
void evaluate_print(ASTNode const *node, EvaluatorContext *context);
void evaluate_return(ASTNode const *node, EvaluatorContext *context);
void evaluate_if_else(ASTNode const *node, EvaluatorContext *context);
void evaluate_function(ASTNode const *node, EvaluatorContext *context);
void evaluate_statement_sequence(ASTNode const *node, EvaluatorContext *context);


/////////////////////////////
/// Expression evaluators ///
/////////////////////////////

void evaluate_number(ASTNode const *node, EvaluatorContext *context) {
    int32_t result_number = char_to_int(node->value);
    if (node->prefix_operator != NULL && *node->prefix_operator == SUB_OP) {
        result_number *= -1;
    }
    context->result_type = NUMBER_TYPE;
    context->result = (EvaluationResult){.number=result_number};
}

void evaluate_variable(ASTNode const *node, EvaluatorContext *context) {
    char const *value = search_identifier_value(context, node->value);

    if (value == NULL) {
        context->error_code = UNDECLARED_IDENTIFIER;
        context->error_message = undefined_identifier_message(node->value);
        return;
    }

    int32_t result_number = *(int32_t*)value;
    if (node->prefix_operator != NULL && *node->prefix_operator == SUB_OP) {
        result_number *= -1;
    }
    context->result_type = NUMBER_TYPE;
    context->result = (EvaluationResult){.number=result_number};
}

void evaluate_arithmetic(ASTNode const *node, EvaluatorContext *context) {
    uint32_t *child_evaluations = malloc(node->children_length * 4);
    if (child_evaluations == NULL) {
        context->error_code = INTERNAL;
        return;
    }

    for (size_t i = 0; i < node->children_length; ++i) {
        evaluate_expression_node(node->children+i, context);
        if (context->error_code) return;
        child_evaluations[i] = context->result.number;
    }

    int32_t result_number = child_evaluations[0];
    if (node->prefix_operator != NULL && *node->prefix_operator == SUB_OP) {
        result_number *= -1;
    }
    for (size_t i = 1; i < node->children_length; ++i) {
        if (node->operators[i-1] == ADD_OP) result_number += child_evaluations[i];
        else if (node->operators[i-1] == SUB_OP) result_number -= child_evaluations[i];
        else if (node->operators[i-1] == MULT_OP) result_number *= child_evaluations[i];
        else if (node->operators[i-1] == DIV_OP) result_number /= child_evaluations[i];
    }
    context->result_type = NUMBER_TYPE;
    context->result = (EvaluationResult){.number=result_number};
}

void evaluate_function_call(ASTNode const *node, EvaluatorContext *context) {
    ASTNode *function_node = (ASTNode *)search_identifier_value(context, node->value);
    // TODO: fix the bug - hash table values should point to a tagged union of int34_t or ASTNode*
    //       int pointer to ASTNode pointer is undefined behavior.

    if (function_node == NULL) {
        context->error_code = UNDECLARED_IDENTIFIER;
        context->error_message = undefined_identifier_message(node->value);
        return;
    }

    if (function_node->node_type != FUNCTION) {
        context->error_code = NOT_CALLABLE,
        context->error_message = not_callable_message(node->value);
        return;
    }

    if (node->children_length != function_node->args_length) {
        context->error_code = UNEXPECTED_ARGUMENTS;
        context->error_message = unexpected_arguments_message(node->value);
        return;
    }

    uint32_t *arg_values = malloc(node->children_length * 4);
    if (arg_values == NULL) {
        context->error_code = INTERNAL;
        return;
    }

    for (size_t i = 0; i < node->children_length; ++i) {
        evaluate_expression_node(node->children+i, context);
        if (context->error_code) return;
        arg_values[i] = context->result.number;
    }

    char error = allocate_stack_frame(
        context,
        function_node->args, 
        arg_values, 
        function_node->args_length
    );
    
    if(error) {
        context->error_code = INTERNAL;
        return;
    }

    evaluate_statement_sequence(function_node->children+0, context);
    
    stack_pop(&context->stack_frames);
    
    if (node->prefix_operator != NULL && *node->prefix_operator == SUB_OP) {
        context->result.number *= -1;
    }
}

void evaluate_expression_node(ASTNode const *node, EvaluatorContext *context) {
     if (node->node_type == NUMBER) evaluate_number(node, context);
     else if (node->node_type == VARIABLE) evaluate_variable(node, context);
     else if (node->node_type == ARITHMETIC) evaluate_arithmetic(node, context);
     else if (node->node_type == FUNCTION_CALL) evaluate_function_call(node, context);
     else context->error_code = INTERNAL; 
}


////////////////////////////
/// Statement evaluators ///
////////////////////////////

void evaluate_declaration(ASTNode const *node, EvaluatorContext *context) { 
    HashTable *current_frame = stack_top(&context->stack_frames);
    char const *name = node->children[0].value;
    
    if (hash_table_get(current_frame, name) != NULL) {
        context->error_code = VARIABLE_EXISTS;
        context->error_message = variable_exists_message(name);
        context->result = (EvaluationResult){.number=0};
        return;
    }

    evaluate_expression_node(node->children+1, context);

    int32_t *result = malloc(sizeof(int32_t));
    memcpy(result, &context->result.number, sizeof(int32_t));
    char error = hash_table_set(current_frame, name, &result);
    if (error) context->error_code = INTERNAL;
    context->result = (EvaluationResult){.number=0};
}

void evaluate_assignment(ASTNode const *node, EvaluatorContext *context) { 
    HashTable *current_frame = stack_top(&context->stack_frames);
    char *name = node->children[0].value;
    
    if (hash_table_get(current_frame, name) == NULL) {
        context->error_code = UNDECLARED_IDENTIFIER;
        context->error_message = undefined_identifier_message(name);
        context->result = (EvaluationResult){.number=0};
        return;
    }

    evaluate_expression_node(node->children+1, context);

    int32_t *result = malloc(sizeof(int32_t));
    memcpy(result, &context->result.number, sizeof(int32_t));
    char error = hash_table_set(current_frame, name, &result);
    if (error) context->error_code = INTERNAL;
    context->result = (EvaluationResult){.number=0};
}

void evaluate_return(ASTNode const *node, EvaluatorContext *context) {
    evaluate_expression_node(node->children+0, context);
}

void evaluate_print(ASTNode const *node, EvaluatorContext *context) {
    evaluate_expression_node(node->children+0, context);
    if (context->error_code) return;

    stack_push(&context->side_effects, &context->result.number);
    if (!context->dry_run) {
        printf("%d\n", context->result.number);
    }

    context->result = (EvaluationResult){.number=0};
}

void evaluate_if_else(ASTNode const *node, EvaluatorContext *context) {
    evaluate_expression_node(node->children+0, context);
    if (context->result.number != 0) {
        evaluate_statement_sequence(node->children+1, context);
    }
    else {
        if (node->children_length == 3) {
            evaluate_statement_sequence(node->children+2, context);
        }
    }
}

void evaluate_function(ASTNode const *node, EvaluatorContext *context) {  
    HashTable *current_frame = stack_top(&context->stack_frames);
    char *name = node->value;
    
    if (hash_table_get(current_frame, name) != NULL) {
        context->error_code = VARIABLE_EXISTS;
        context->error_message = variable_exists_message(name);
        context->result = (EvaluationResult){.number=0};
        return;
    }

    char error = hash_table_set(current_frame, name, &node);
    if (error) context->error_code = INTERNAL;
    context->result = (EvaluationResult){.number=0};
}

void evaluate_statement_sequence(ASTNode const *node, EvaluatorContext *context) {
    for (size_t i = 0; i < node->children_length; ++i) {
        if (node->children[i].node_type == DECLARATION) evaluate_declaration(node->children+i, context);
        else if (node->children[i].node_type == ASSIGNMENT) evaluate_assignment(node->children+i, context);
        else if (node->children[i].node_type == PRINT_STMT) evaluate_print(node->children+i, context); 
        else if (node->children[i].node_type == IF_ELSE_STMT) evaluate_if_else(node->children+i, context);
        else if (node->children[i].node_type == FUNCTION) evaluate_function(node->children+i, context);
        else { // node_type == RETURN 
            evaluate_return(node->children+i, context);
            return;
        }
        if (context->error_code) return;
    }
}

EvaluatorContext evaluate(ASTNode *node, char dry_run) {
    if (node->node_type != STMT_SEQUENCE) { 
        EvaluatorContext context = {
            .error_code = INTERNAL,
            .error_message = "Internal Error: Invalid AST node type received",
            .dry_run = dry_run
        };
        return context;
    }

    Stack stack_frames = init_stack(_INITIAL_STACK_FRAMES_CAPACITY, sizeof(HashTable));
    if (stack_frames.buffer == NULL) {
        EvaluatorContext context = {
            .error_code = INTERNAL,
            .error_message = "Internal Error: Could not allocate memory for stack frames",
            .dry_run = dry_run
        };
        return context;
    }
    
    HashTable frame = init_hash_table(_INITIAL_IDENTIFIER_TABLE_CAPACITY, sizeof(char*));
    HashTable *main_frame = malloc(sizeof(HashTable));
    if (frame.rows == NULL || main_frame == NULL) {
        EvaluatorContext context = {
            .error_code = INTERNAL,
            .error_message = "Internal Error: Could not allocate memory for main frame",
            .dry_run = dry_run
        };
        return context;
    }
    memcpy(main_frame, &frame, sizeof(HashTable));

    if (!stack_push(&stack_frames, main_frame)) {
        EvaluatorContext context = {
            .error_code = INTERNAL,
            .error_message = "Internal Error: Could not allocate memory for main frame",
            .dry_run = dry_run
        };
        return context;
    }

    Stack side_effects = init_stack(1024, sizeof(int32_t));

    EvaluatorContext context = {
        .stack_frames = stack_frames,
        .error_code = PASS,
        .side_effects = side_effects,
        .dry_run = dry_run
    };

    evaluate_statement_sequence(node, &context);
    return context;
}


/*
TODO 
- string type support (currently only supports ints). Also maybe floats in future etc...
- synthetic stress test (to test performance improvement after using bump allocation in certain places like the parser)
- better way to handle hash table storage (tagged union instead of a generic pointer potentially)
*/