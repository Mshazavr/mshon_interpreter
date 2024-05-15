#ifndef __TOKENIZER__
#define __TOKENIZER__

// Used for logging
extern const char *TokeTypeNames[];

enum TokenType {
    PLUS,
    MINUS,
    DIV,
    MULT,
    ROUND_OPEN,
    ROUND_CLOSE,
    CURLY_OPEN,
    CURLY_CLOSE,
    SQUARE_OPEN,
    SQUARE_CLOSE,
    SEMICOLON, 
    COMMA,
    EQUAL,
    DOUBLE_EQUAL,
    //DOUBLE_QUOTES, TODO: adding string support

    IF,
    ELSE,
    FN,
    LET,
    RETURN,
    PRINT,

    NUMERIC_LITERAL,
    IDENTIFIER,
};

struct Token {
    enum TokenType token_type;
    char *token_value;
};

struct TokenizerState {
    struct Token *parsed_tokens;
    int parsed_tokens_length;
    int parsed_tokens_capacity;
    char *code; 
};

struct TokenizerState init_tokenizer_state(char *code);
void tokenize(struct TokenizerState *tokenizer_state);

#endif