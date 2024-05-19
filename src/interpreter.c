#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"
#include "parser.h"
#include "evaluator.h"


char interpret(char const *code, char **error_message, EvaluatorContext *context) {
    // Tokenize 
    TokenizerState tokenizer_state = init_tokenizer_state(code);
    char error = tokenize(&tokenizer_state);
    if (error) {
        *error_message = strdup(tokenizer_state.error_message);
        for (size_t i = 0; i < tokenizer_state.parsed_tokens_length; ++i) {
            delete_token(tokenizer_state.parsed_tokens+i);
        }
        free(tokenizer_state.parsed_tokens);
        return error;
    }
    Token *tokens = tokenizer_state.parsed_tokens; // ownership transfer 
    size_t num_tokens = tokenizer_state.parsed_tokens_length;

    // Parse
    ASTNode root = parse_ast(tokens, num_tokens);
    if (root.node_type == INVALID) {
        *error_message = "Syntax error";

        // free mempory
        for (size_t i = 0; i < num_tokens; ++i) {
            delete_token(tokens + i);
        }
        free(tokens);
        delete_node(&root);

        return 1;
    }

    // Evaluate 
    *context = evaluate(&root);
    if (context->error_code) {
        *error_message = strdup(context->error_message);
    }

    // free memory 
    for (size_t i = 0; i < num_tokens; ++i) {
        delete_token(tokens + i);
    }
    free(tokens);
    delete_node(&root);
    
    return context->error_code;
}