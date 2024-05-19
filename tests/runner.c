#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <libgen.h>
#include <assert.h>

#include "interpreter.h"
#include "evaluator.h"

#define MAX_FILE_SIZE 1048576

char *get_directory(const char *file_path) {
    char *file_path_copy = strdup(file_path);
    char *dir_path = dirname(file_path_copy);
    char *dir_path_copy = strdup(dir_path);
    free(file_path_copy);
    return dir_path_copy;
}

char *get_test_path(const char *test_name) {
    char *source_dir = get_directory(__FILE__);
    char test_cases_dir_path[300];
    snprintf(test_cases_dir_path, sizeof(test_cases_dir_path), "%s/test_cases", source_dir);
    free(source_dir);
 
    int length = snprintf(NULL, 0, "%s/%s.shr", test_cases_dir_path, test_name);
    char *full_path = malloc(length + 1);  // Allocate memory for the full path
    snprintf(full_path, length + 1, "%s/%s.shr", test_cases_dir_path, test_name);

    return full_path;
}

typedef struct {
    size_t test_index;
    const char *test_name;
    int32_t *side_effects; 
} TestCase;

TestCase TEST_CASES[5] = {
    {.test_index=0, .test_name="test0", .side_effects=(int32_t[]){1} },
    {.test_index=1, .test_name="test1", .side_effects=(int32_t[]){-5499} },
    {.test_index=2, .test_name="test2", .side_effects=(int32_t[]){2} },
    {.test_index=3, .test_name="test3", .side_effects=(int32_t[]){8} },
    {.test_index=4, .test_name="test4", .side_effects=(int32_t[]){8, -34} }
};

void print_test_verdict(TestCase *test_case, char passed) {
    printf(">>> Test id: %ld - Test name: %s -------- ", test_case->test_index, test_case->test_name);
    if (passed) {
        printf("\033[32mPASSED\033[0m\n");
    }
    else {
        printf("\033[31mFAILED\033[0m\n");
    }
}

char *get_code_from_test_case(TestCase *test_case) {
    char *filepath = get_test_path(test_case->test_name);
    FILE *file = fopen(filepath, "r");

    char *code = malloc(MAX_FILE_SIZE);
    size_t bytes_read = fread(code, 1, MAX_FILE_SIZE - 1, file);
    code[bytes_read] = '\0';       

    fclose(file);
    free(filepath);
    return code;
}

void run_test_case(TestCase *test_case) {
    char *code = get_code_from_test_case(test_case);
    char *error_message;
    EvaluatorContext context;

    char exit_code = interpret(code, &error_message, &context, 1);

    if (exit_code) {
        print_test_verdict(test_case, 0);
        printf("error message: %s\n", error_message);
        return;
    }

    char passed = 1;
    for (size_t i=0;i<context.side_effects.length; ++i) {
        int32_t output_number = *(int32_t*)stack_at(&context.side_effects, context.side_effects.length-i-1);
        if (output_number != test_case->side_effects[i]) {
            passed = 0;
            break;
        }
    }
    print_test_verdict(test_case, passed);
}


int main() {
    for (char i=0; i < 4; ++i) {
        run_test_case(TEST_CASES+i);
    }
}
