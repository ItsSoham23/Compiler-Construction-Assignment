#ifndef PARSER_H
#define PARSER_H

#include "parserDef.h"

/* Load grammar from file */
Grammar loadGrammar(const char* filename);

/* Compute FIRST and FOLLOW sets */
FirstAndFollow ComputeFirstAndFollowSets(Grammar G);

/* Create predictive parsing table */
ParseTable createParseTable(Grammar G, FirstAndFollow F);

/* Parse input source code using predictive parser */
ParseTree parseInputSourceCode(char *testcaseFile, Grammar G, ParseTable T);

/* Print parse tree in inorder traversal */
void printParseTree(ParseTree PT, char *outfile);

#endif
