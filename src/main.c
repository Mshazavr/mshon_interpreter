#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "tokenizer.h"
#include "parser.h"
#include "hash_table.h"
#include "stack.h"
#include "evaluator.h"
#include "interpreter.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Please provide a file path.\n");
        return 1;
    }

    char *file_path = argv[1];
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        printf("Failed to open file: %s\n", file_path);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char *code = malloc(length + 1);
    if (code == NULL) {
        printf("Failed to allocate memory for file content.\n");
        fclose(file);
        return 1;
    }

    fread(code, 1, length, file);
    code[length] = '\0';
    fclose(file);

    char *error_message;
    EvaluatorContext context;
    char exit_code = interpret(code, &error_message, &context, 0);
    if (exit_code) {
        printf("error message: %s\n", error_message);
    }

    free(code);
    return 0;
}