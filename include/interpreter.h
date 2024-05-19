#ifndef __INTERPRETER__
#define __INTERPRETER__

#include "evaluator.h"

char interpret(char const *code, char **error_message, EvaluatorContext *context, char dry_run);

#endif

/*
TODOs
- report stack trace in case of error
*/
