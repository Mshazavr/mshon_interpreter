#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "tokenizer.h"
#include "parser.h"
#include "hash_table.h"
#include "stack.h"
#include "evaluator.h"

void test_evaluator() {
    //char *code = "let Brazil = 42 + (gg - 1 - 12); Brazil = ((444));";
    //char *code = "if x + 1 - 2 { let brazil = mentioned; } else { cuba = 21; }";
    //char *code = "fn main(x, y) { let x = 12; print x + y; return y-x; }";
    //char *code = "let x = fff(aa, 43-1+Brazil, (your_func(Mentioned)));";
    char *code = "fn sum(a, b) { if (1+1) {return a+b;} else {return 0;} } let x = 12+43; let y = sum(40, 60); print -x*y+1;";
    //char *code = "let x = -1; print x;";
    //char *code = "fn sum(a, b) { return a+b; } let y = sum(4, 5); print y;";
    //struct Node expected_node = {.node_type = ARITHMETIC,};
    
    // Tokenize 
    TokenizerState tokenizer_state = init_tokenizer_state(code);
    char error = tokenize(&tokenizer_state);
    if (error) {
        printf("Tokenization failed: encountered an invalid character\n");
        return;
    }

    // Parse
    ASTNode root = parse_ast(tokenizer_state.parsed_tokens, tokenizer_state.parsed_tokens_length);
    if (root.node_type == INVALID) {
        printf("Syntax error\n");
        return;
    }

    // Evaluate 
    EvaluatorContext context = evaluate(&root);
    printf("Exit code: %d\n", context.error_code);
    printf("Exit messge: %s\n", context.error_message);

    //print_node(&root, 0);
}

int main() {
    test_evaluator();
    return 0;
}