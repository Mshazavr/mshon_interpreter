#ifndef __TOKENIZER__
#define __TOKENIZER__

// Used for logging
extern const char *TokeTypeNames[];

enum TokenizerError {
    TOKENIZER_PASS = 0,
    TOKENIZER_INVALID_CHARACTER
};

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

typedef struct {
    enum TokenType token_type;
    char *token_value;
} Token;

typedef struct {
    Token *parsed_tokens;
    size_t parsed_tokens_length;
    size_t parsed_tokens_capacity;
    char const *code;
    char const *code_start;
    char *error_message; 
    enum TokenizerError error_code;
} TokenizerState;

void delete_token(Token *token);

TokenizerState init_tokenizer_state(char const *code);

char tokenize(TokenizerState *tokenizer_state);

#endif

/*
TODOs 
- tokenizer fails gracefully in case of unexpected char
*/