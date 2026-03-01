/*
Group 1
Romit Jain - 2023A7PS0021P
Soham Vinay Deshmukh - 2023A7PS0025P
Nishant Amarish Pradhan - 2023A7PS0030P
Devesh Saraogi - 2023A7PS0242P
*/

#ifndef PARSER_H
#define PARSER_H

#include "parserDef.h"
#include "lexer.h"

/* ============================================================
 * Grammar loading
 * ============================================================ */
Grammar loadGrammar(const char *grammarFile);

/* ============================================================
 * First and Follow computation
 * ============================================================ */
FirstAndFollow computeFirstAndFollowSets(Grammar *G);

/* ============================================================
 * Parse Table construction
 * ============================================================ */
ParseTable createParseTable(Grammar *G, FirstAndFollow *F);

/* ============================================================
 * Parsing
 * ============================================================ */
ParseTree *parseInputSourceCode(const char *testcaseFile, Grammar *G,
                                ParseTable *T, FirstAndFollow *F);

/* ============================================================
 * Parse Tree printing
 * Inorder traversal:  leftmost child -> parent -> remaining siblings
 * ============================================================ */
void printParseTree(ParseTree *PT, const char *outfile);

/* ============================================================
 * Utility
 * ============================================================ */
const char *nonTerminalName(NonTerminal nt);
void        freeParseTree(ParseTreeNode *node);

#endif /* PARSER_H */
