#include "lexer.h"
#include <string.h>
#include <ctype.h>

#define MAX_LEXEME_LEN 128

static int get_char(Lexer* lexer) {
    lexer->curr_char = fgetc(lexer->input);
    if (lexer->curr_char == '\n') lexer->line++;
    lexer->pos++;
    return lexer->curr_char;
}

static void unget_char(Lexer* lexer) {
    if (lexer->curr_char == '\n') lexer->line--;
    lexer->pos--;
    ungetc(lexer->curr_char, lexer->input);
}

Lexer* lexer_init(const char* filename) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    lexer->input = fopen(filename, "r");
    lexer->line = 1;
    lexer->pos = 0;
    lexer->curr_char = ' ';
    return lexer;
}

void lexer_free(Lexer* lexer) {
    if (lexer->input) fclose(lexer->input);
    free(lexer);
}

void token_free(Token* token) {
    if (token->lexeme) free(token->lexeme);
}

Token lexer_next_token(Lexer* lexer) {
    Token token;
    token.lexeme = (char*)malloc(MAX_LEXEME_LEN);
    token.line = lexer->line;
    int i = 0;

    // Skip whitespace
    while (isspace(lexer->curr_char) && lexer->curr_char != EOF) {
        get_char(lexer);
    }

    // End of file
    if (lexer->curr_char == EOF) {
        token.type = TK_EOF;
        strcpy(token.lexeme, "EOF");
        return token;
    }

    // Comments: % ... \n
    if (lexer->curr_char == '%') {
        token.type = TK_COMMENT;
        token.lexeme[i++] = lexer->curr_char;
        get_char(lexer);
        while (lexer->curr_char != '\n' && lexer->curr_char != EOF) {
            token.lexeme[i++] = lexer->curr_char;
            get_char(lexer);
        }
        token.lexeme[i] = '\0';
        get_char(lexer);
        return token;
    }

    // Keywords/Field-ID/ID: [a-z]+
    if (islower(lexer->curr_char)) {
        token.lexeme[i++] = lexer->curr_char;
        get_char(lexer);
        while (islower(lexer->curr_char)) {
            token.lexeme[i++] = lexer->curr_char;
            get_char(lexer);
        }
        token.lexeme[i] = '\0';
        token.type = TK_KEYWORD; // For now, treat as keyword
        return token;
    }

    // FunID: _[A-Za-z]+[0-9]+
    if (lexer->curr_char == '_') {
        token.lexeme[i++] = lexer->curr_char;
        get_char(lexer);
        if (isalpha(lexer->curr_char)) {
            while (isalpha(lexer->curr_char)) {
                token.lexeme[i++] = lexer->curr_char;
                get_char(lexer);
            }
            if (isdigit(lexer->curr_char)) {
                while (isdigit(lexer->curr_char)) {
                    token.lexeme[i++] = lexer->curr_char;
                    get_char(lexer);
                }
                token.lexeme[i] = '\0';
                token.type = TK_FUNID;
                return token;
            }
        }
        token.type = TK_ERROR;
        strcpy(token.lexeme, "Invalid FunID");
        return token;
    }

    // RUID: #[a-z]+
    if (lexer->curr_char == '#') {
        token.lexeme[i++] = lexer->curr_char;
        get_char(lexer);
        if (islower(lexer->curr_char)) {
            while (islower(lexer->curr_char)) {
                token.lexeme[i++] = lexer->curr_char;
                get_char(lexer);
            }
            token.lexeme[i] = '\0';
            token.type = TK_RUID;
            return token;
        }
        token.type = TK_ERROR;
        strcpy(token.lexeme, "Invalid RUID");
        return token;
    }
//test comment 
    // Numbers: digit+
    if (isdigit(lexer->curr_char)) {
        token.lexeme[i++] = lexer->curr_char;
        get_char(lexer);
        while (isdigit(lexer->curr_char)) {
            token.lexeme[i++] = lexer->curr_char;
            get_char(lexer);
        }
        if (lexer->curr_char == '.') {
            token.lexeme[i++] = lexer->curr_char;
            get_char(lexer);
            if (isdigit(lexer->curr_char)) {
                while (isdigit(lexer->curr_char)) {
                    token.lexeme[i++] = lexer->curr_char;
                    get_char(lexer);
                }
                if (lexer->curr_char == 'E') {
                    token.lexeme[i++] = lexer->curr_char;
                    get_char(lexer);
                    if (lexer->curr_char == '+' || lexer->curr_char == '-') {
                        token.lexeme[i++] = lexer->curr_char;
                        get_char(lexer);
                    }
                    if (isdigit(lexer->curr_char)) {
                        while (isdigit(lexer->curr_char)) {
                            token.lexeme[i++] = lexer->curr_char;
                            get_char(lexer);
                        }
                        token.lexeme[i] = '\0';
                        token.type = TK_RNUM;
                        return token;
                    }
                }
                token.lexeme[i] = '\0';
                token.type = TK_RNUM;
                return token;
            }
        }
        token.lexeme[i] = '\0';
        token.type = TK_NUM;
        return token;
    }

    // Single character tokens
    switch (lexer->curr_char) {
        case '[': token.type = TK_SINGLE; strcpy(token.lexeme, "["); break;
        case ']': token.type = TK_SINGLE; strcpy(token.lexeme, "]"); break;
        case '(': token.type = TK_SINGLE; strcpy(token.lexeme, "("); break;
        case ')': token.type = TK_SINGLE; strcpy(token.lexeme, ")"); break;
        case ',': token.type = TK_SINGLE; strcpy(token.lexeme, ","); break;
        case ';': token.type = TK_SINGLE; strcpy(token.lexeme, ";"); break;
        case ':': token.type = TK_SINGLE; strcpy(token.lexeme, ":"); break;
        case '.': token.type = TK_SINGLE; strcpy(token.lexeme, "."); break;
        case '+': token.type = TK_SINGLE; strcpy(token.lexeme, "+"); break;
        case '-': token.type = TK_SINGLE; strcpy(token.lexeme, "-"); break;
        case '*': token.type = TK_SINGLE; strcpy(token.lexeme, "*"); break;
        case '/': token.type = TK_SINGLE; strcpy(token.lexeme, "/"); break;
        default:
            token.type = TK_ERROR;
            snprintf(token.lexeme, MAX_LEXEME_LEN, "Unknown char: %c", lexer->curr_char);
            break;
    }
    get_char(lexer);
    return token;
}
