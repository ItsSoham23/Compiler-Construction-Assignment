#ifndef LEXER_H
#define LEXER_H

#include "lexerDef.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Interface */
typedef FILE* twinBuffer;
typedef Token tokenInfo;

FILE *getStream(FILE *fp);
tokenInfo getNextToken(twinBuffer B);
void removeComments(char *testcaseFile, char *cleanFile);

#endif