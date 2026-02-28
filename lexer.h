#ifndef LEXER_H
#define LEXER_H

#include "lexerDef.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Interface */
void removeComments(const char* filename);
void printTokens(const char* filename);

/* Utilities */
void printError(const char* msg);
void printLexerError(const char* msg, State* s);

/* Lexer core */
State initializeState(const char* fileName);
TokenList scan(State* s);

/* Helpers */
Token newToken(TokenType type, State* s);
TokenList newTokenList(int initialCapacity);
void appendToTokenList(Token c, TokenList* t);

/* Token type naming helper (defined in lexer.c) */
const char* tokenTypeName(TokenType type);

int match(char a, char b, const char* msg, State* s);

/* Keyword map */
Hashmap initializeKeywordMap();
unsigned int hash(const char* s);
void insertInHashmap(Hashmap* h, const char* key, TokenType token);
TokenType lookupKeyword(Hashmap* h, const char* key);

/* Character checks */
int isSmallAlpha(char c);
int isAlpha(char c);
int isNum(char c);
int isAlphaNum(char c);

#endif