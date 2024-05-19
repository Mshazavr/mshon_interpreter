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
    VARIABLE_EXISTS,
    UNEXPECTED_TYPE, 
    UNEXPECTED_ARGUMENTS,
    NOT_CALLABLE,
    DIVISION_BY_ZERO,
    INTERNAL
};

typedef enum {
    NUMBER_TYPE, 
    STRING_TYPE 
} EvaluationResultType;

typedef union {
    int32_t number;
    char *string;
} EvaluationResult;

typedef struct {
    Stack stack_frames;
    enum ErrorCode error_code;
    char *error_message;
    EvaluationResultType result_type;
    EvaluationResult result;
    Stack side_effects; // only type of side effect is int32_t currently 

} EvaluatorContext;

EvaluatorContext evaluate(ASTNode *node);

#endif