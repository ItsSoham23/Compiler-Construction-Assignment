#ifndef PARSERDEF_H
#define PARSERDEF_H

#include "lexerDef.h"   // for Token, TokenType

#define EPSILON -1
#define MAX_NONTERMINALS 100
#define MAX_TERMINALS    100
#define MAX_RULES        200
#define MAX_SYMBOLS      50
#define MAX_PRODUCTIONS  200

/* ---------------- Grammar Representation ---------------- */
typedef struct {
    int lhs;              // Nonterminal index
    int rhs[MAX_RULES];   // Sequence of symbols (terminals/nonterminals)
    int rhsCount;         // Number of symbols on RHS
} Production;

typedef struct {
    Production rules[MAX_RULES];
    int ruleCount;
} Grammar;

/* ---------------- FIRST and FOLLOW Sets ---------------- */
typedef struct {
    int first[MAX_NONTERMINALS][MAX_TERMINALS];   // boolean matrix
    int follow[MAX_NONTERMINALS][MAX_TERMINALS];  // boolean matrix
} FirstAndFollow;

/* ---------------- Predictive Parsing Table ---------------- */
typedef struct {
    int table[MAX_NONTERMINALS][MAX_TERMINALS];   // entry = production index
} ParseTable;

/* ---------------- Parse Tree ---------------- */
typedef enum { TERMINAL, NONTERMINAL } SymbolType;

typedef struct Symbol {
    SymbolType type;
    int value;   // TokenType if terminal, Nonterminal enum if nonterminal
} Symbol;

typedef struct ParseTreeNode {
    Symbol symbol;
    struct ParseTreeNode* parent;
    struct ParseTreeNode** children;
    int childCount;
    Token tokenInfo;   // filled if leaf node
} ParseTreeNode;

typedef struct {
    ParseTreeNode* root;
} ParseTree;

typedef enum {
    NT_PROGRAM,                 // 0
    NT_MAINFUNCTION,            // 1
    NT_OTHERFUNCTIONS,          // 2
    NT_FUNCTION,                // 3
    NT_INPUTPAR,                // 4
    NT_OUTPUTPAR,               // 5
    NT_PARAMETERLIST,           // 6
    NT_DATATYPE,                // 7
    NT_PRIMITIVEDATATYPE,       // 8
    NT_CONSTRUCTEDDATATYPE,     // 9
    NT_REMAININGLIST,           // 10
    NT_STMTS,                   // 11
    NT_TYPEDEFINITIONS,         // 12
    NT_ACTUALORREDEFINED,       // 13 (NEW: for typeDefinition | definetypestmt)
    NT_TYPEDEFINITION,          // 14
    NT_FIELDDEFINITIONS,        // 15
    NT_FIELDDEFINITION,         // 16
    NT_FIELDTYPE,               // 17 (NEW: for primitiveDatatype | constructedDatatype)
    NT_MOREFIELDS,              // 18
    NT_DECLARATIONS,            // 19
    NT_DECLARATION,             // 20
    NT_GLOBALORNOT,             // 21
    NT_OTHERSTMTS,              // 22
    NT_STMT,                    // 23
    NT_ASSIGNMENTSTMT,          // 24
    NT_SINGLEORRECID,           // 25
    NT_CONSTRUCTEDVARIABLE,     // 26
    NT_ONEEXPANSION,            // 27
    NT_MOREEXPANSIONS,          // 28
    NT_FUNCALLSTMT,             // 29
    NT_OUTPUTPARAMETERS,        // 30
    NT_INPUTPARAMETERS,         // 31
    NT_ITERATIVESTMT,           // 32
    NT_CONDITIONALSTMT,         // 33
    NT_ELSEPART,                // 34
    NT_IOSTMT,                  // 35
    NT_ARITHMETICEXPR,          // 36
    NT_EXPPRIME,                // 37
    NT_TERM,                    // 38
    NT_TERMPRIME,               // 39
    NT_FACTOR,                  // 40
    NT_HIGHPRECEDENCEOP,        // 41
    NT_LOWPRECEDENCEOP,         // 42
    NT_BOOLEANEXPR,             // 43 (NEW: for booleanExpression)
    NT_LOGICALOP,               // 44 (NEW: for logicalOp)
    NT_RELATIONALOP,            // 45 (NEW: for relationalOp)
    NT_VAR,                     // 46 (NEW: for var)
    NT_IDLIST,                  // 47 (NEW: for idList)
    NT_MOREIDS,                 // 48 (NEW: for more_ids)
    NT_OPTIONALRETURN,          // 49 (NEW: for optionalReturn)
    NT_DEFINETYPESTMT,          // 50 (NEW: for definetypestmt)
    NT_A,                       // 51 (NEW: for A in definetypestmt)
    NT_UNKNOWN
} NonTerminal;

#endif