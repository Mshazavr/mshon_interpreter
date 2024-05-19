#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <libgen.h>

#include "interpreter.h"

#define MAX_FILE_SIZE 1048576

char *get_directory(const char *file_path) {
    char *file_path_copy = strdup(file_path);
    char *dir_path = dirname(file_path_copy);
    char *dir_path_copy = strdup(dir_path);
    free(file_path_copy);
    return dir_path_copy;
}

void print_test_passed() {

}


void run_test(size_t test_index, char const *test_name, char const *code) {
    char *error_message;
    char exit_code = interpret(code, &error_message);
    if (exit_code) {
        printf("error message: %s\n", error_message);
    }
    printf("Exit code: %d\n", exit_code);
}


int main() {
    char *source_dir = get_directory(__FILE__);
    char test_cases_dir_path[300];
    snprintf(test_cases_dir_path, sizeof(test_cases_dir_path), "%s/test_cases", source_dir);
    free(source_dir);

    DIR *test_cases_dir;
    struct dirent *dir;
    test_cases_dir = opendir(test_cases_dir_path);

    size_t test_index = 0;
    if (test_cases_dir) {
        while((dir = readdir(test_cases_dir)) != NULL) {
            if (strstr(dir->d_name, ".shr") != NULL) {
                char filepath[600];
                snprintf(filepath, sizeof(filepath), "%s/%s", test_cases_dir_path, dir->d_name);

                FILE *file = fopen(filepath, "r");
                if (file) {
                    char *code = malloc(MAX_FILE_SIZE);
                    if (code) {
                        size_t bytes_read = fread(code, 1, MAX_FILE_SIZE - 1, file);
                        code[bytes_read] = '\0';

                        run_test(test_index, dir->d_name, code);               

                        free(code);
                    }
                    fclose(file);
                }
            }
        }
        closedir(test_cases_dir);
    }
    return 0;
}
