#include <stdio.h>
#include "tokenizer/tokenizer.h"
#include "parser/parser.h"
#include <string.h>
#include <assert.h>

void test_tokenizer() {
    char *code = (
        "let xd = 12;"
        "let y = 14;"
        "fn main() {"
        "    if (xd == y - 2) {"
        "        print(xd + y);"
        "    }"
        "}"
    );
    struct Token expected_tokens[] = {
        {LET}, 
        {IDENTIFIER, "xd"}, 
        {EQUAL}, 
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
    struct TokenizerState tokenizer_state = init_tokenizer_state(code);
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
    //char *code = "fn main(x, y) { let x = 12; print x + y; return y-x; }";
    char *code = "let x = fff(aa, 43-1+Brazil, (your_func(Mentioned)));";
    //char *code = "";
    //struct Node expected_node = {.node_type = ARITHMETIC,};
    
    // Tokenize 
    struct TokenizerState tokenizer_state = init_tokenizer_state(code);
    tokenize(&tokenizer_state);

    // Parse
    struct Node root = parse_ast(tokenizer_state.parsed_tokens, tokenizer_state.parsed_tokens_length);

    //assert (ast_equal(&root, &expected_node));
    print_node(&root, 0);
}


int main() {
    test_tokenizer();
    test_parser();
    return 0;
}