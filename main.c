#include <stdio.h>
#include "tokenizer/tokenizer.h"
#include "parser/parser.h"
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "hash_table/hash_table.h"
#include "stack/stack.h"
#include "evaluator/evaluator.h"

void test_tokenizer() {
    char *code = (
        "let xd = -12;"
        "let y = 14;"
        "fn main() {"
        "    if (xd == y - 2) {"
        "        print(xd + y);"
        "    }"
        "}"
    );
    Token expected_tokens[] = {
        {LET}, 
        {IDENTIFIER, "xd"}, 
        {EQUAL}, 
        {MINUS},
        {NUMERIC_LITERAL, "12"}, 
        {SEMICOLON},
        {LET}, 
        {IDENTIFIER, "y"}, 
        {EQUAL}, 
        {NUMERIC_LITERAL, "14"}, 
        {SEMICOLON},
        {FN}, 
        {IDENTIFIER, "main"}, 
        {ROUND_OPEN}, 
        {ROUND_CLOSE}, 
        {CURLY_OPEN},
        {IF}, 
        {ROUND_OPEN}, 
        {IDENTIFIER, "xd"}, 
        {DOUBLE_EQUAL}, 
        {IDENTIFIER, "y"}, 
        {MINUS}, 
        {NUMERIC_LITERAL, "2"}, 
        {ROUND_CLOSE},
        {CURLY_OPEN},
        {PRINT}, 
        {ROUND_OPEN}, 
        {IDENTIFIER, "xd"}, 
        {PLUS}, 
        {IDENTIFIER, "y"}, 
        {ROUND_CLOSE},
        {SEMICOLON},
        {CURLY_CLOSE},
        {CURLY_CLOSE}
    };
    TokenizerState tokenizer_state = init_tokenizer_state(code);
    tokenize(&tokenizer_state);
    for (int i = 0; i < tokenizer_state.parsed_tokens_length; ++i) {
        assert(tokenizer_state.parsed_tokens[i].token_type == expected_tokens[i].token_type);
        if (tokenizer_state.parsed_tokens[i].token_value == NULL) {
            assert(expected_tokens[i].token_value == NULL);
        }
        else {
            assert(strcmp(tokenizer_state.parsed_tokens[i].token_value, expected_tokens[i].token_value) == 0);
        }
    }
}

void test_parser() {
    //char *code = "let Brazil = 42 + (gg - 1 - 12); Brazil = ((444));";
    //char *code = "if x + 1 - 2 { let brazil = mentioned; } else { cuba = 21; }";
    char *code = "fn main(x, y) { let x = -12; print x + y; return -y-x; }";
    //char *code = "let x = fff(aa, 43-1+Brazil, (your_func(Mentioned)));";
    //char *code = "";
    //char *code = "let x = -1;";
    //struct Node expected_node = {.node_type = ARITHMETIC,};
    
    // Tokenize 
    TokenizerState tokenizer_state = init_tokenizer_state(code);
    tokenize(&tokenizer_state);

    // Parse
    ASTNode root = parse_ast(tokenizer_state.parsed_tokens, tokenizer_state.parsed_tokens_length);

    //assert (ast_equal(&root, &expected_node));
    print_node(&root, 0);
    assert(root.node_type == STMT_SEQUENCE);
}

void test_hash_table() {
    HashTable ht = init_hash_table(16, sizeof(int)); 
    int value;

    value = 1;
    hash_table_set(&ht, "hi", &value); 
    assert(*(int *)hash_table_get(&ht, "hi") == 1);
    value = 2;
    hash_table_set(&ht, "my", &value); 
    value = 3;
    hash_table_set(&ht, "hi", &value); 
    value = 4;
    hash_table_set(&ht, "friend", &value); 
    value = 5;
    hash_table_set(&ht, "zz", &value);

    assert(*(int *)hash_table_get(&ht, "hi") == 3);
    assert(*(int *)hash_table_get(&ht, "my") == 2);
    assert(*(int *)hash_table_get(&ht, "friend") == 4);
    assert(*(int *)hash_table_get(&ht, "zz") == 5);
    assert(hash_table_get(&ht, "invalid key") == NULL);
}


typedef struct {
    int a;
    char b;
} A;

char cmp(A l, A r) {
    return l.a == r.a && l.b == r.b;
}

void test_stack() {
    Stack st = init_stack(2, sizeof(A));
    A e1 = {.a = 1, .b = '1'};
    A e2 = {.a = 2, .b = '2'};
    stack_push(&st, &e1);
    stack_push(&st, &e2);
    stack_push(&st, &e1);
    stack_push(&st, &e2);

    assert(cmp(*(A*)stack_top(&st), e2));
    stack_pop(&st);
    assert(cmp(*(A*)stack_top(&st), e1));
    stack_pop(&st);
    assert(cmp(*(A*)stack_top(&st), e2));
    stack_pop(&st);
    assert(cmp(*(A*)stack_top(&st), e1));

    delete_stack(&st);

    assert(e1.a == 1 && e1.b == '1');
    assert(e2.a == 2 && e2.b == '2');
}



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
    tokenize(&tokenizer_state);

    // Parse
    ASTNode root = parse_ast(tokenizer_state.parsed_tokens, tokenizer_state.parsed_tokens_length);

    // Evaluate 
    EvaluatorContext context = evaluate(&root);
    printf("Exit code: %d\n", context.error_code);
    printf("Exit messge: %s\n", context.error_message);

    //print_node(&root, 0);
}

int main() {
    test_tokenizer();
    test_parser();
    test_hash_table();
    test_stack();
    test_evaluator();
    return 0;
}