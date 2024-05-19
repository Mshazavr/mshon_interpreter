#include "evaluator.h"

char interpret(char const *code, char **error_message, EvaluatorContext *context, char dry_run);

/*
TODOs
- report stack trace in case of error
*/