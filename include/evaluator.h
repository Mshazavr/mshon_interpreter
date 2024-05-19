#ifndef __EVALUATOR__
#define __EVALUATOR__


#include <stdlib.h>
#include "stack.h"
#include "hash_table.h"
#include "parser.h"

#define _INITIAL_STACK_FRAMES_CAPACITY 64
#define _INITIAL_IDENTIFIER_TABLE_CAPACITY 32

enum ErrorCode {
    PASS,
    UNDECLARED_IDENTIFIER,
    CALLABLE_IDENTIFIER_NOT_CALLED,
    VARIABLE_EXISTS,
    UNEXPECTED_TYPE, 
    UNEXPECTED_ARGUMENTS,
    NOT_CALLABLE,
    DIVISION_BY_ZERO,
    INTERNAL
};

typedef struct {
    enum {
        INT32_T_ENTRY, 
        ASTNODE_POINTER_ENTRY,
    } type; 

    union {
        int32_t number;
        const ASTNode *function_node;
    } value;
} StackFrameEntry;

typedef struct {
    Stack stack_frames;
    enum ErrorCode error_code;
    char *error_message;

    enum {
        NUMBER_TYPE, 
        STRING_TYPE 
    } result_type;

    union {
        int32_t number;
        char *string;
    } result;

    Stack side_effects; // only type of side effect is int32_t currently 
    char dry_run;

} EvaluatorContext;

EvaluatorContext evaluate(ASTNode *node, char dry_run);

#endif