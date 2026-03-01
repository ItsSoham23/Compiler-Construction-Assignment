/*
Group 1
Romit Jain - 2023A7PS0021P
Soham Vinay Deshmukh - 2023A7PS0025P
Nishant Amarish Pradhan - 2023A7PS0030P
Devesh Saraogi - 2023A7PS0242P
*/

#ifndef PARSERDEF_H
#define PARSERDEF_H

#include "lexerDef.h"

/* Ensure compatibility: define tokenInfo as alias for Token from lexerDef.h */
typedef Token tokenInfo;

/* Number of terminal slots for FIRST/FOLLOW and parse tables. */
#ifndef NUM_TERMINALS
#define NUM_TERMINALS 100
#endif

/* ============================================================
 * Non-terminals
 * ============================================================ */
typedef enum {
    NT_PROGRAM = 0,
    NT_MAIN_FUNCTION,
    NT_OTHER_FUNCTIONS,
    NT_FUNCTION,
    NT_INPUT_PAR,
    NT_OUTPUT_PAR,
    NT_PARAMETER_LIST,
    NT_DATATYPE,
    NT_PRIMITIVE_DATATYPE,
    NT_CONSTRUCTED_DATATYPE,
    NT_REMAINING_LIST,
    NT_STMTS,
    NT_TYPE_DEFINITIONS,
    NT_ACTUAL_OR_REDEFINED,
    NT_TYPE_DEFINITION,
    NT_FIELD_DEFINITIONS,
    NT_FIELD_DEFINITION,
    NT_FIELD_TYPE,
    NT_MORE_FIELDS,
    NT_DECLARATIONS,
    NT_DECLARATION,
    NT_GLOBAL_OR_NOT,
    NT_OTHER_STMTS,
    NT_STMT,
    NT_ASSIGNMENT_STMT,
    NT_SINGLE_OR_REC_ID,
    NT_OPTION_SINGLE_CONSTRUCTED,
    NT_ONE_EXPANSION,
    NT_MORE_EXPANSIONS,
    NT_FUN_CALL_STMT,
    NT_OUTPUT_PARAMETERS,
    NT_INPUT_PARAMETERS,
    NT_ITERATIVE_STMT,
    NT_CONDITIONAL_STMT,
    NT_ELSE_PART,
    NT_IO_STMT,
    NT_ARITHMETIC_EXPRESSION,
    NT_EXP_PRIME,
    NT_TERM,
    NT_TERM_PRIME,
    NT_FACTOR,
    NT_HIGH_PREC_OPERATORS,
    NT_LOW_PREC_OPERATORS,
    NT_BOOLEAN_EXPRESSION,
    NT_VAR,
    NT_LOGICAL_OP,
    NT_RELATIONAL_OP,
    NT_RETURN_STMT,
    NT_OPTIONAL_RETURN,
    NT_ID_LIST,
    NT_MORE_IDS,
    NT_DEFINE_TYPE_STMT,
    NT_A,
    NUM_NON_TERMINALS   /* sentinel */
} NonTerminal;

/* ============================================================
 * Grammar Symbol
 * ============================================================ */
typedef enum { SYM_TERMINAL, SYM_NON_TERMINAL } SymbolKind;

typedef struct {
    SymbolKind kind;
    union {
        TokenType    terminal;
        NonTerminal  nonTerminal;
    } sym;
} GrammarSymbol;

/* ============================================================
 * Grammar Rule
 * ============================================================ */
#define MAX_RHS_LEN  16
#define MAX_RULES    200

typedef struct {
    NonTerminal   lhs;
    GrammarSymbol rhs[MAX_RHS_LEN];
    int           rhsLen;   /* 0 means epsilon */
} Rule;

typedef struct {
    Rule rules[MAX_RULES];
    int  numRules;
    int  ruleCount; /* alias for compatibility with older driver code */
} Grammar;

/* ============================================================
 * First / Follow sets
 * ============================================================ */
typedef struct {
    /* first[i] is a bitmask of terminal indices that are in FIRST of NT i */
    /* We use an array of ints as a bitset over NUM_TERMINALS terminals    */
    int firstSet [NUM_NON_TERMINALS][NUM_TERMINALS];
    int followSet[NUM_NON_TERMINALS][NUM_TERMINALS];
    int nullable [NUM_NON_TERMINALS];  /* 1 if NT can derive epsilon */
} FirstAndFollow;

/* ============================================================
 * Parse Table
 * Rows = non-terminals, Cols = terminals
 * value = rule index (into Grammar.rules[]), or -1 for error
 * ============================================================ */
#define PT_ERROR   -1
#define PT_SYNCH   -2   /* synchronisation entry */

typedef struct {
    int table[NUM_NON_TERMINALS][NUM_TERMINALS];
} ParseTable;

/* ============================================================
 * Parse Tree Node
 * ============================================================ */
typedef struct ParseTreeNode {
    /* If isLeaf == 1 it is a terminal node */
    int             isLeaf;
    /* Leaf data */
    tokenInfo       token;
    /* Non-leaf data */
    NonTerminal     nt;
    /* Tree links */
    struct ParseTreeNode *parent;
    struct ParseTreeNode *firstChild;
    struct ParseTreeNode *nextSibling;
} ParseTreeNode;

typedef struct {
    ParseTreeNode *root;
} ParseTree;

/* ============================================================
 * Non-terminal name table
 * ============================================================ */
extern const char *NT_NAMES[];   /* defined in parser.c */

#endif /* PARSERDEF_H */
