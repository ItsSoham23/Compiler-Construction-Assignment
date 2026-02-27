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
     TK_COMMENT,
     TK_FIELDID,
     TK_ID,
     TK_NUM,
     TK_RNUM,
     TK_FUNID,
     TK_RUID,
     TK_WITH,
     TK_PARAMETERS,
     TK_END,
     TK_WHILE,
     TK_UNION,
     TK_ENDUNION,
     TK_DEFINETYPE,
     TK_AS,
     TK_TYPE,
     TK_MAIN,
     TK_GLOBAL,
     TK_PARAMETER,
     TK_LIST,
     TK_SQL,
     TK_SQR,
     TK_INPUT,
     TK_OUTPUT,
     TK_INT,
     TK_REAL,
     TK_COMMA,
     TK_SEM,
     TK_COLON,
     TK_DOT,
     TK_ENDWHILE,
     TK_OP,
     TK_CL,
     TK_IF,
     TK_THEN,
     TK_ENDIF,
     TK_PLUS,
     TK_MINUS,
     TK_MUL,
     TK_DIV,
     TK_LT,
     TK_LE,
     TK_GT,
     TK_GE,
     TK_EQ,
     TK_NE,
     TK_AND,
     TK_OR,
     TK_NOT,
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
