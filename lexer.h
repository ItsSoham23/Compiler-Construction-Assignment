#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Token types
typedef enum {
    TK_WHITESPACE,
    TK_COMMENT,
    TK_KEYWORD,
    TK_FIELDID,
    TK_ID,
    TK_FUNID,
    TK_RUID,
    TK_NUM,
    TK_RNUM,
    TK_ASSIGNOP,
    TK_REL_OP,
    TK_LOG_OP,
    TK_SINGLE,
    TK_ERROR,
    TK_EOF
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char* lexeme;
    int line;
} Token;

// Lexer state
typedef struct {
    FILE* input;
    int line;
    int pos;
    int curr_char;
} Lexer;

// Lexer API
Lexer* lexer_init(const char* filename);
Token lexer_next_token(Lexer* lexer);
void lexer_free(Lexer* lexer);
void token_free(Token* token);

#endif // LEXER_H
