
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"

// Used for logging
const char *TokeTypeNames[] = {
    "PLUS",
    "MINUS",
    "DIV",
    "MULT",
    "ROUND_OPEN",
    "ROUND_CLOSE",
    "CURLY_OPEN",
    "CURLY_CLOSE",
    "SQUARE_OPEN",
    "SQUARE_CLOSE",
    "SEMICOLON", 
    "COMMA",
    "EQUAL",
    "DOUBLE_EQUAL",
    //"DOUBLE_QUOTES", TODO: adding string support
    "IF",
    "ELSE",
    "FN",
    "LET",
    "RETURN",
    "PRINT",
    "NUMERIC_LITERAL",
    "IDENTIFIER",
};

void delete_token(Token *token) {
    free(token->token_value);
}

size_t num_digits(size_t x) {
    if (x == 0) return 1;
    size_t res = 0;
    while(x > 0) {
        x/=10;
        ++res;
    }
    return res;
}

char *invalid_character_error_message(size_t pos, char c) {
    char *error_message = malloc(33 + num_digits(pos));
    if (error_message != NULL)
        sprintf(error_message, "Invalid character at position %ld: %c", pos, c);
    return error_message;
}

TokenizerState init_tokenizer_state(char const *code) {
    int parsed_tokens_capacity = 10;
    TokenizerState tokenizer_state = {
        .parsed_tokens = malloc(parsed_tokens_capacity * sizeof(Token)),
        .parsed_tokens_capacity = parsed_tokens_capacity,
        .parsed_tokens_length = 0,
        .code = code,
        .code_start = code
    };
    return tokenizer_state;
}

void tokenizer_state_adjust_capacity(TokenizerState *tokenizer_state) {
     if (tokenizer_state->parsed_tokens_capacity == tokenizer_state->parsed_tokens_length) {
        tokenizer_state->parsed_tokens_capacity *= 2;
        tokenizer_state->parsed_tokens = realloc(
            tokenizer_state->parsed_tokens, 
            tokenizer_state->parsed_tokens_capacity * sizeof(Token)
        );
     }
}

char is_whitespace(char c) {
    return c == ' ' || c == '\n' || c == '\t';
}

char is_alphabetical(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

char is_numeric(char c) {
    return c >= '0' && c <= '9';
}

char is_alphanumeric(char c) {
    return is_alphabetical(c) || is_numeric(c);
}

void parse_next_token(TokenizerState *tokenizer_state) {
    while(is_whitespace(tokenizer_state->code[0])) {
        ++tokenizer_state->code;
        continue;
    }
    
    if (tokenizer_state->code[0] == '\0') return; 

    Token next_token = {.token_type = IDENTIFIER, .token_value = NULL};

    if (tokenizer_state->code[0] == '+') next_token.token_type = PLUS;
    else if (tokenizer_state->code[0] == '-') next_token.token_type = MINUS;
    else if (tokenizer_state->code[0] == '*') next_token.token_type = MULT;
    else if (tokenizer_state->code[0] == '/') next_token.token_type = DIV;
    else if (tokenizer_state->code[0] == '(') next_token.token_type = ROUND_OPEN;
    else if (tokenizer_state->code[0] == ')') next_token.token_type = ROUND_CLOSE;
    else if (tokenizer_state->code[0] == '{') next_token.token_type = CURLY_OPEN;
    else if (tokenizer_state->code[0] == '}') next_token.token_type = CURLY_CLOSE;
    else if (tokenizer_state->code[0] == '[') next_token.token_type = SQUARE_OPEN;
    else if (tokenizer_state->code[0] == ']') next_token.token_type = SQUARE_CLOSE;
    else if (tokenizer_state->code[0] == ';') next_token.token_type = SEMICOLON;
    else if (tokenizer_state->code[0] == ',') next_token.token_type = COMMA;
    else if (tokenizer_state->code[0] == '=') {
        if(tokenizer_state->code[1] == '=') {
            next_token.token_type = DOUBLE_EQUAL;
            ++tokenizer_state->code;
        }
        else {
            next_token.token_type = EQUAL;
        }
    }

    // The next token has more than a single char in its value
    if (next_token.token_type == IDENTIFIER) {
        char has_alphabetical = 0;
        char const *code_start = tokenizer_state->code;
        while (is_alphanumeric(tokenizer_state->code[0])) {
            if (is_alphabetical(tokenizer_state->code[0])) has_alphabetical = 1;
            ++tokenizer_state->code;
        }
        if (tokenizer_state->code == code_start) {
            tokenizer_state->error_code = TOKENIZER_INVALID_CHARACTER;
            tokenizer_state->error_message = invalid_character_error_message(
                tokenizer_state->code - tokenizer_state->code_start, 
                tokenizer_state->code[0]
            );
            return;
        }
        
        next_token.token_value = malloc(tokenizer_state->code - code_start + 1);
        strncpy(next_token.token_value, code_start, tokenizer_state->code - code_start);
        next_token.token_value[tokenizer_state->code - code_start] = '\0';

        if (!has_alphabetical) next_token.token_type = NUMERIC_LITERAL;
        else if (strcmp(next_token.token_value, "imagine") == 0) next_token.token_type = IF;
        else if (strcmp(next_token.token_value, "bummer") == 0) next_token.token_type = ELSE;
        else if (strcmp(next_token.token_value, "fn") == 0) next_token.token_type = FN;
        else if (strcmp(next_token.token_value, "suppose") == 0) next_token.token_type = LET;
        else if (strcmp(next_token.token_value, "checkit") == 0) next_token.token_type = RETURN;
        else if (strcmp(next_token.token_value, "vomit") == 0) next_token.token_type = PRINT;

        //no need to save the token value for keywords 
        if (next_token.token_type != IDENTIFIER && next_token.token_type != NUMERIC_LITERAL) {
            free(next_token.token_value);
            next_token.token_value = NULL;
        }
    }
    else {
        ++tokenizer_state->code;
    }

    tokenizer_state_adjust_capacity(tokenizer_state);
    tokenizer_state->parsed_tokens[tokenizer_state->parsed_tokens_length++] = next_token;
    return;
}

char tokenize(TokenizerState *tokenizer_state) {
    while(tokenizer_state->code[0] != '\0') {
        parse_next_token(tokenizer_state);
        if (tokenizer_state->error_code) {
            return tokenizer_state->error_code;
        }
    }
    return TOKENIZER_PASS;
}