#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parserDef.h"
#include "parser.h"
#include "lexer.h"

/* ---------------- Nonterminal to String Mapping ---------------- */
const char* nonTerminalToString(NonTerminal nt) {
    switch(nt) {
        case NT_PROGRAM: return "program";
        case NT_MAINFUNCTION: return "mainFunction";
        case NT_OTHERFUNCTIONS: return "otherFunctions";
        case NT_FUNCTION: return "function";
        case NT_INPUTPAR: return "input_par";
        case NT_OUTPUTPAR: return "output_par";
        case NT_PARAMETERLIST: return "parameter_list";
        case NT_DATATYPE: return "dataType";
        case NT_PRIMITIVEDATATYPE: return "primitiveDatatype";
        case NT_CONSTRUCTEDDATATYPE: return "constructedDatatype";
        case NT_REMAININGLIST: return "remaining_list";
        case NT_STMTS: return "stmts";
        case NT_TYPEDEFINITIONS: return "typeDefinitions";
        case NT_ACTUALORREDEFINED: return "actualOrRedefined";
        case NT_TYPEDEFINITION: return "typeDefinition";
        case NT_FIELDDEFINITIONS: return "fieldDefinitions";
        case NT_FIELDDEFINITION: return "fieldDefinition";
        case NT_FIELDTYPE: return "fieldType";
        case NT_MOREFIELDS: return "moreFields";
        case NT_DECLARATIONS: return "declarations";
        case NT_DECLARATION: return "declaration";
        case NT_GLOBALORNOT: return "global_or_not";
        case NT_OTHERSTMTS: return "otherStmts";
        case NT_STMT: return "stmt";
        case NT_ASSIGNMENTSTMT: return "assignmentStmt";
        case NT_SINGLEORRECID: return "singleOrRecId";
        case NT_CONSTRUCTEDVARIABLE: return "constructedVariable";
        case NT_ONEEXPANSION: return "oneExpansion";
        case NT_MOREEXPANSIONS: return "moreExpansions";
        case NT_FUNCALLSTMT: return "funCallStmt";
        case NT_OUTPUTPARAMETERS: return "outputParameters";
        case NT_INPUTPARAMETERS: return "inputParameters";
        case NT_ITERATIVESTMT: return "iterativeStmt";
        case NT_CONDITIONALSTMT: return "conditionalStmt";
        case NT_ELSEPART: return "elsePart";
        case NT_IOSTMT: return "ioStmt";
        case NT_ARITHMETICEXPR: return "arithmeticExpression";
        case NT_EXPPRIME: return "expPrime";
        case NT_TERM: return "term";
        case NT_TERMPRIME: return "termPrime";
        case NT_FACTOR: return "factor";
        case NT_HIGHPRECEDENCEOP: return "highPrecedenceOperator";
        case NT_LOWPRECEDENCEOP: return "lowPrecedenceOperator";
        case NT_BOOLEANEXPR: return "booleanExpression";
        case NT_LOGICALOP: return "logicalOp";
        case NT_RELATIONALOP: return "relationalOp";
        case NT_VAR: return "var";
        case NT_IDLIST: return "idList";
        case NT_MOREIDS: return "more_ids";
        case NT_OPTIONALRETURN: return "optionalReturn";
        case NT_DEFINETYPESTMT: return "definetypestmt";
        case NT_A: return "A";
        default: return "unknown";
    }
}

/* ---------------- Terminal Mapping ---------------- */
TokenType lookupTerminal(const char* name) {
    #define MATCH(tok) if(strcmp(name, #tok) == 0) return tok;
    MATCH(TK_ASSIGNOP) MATCH(TK_COMMENT) MATCH(TK_FIELDID) MATCH(TK_ID)
    MATCH(TK_NUM) MATCH(TK_RNUM) MATCH(TK_FUNID) MATCH(TK_RUID)
    MATCH(TK_WITH) MATCH(TK_PARAMETERS) MATCH(TK_END) MATCH(TK_WHILE)
    MATCH(TK_UNION) MATCH(TK_ENDUNION) MATCH(TK_DEFINETYPE) MATCH(TK_AS)
    MATCH(TK_TYPE) MATCH(TK_MAIN) MATCH(TK_GLOBAL) MATCH(TK_PARAMETER)
    MATCH(TK_LIST) MATCH(TK_SQL) MATCH(TK_SQR) MATCH(TK_INPUT)
    MATCH(TK_OUTPUT) MATCH(TK_INT) MATCH(TK_REAL) MATCH(TK_COMMA)
    MATCH(TK_SEM) MATCH(TK_COLON) MATCH(TK_DOT) MATCH(TK_ENDWHILE)
    MATCH(TK_OP) MATCH(TK_CL) MATCH(TK_IF) MATCH(TK_THEN)
    MATCH(TK_ENDIF) MATCH(TK_READ) MATCH(TK_WRITE) MATCH(TK_RETURN)
    MATCH(TK_PLUS) MATCH(TK_MINUS) MATCH(TK_MUL) MATCH(TK_DIV)
    MATCH(TK_CALL) MATCH(TK_RECORD) MATCH(TK_ENDRECORD) MATCH(TK_ELSE)
    MATCH(TK_AND) MATCH(TK_OR) MATCH(TK_NOT) MATCH(TK_LT)
    MATCH(TK_LE) MATCH(TK_EQ) MATCH(TK_GT) MATCH(TK_GE) MATCH(TK_NE)
    return TK_ERROR;
    #undef MATCH
}

/* ---------------- Nonterminal Lookup  Mapping ---------------- */
NonTerminal lookupNonTerminal(const char* name) {
    if (!name) return NT_UNKNOWN;

    if (strcmp(name, "program") == 0) return NT_PROGRAM;
    if (strcmp(name, "mainFunction") == 0) return NT_MAINFUNCTION;
    if (strcmp(name, "otherFunctions") == 0) return NT_OTHERFUNCTIONS;
    if (strcmp(name, "function") == 0) return NT_FUNCTION;
    if (strcmp(name, "input_par") == 0) return NT_INPUTPAR;
    if (strcmp(name, "output_par") == 0) return NT_OUTPUTPAR;
    if (strcmp(name, "parameter_list") == 0) return NT_PARAMETERLIST;
    if (strcmp(name, "dataType") == 0) return NT_DATATYPE;
    if (strcmp(name, "primitiveDatatype") == 0) return NT_PRIMITIVEDATATYPE;
    if (strcmp(name, "constructedDatatype") == 0) return NT_CONSTRUCTEDDATATYPE;
    if (strcmp(name, "remaining_list") == 0) return NT_REMAININGLIST;
    if (strcmp(name, "stmts") == 0) return NT_STMTS;
    if (strcmp(name, "typeDefinitions") == 0) return NT_TYPEDEFINITIONS;
    if (strcmp(name, "actualOrRedefined") == 0) return NT_ACTUALORREDEFINED;
    if (strcmp(name, "typeDefinition") == 0) return NT_TYPEDEFINITION;
    if (strcmp(name, "fieldDefinitions") == 0) return NT_FIELDDEFINITIONS;
    if (strcmp(name, "fieldDefinition") == 0) return NT_FIELDDEFINITION;
    if (strcmp(name, "fieldType") == 0) return NT_FIELDTYPE;
    if (strcmp(name, "moreFields") == 0) return NT_MOREFIELDS;
    if (strcmp(name, "declarations") == 0) return NT_DECLARATIONS;
    if (strcmp(name, "declaration") == 0) return NT_DECLARATION;
    if (strcmp(name, "global_or_not") == 0) return NT_GLOBALORNOT;
    if (strcmp(name, "otherStmts") == 0) return NT_OTHERSTMTS;
    if (strcmp(name, "stmt") == 0) return NT_STMT;
    if (strcmp(name, "assignmentStmt") == 0) return NT_ASSIGNMENTSTMT;
    if (strcmp(name, "singleOrRecId") == 0) return NT_SINGLEORRECID;
    if (strcmp(name, "constructedVariable") == 0) return NT_CONSTRUCTEDVARIABLE;
    if (strcmp(name, "oneExpansion") == 0) return NT_ONEEXPANSION;
    if (strcmp(name, "moreExpansions") == 0) return NT_MOREEXPANSIONS;
    if (strcmp(name, "funCallStmt") == 0) return NT_FUNCALLSTMT;
    if (strcmp(name, "outputParameters") == 0) return NT_OUTPUTPARAMETERS;
    if (strcmp(name, "inputParameters") == 0) return NT_INPUTPARAMETERS;
    if (strcmp(name, "iterativeStmt") == 0) return NT_ITERATIVESTMT;
    if (strcmp(name, "conditionalStmt") == 0) return NT_CONDITIONALSTMT;
    if (strcmp(name, "elsePart") == 0) return NT_ELSEPART;
    if (strcmp(name, "ioStmt") == 0) return NT_IOSTMT;
    if (strcmp(name, "arithmeticExpression") == 0) return NT_ARITHMETICEXPR;
    if (strcmp(name, "expPrime") == 0) return NT_EXPPRIME;
    if (strcmp(name, "term") == 0) return NT_TERM;
    if (strcmp(name, "termPrime") == 0) return NT_TERMPRIME;
    if (strcmp(name, "factor") == 0) return NT_FACTOR;
    if (strcmp(name, "highPrecedenceOperator") == 0) return NT_HIGHPRECEDENCEOP;
    if (strcmp(name, "lowPrecedenceOperator") == 0) return NT_LOWPRECEDENCEOP;
    if (strcmp(name, "booleanExpression") == 0) return NT_BOOLEANEXPR;
    if (strcmp(name, "logicalOp") == 0) return NT_LOGICALOP;
    if (strcmp(name, "relationalOp") == 0) return NT_RELATIONALOP;
    if (strcmp(name, "var") == 0) return NT_VAR;
    if (strcmp(name, "idList") == 0) return NT_IDLIST;
    if (strcmp(name, "more_ids") == 0) return NT_MOREIDS;
    if (strcmp(name, "optionalReturn") == 0) return NT_OPTIONALRETURN;
    if (strcmp(name, "definetypestmt") == 0) return NT_DEFINETYPESTMT;
    if (strcmp(name, "A") == 0) return NT_A;

    return NT_UNKNOWN;
}

/* ---------------- Utility: trim whitespace */
char* trim(char* str) {
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    char* end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

/* ---------------- Grammar Loader ---------------- */
Grammar loadGrammar(const char* filename) {
    Grammar G;
    G.ruleCount = 0;

    FILE* f = fopen(filename, "r");
    if(!f) {
        printf("Error: Could not open grammar file %s\n", filename);
        return G;
    }

    char line[512];
    int productionCount = 0;

    while(fgets(line, sizeof(line), f) && G.ruleCount < MAX_RULES) {
        // Remove comments and trim
        char* comment = strchr(line, '%');
        if(comment) *comment = '\0';

        // Skip empty lines
        char* trimmed = trim(line);
        if(*trimmed == '\0') continue;

        // Parse: "number→lhs::=rhs | rhs2"
        // Format: LHS contains →, RHS contains ::= or ===>
        char* arrow = strstr(line, "→");
        if(!arrow) {
            // Try alternative format without arrow
            char* ruleSep = strstr(line, "::");
            if(!ruleSep) ruleSep = strstr(line, "===");
            if(!ruleSep) continue;
            arrow = ruleSep - 2;
        }

        // Extract LHS (left side of arrow/::=)
        char lhsStr[256];
        char* sep = strstr(trimmed, "::=");
        if(!sep) sep = strstr(trimmed, "===>");
        if(!sep) sep = strstr(trimmed, "→");
        if(!sep) continue;

        int lhsLen = sep - trimmed;
        strncpy(lhsStr, trimmed, lhsLen);
        lhsStr[lhsLen] = '\0';
        char* lhsName = trim(lhsStr);

        // Get nonterminal index for LHS
        NonTerminal lhsNT = lookupNonTerminal(lhsName);
        if(lhsNT == NT_UNKNOWN) continue;

        // Extract RHS (right side of ::= or ===>)
        char* rhs = strstr(trimmed, "::=");
        if(!rhs) rhs = strstr(trimmed, "===");
        if(!rhs) rhs = strchr(trimmed, '→') + 1;
        if(!rhs) continue;
        rhs = trim(rhs);

        // Parse alternatives (separated by |)
        char rhsCopy[500];
        strncpy(rhsCopy, rhs, sizeof(rhsCopy)-1);
        rhsCopy[sizeof(rhsCopy)-1] = '\0';

        char* alt = strtok(rhsCopy, "|");
        while(alt && G.ruleCount < MAX_RULES) {
            alt = trim(alt);
            if(*alt == '\0') {
                alt = strtok(NULL, "|");
                continue;
            }

            // Create production rule
            Production prod;
            prod.lhs = (int)lhsNT;
            prod.rhsCount = 0;

            // Parse symbols in RHS
            char altCopy[256];
            strncpy(altCopy, alt, sizeof(altCopy)-1);
            altCopy[sizeof(altCopy)-1] = '\0';

            char* sym = strtok(altCopy, " \t");
            while(sym && prod.rhsCount < MAX_SYMBOLS) {
                sym = trim(sym);
                if(*sym == '\0') {
                    sym = strtok(NULL, " \t");
                    continue;
                }

                // Check for epsilon
                if(strcmp(sym, "ε") == 0 || strcmp(sym, "eps") == 0 || strcmp(sym, "epsilon") == 0) {
                    prod.rhs[prod.rhsCount++] = EPSILON;
                } else {
                    // Try terminal first
                    TokenType t = lookupTerminal(sym);
                    if(t != TK_ERROR) {
                        prod.rhs[prod.rhsCount++] = (int)t;
                    } else {
                        // Try nonterminal
                        NonTerminal nt = lookupNonTerminal(sym);
                        if(nt != NT_UNKNOWN) {
                            // Encode nonterminal as negative value to distinguish from terminal
                            prod.rhs[prod.rhsCount++] = -(int)nt - 1;
                        }
                    }
                }
                sym = strtok(NULL, " \t");
            }

            G.rules[G.ruleCount++] = prod;
            alt = strtok(NULL, "|");
        }
    }

    fclose(f);
    printf("Grammar loaded: %d production rules\n", G.ruleCount);
    return G;
}

/* -------- Helper: Check if symbol is nonterminal (encoded as negative) -------- */
static int isNonTerminal(int symbol) {
    return symbol < -1;
}

static int decodeNonTerminal(int symbol) {
    return -(symbol + 1);
}

static int encodeNonTerminal(int nt) {
    return -(nt + 1);
}

/* -------- Helper: Check if a production can derive epsilon -------- */
static int canDeriveEpsilon(Production p, FirstAndFollow* F) {
    for (int i = 0; i < p.rhsCount; i++) {
        if (p.rhs[i] == EPSILON) {
            return 1;  // Explicit epsilon
        }
        if (!isNonTerminal(p.rhs[i])) {
            return 0;  // Terminal - cannot derive epsilon
        }
        int nt = decodeNonTerminal(p.rhs[i]);
        // Check if this nonterminal can derive epsilon (marked in FIRST[nt][-1])
        // We'll use index MAX_TERMINALS-1 to mark epsilon capability
        if (!F->first[nt][MAX_TERMINALS-1]) {
            return 0;
        }
    }
    return 1;  // All symbols can derive epsilon
}

/* -------- FIRST and FOLLOW Set Computation -------- */
FirstAndFollow ComputeFirstAndFollowSets(Grammar G) {
    FirstAndFollow F;
    memset(&F, 0, sizeof(F));

    // PHASE 1: Compute FIRST sets using fixed-point iteration
    printf("Computing FIRST sets...\n");

    // Initialize: FIRST(terminal) = {terminal}
    // We implicitly assume terminals are in FIRST

    int changed = 1;
    int iterations = 0;
    while (changed && iterations < 100) {
        changed = 0;
        iterations++;

        for (int rule = 0; rule < G.ruleCount; rule++) {
            Production p = G.rules[rule];
            int A = p.lhs;

            if (p.rhsCount == 0) {
                // Epsilon production
                if (!F.first[A][MAX_TERMINALS-1]) {
                    F.first[A][MAX_TERMINALS-1] = 1;
                    changed = 1;
                }
                continue;
            }

            // Check if RHS has explicit epsilon
            int hasEpsilon = (p.rhsCount == 1 && p.rhs[0] == EPSILON);
            if (hasEpsilon) {
                if (!F.first[A][MAX_TERMINALS-1]) {
                    F.first[A][MAX_TERMINALS-1] = 1;
                    changed = 1;
                }
                continue;
            }

            // Process: A → X₁ X₂ ... Xₙ
            int derivesEpsilon = 1;
            for (int i = 0; i < p.rhsCount; i++) {
                int sym = p.rhs[i];

                if (isNonTerminal(sym)) {
                    // Add FIRST(Xi) to FIRST(A)
                    int Xi = decodeNonTerminal(sym);
                    for (int t = 0; t < MAX_TERMINALS; t++) {
                        if (F.first[Xi][t] && !F.first[A][t]) {
                            F.first[A][t] = 1;
                            changed = 1;
                        }
                    }
                    // Check if Xi derives epsilon
                    if (!F.first[Xi][MAX_TERMINALS-1]) {
                        derivesEpsilon = 0;
                        break;
                    }
                } else {
                    // Terminal: add it to FIRST(A)
                    if (sym != EPSILON && !F.first[A][sym]) {
                        F.first[A][sym] = 1;
                        changed = 1;
                    }
                    derivesEpsilon = 0;
                    break;
                }
            }

            // If all symbols can derive epsilon, A derives epsilon
            if (derivesEpsilon && !F.first[A][MAX_TERMINALS-1]) {
                F.first[A][MAX_TERMINALS-1] = 1;
                changed = 1;
            }
        }
    }

    printf("FIRST sets computed in %d iterations\n", iterations);

    // PHASE 2: Compute FOLLOW sets using fixed-point iteration
    printf("Computing FOLLOW sets...\n");

    // Initialize: FOLLOW(start symbol) = {$}
    // Use MAX_TERMINALS-1 index for EOF marker as well (or last valid token)
    F.follow[NT_PROGRAM][0] = 1;  // Assume TK_END or similar is reasonable end marker

    changed = 1;
    iterations = 0;
    while (changed && iterations < 100) {
        changed = 0;
        iterations++;

        for (int rule = 0; rule < G.ruleCount; rule++) {
            Production p = G.rules[rule];
            int A = p.lhs;

            // For each symbol B in the production
            for (int i = 0; i < p.rhsCount; i++) {
                if (!isNonTerminal(p.rhs[i])) continue;  // Only for nonterminals

                int B = decodeNonTerminal(p.rhs[i]);
                int beta_idx = i + 1;  // Start of β (what follows B)

                // Add FIRST(β) - {ε} to FOLLOW(B)
                int beta_derives_epsilon = 1;
                for (int j = beta_idx; j < p.rhsCount; j++) {
                    int sym = p.rhs[j];

                    if (isNonTerminal(sym)) {
                        int C = decodeNonTerminal(sym);
                        for (int t = 0; t < MAX_TERMINALS-1; t++) {
                            if (F.first[C][t] && !F.follow[B][t]) {
                                F.follow[B][t] = 1;
                                changed = 1;
                            }
                        }
                        if (!F.first[C][MAX_TERMINALS-1]) {
                            beta_derives_epsilon = 0;
                            break;
                        }
                    } else {
                        // Terminal
                        if (sym != EPSILON && !F.follow[B][sym]) {
                            F.follow[B][sym] = 1;
                            changed = 1;
                        }
                        beta_derives_epsilon = 0;
                        break;
                    }
                }

                // If β derives epsilon, add FOLLOW(A) to FOLLOW(B)
                if (beta_derives_epsilon) {
                    for (int t = 0; t < MAX_TERMINALS; t++) {
                        if (F.follow[A][t] && !F.follow[B][t]) {
                            F.follow[B][t] = 1;
                            changed = 1;
                        }
                    }
                }
            }
        }
    }

    printf("FOLLOW sets computed in %d iterations\n", iterations);
    return F;
}

/* -------- Helper: Compute FIRST of a sequence of symbols -------- */
static void computeFirstOfSequence(int* symbols, int count, FirstAndFollow* F, int* firstSet) {
    memset(firstSet, 0, MAX_TERMINALS * sizeof(int));

    int derivesEpsilon = 1;
    for (int i = 0; i < count; i++) {
        int sym = symbols[i];

        if (isNonTerminal(sym)) {
            int nt = decodeNonTerminal(sym);
            for (int t = 0; t < MAX_TERMINALS-1; t++) {
                if (F->first[nt][t]) {
                    firstSet[t] = 1;
                }
            }
            if (!F->first[nt][MAX_TERMINALS-1]) {
                derivesEpsilon = 0;
                break;
            }
        } else {
            if (sym != EPSILON) {
                firstSet[sym] = 1;
            }
            derivesEpsilon = 0;
            break;
        }
    }

    if (derivesEpsilon) {
        firstSet[MAX_TERMINALS-1] = 1;
    }
}

/* -------- Parse Table Creation with Grammar -------- */
ParseTable createParseTable(Grammar G, FirstAndFollow F) {
    ParseTable T;
    ParseTable empty_table;
    memset(&empty_table, -1, sizeof(empty_table));
    T = empty_table;

    printf("Building predictive parsing table...\n");

    // For each production rule
    for (int rule = 0; rule < G.ruleCount; rule++) {
        Production p = G.rules[rule];
        int A = p.lhs;  // LHS nonterminal

        // Compute FIRST of RHS
        int firstSet[MAX_TERMINALS];
        computeFirstOfSequence(p.rhs, p.rhsCount, &F, firstSet);

        // For each terminal in FIRST(α), add production to M[A, a]
        for (int t = 0; t < MAX_TERMINALS-1; t++) {
            if (firstSet[t]) {
                if (T.table[A][t] != -1) {
                    printf("WARNING: Conflict at M[%d][%d]: rules %d and %d\n", A, t, T.table[A][t], rule);
                }
                T.table[A][t] = rule;
            }
        }

        // If ε in FIRST(α), for each terminal in FOLLOW(A), add production
        if (firstSet[MAX_TERMINALS-1]) {
            for (int t = 0; t < MAX_TERMINALS; t++) {
                if (F.follow[A][t]) {
                    if (T.table[A][t] != -1 && T.table[A][t] != rule) {
                        printf("WARNING: Conflict at M[%d][%d]: rules %d and %d\n", A, t, T.table[A][t], rule);
                    }
                    T.table[A][t] = rule;
                }
            }
        }
    }

    printf("Predictive parsing table created (%d rules)\n", G.ruleCount);
    return T;
}

/* ---------------- Parse Tree Utilities ---------------- */
ParseTreeNode* newParseTreeNode(Symbol symbol, ParseTreeNode* parent) {
    ParseTreeNode* node = (ParseTreeNode*)malloc(sizeof(ParseTreeNode));
    node->symbol = symbol;
    node->parent = parent;
    node->children = NULL;
    node->childCount = 0;
    memset(&node->tokenInfo, 0, sizeof(Token));
    return node;
}

/* -------- Stack-based Predictive Parser Implementation -------- */
ParseTree parseInputSourceCode(char *testcaseFile, Grammar G, ParseTable T) {
    ParseTree PT;
    PT.root = newParseTreeNode((Symbol){NONTERMINAL, NT_PROGRAM}, NULL);

    // Initialize lexer and get tokens
    State s = initializeState(testcaseFile);
    TokenList tokens = scan(&s);

    if (tokens.size == 0) {
        printf("Error: No tokens generated from input\n");
        return PT;
    }

    // Initialize parsing stack and current token
    int stack[512];
    int stackPtr = 0;
    int tokenIdx = 0;

    // Push EOF and start symbol
    stack[stackPtr++] = 0;  // EOF marker
    stack[stackPtr++] = encodeNonTerminal(NT_PROGRAM);

    // ParseTree node stack
    ParseTreeNode* nodeStack[512];
    int nodeStackPtr = 0;
    nodeStack[nodeStackPtr++] = PT.root;

    int syntaxErrorCount = 0;
    int maxErrors = 10;

    printf("DEBUG: Starting parse with %d tokens\n", tokens.size);
    if (tokens.size > 0) {
        printf("DEBUG: First token type: %d\n", tokens.buf[0].type);
    }

    // Main parsing loop
    while (stackPtr > 0) {
        int top = stack[--stackPtr];
        ParseTreeNode* topNode = nodeStack[--nodeStackPtr];

        // Get current token
        if (tokenIdx >= tokens.size) {
            break;
        }
        Token currentToken = tokens.buf[tokenIdx];

        if (top == 0) {
            // EOF on stack - parsing complete
            if (currentToken.type == TK_END || tokenIdx >= tokens.size) {
                break;
            } else {
                printf("Error: Expected EOF at line %u\n", currentToken.lineNo);
                syntaxErrorCount++;
                tokenIdx++;
                continue;
            }
        }

        if (isNonTerminal(top)) {
            // Nonterminal: lookup in parsing table
            int A = decodeNonTerminal(top);
            int ruleIdx = T.table[A][currentToken.type];

            printf("DEBUG: Expanding %s with token %d, rule: %d\n",
                   nonTerminalToString((NonTerminal)A), currentToken.type, ruleIdx);

            if (ruleIdx >= 0) {
                // Production found
                Production prod = G.rules[ruleIdx];
                printf("DEBUG:   RHS count: %d\n", prod.rhsCount);

                // Create child nodes for RHS and add to parent
                ParseTreeNode* childNodes[MAX_TERMINALS];
                int childCount = 0;

                for (int i = 0; i < prod.rhsCount; i++) {
                    if (prod.rhs[i] != EPSILON) {
                        // Create child node
                        Symbol childSymbol;
                        if (isNonTerminal(prod.rhs[i])) {
                            childSymbol.type = NONTERMINAL;
                            childSymbol.value = decodeNonTerminal(prod.rhs[i]);
                        } else {
                            childSymbol.type = TERMINAL;
                            childSymbol.value = prod.rhs[i];
                        }
                        ParseTreeNode* childNode = newParseTreeNode(childSymbol, topNode);
                        childNodes[childCount++] = childNode;
                    }
                }

                // Add child nodes to parent
                if (childCount > 0) {
                    topNode->children = (ParseTreeNode**)malloc(childCount * sizeof(ParseTreeNode*));
                    topNode->childCount = childCount;
                    for (int i = 0; i < childCount; i++) {
                        topNode->children[i] = childNodes[i];
                    }
                }

                // Push RHS onto stack (right to left) with corresponding child nodes
                for (int i = prod.rhsCount - 1, childIdx = childCount - 1; i >= 0; i--) {
                    if (prod.rhs[i] != EPSILON) {
                        stack[stackPtr] = prod.rhs[i];
                        nodeStack[nodeStackPtr] = childNodes[childIdx];
                        stackPtr++;
                        nodeStackPtr++;
                        childIdx--;
                    }
                }
            } else {
                // No production found - syntax error
                printf("Syntax error at line %u: unexpected token %d in %s\n",
                       currentToken.lineNo, currentToken.type, nonTerminalToString((NonTerminal)A));
                syntaxErrorCount++;

                // Panic mode error recovery: skip to next synchronization point
                if (syntaxErrorCount >= maxErrors) {
                    printf("Too many syntax errors - aborting\n");
                    break;
                }
                tokenIdx++;
            }
        } else {
            // Terminal: match with input
            if (top == (int)currentToken.type) {
                // Match successful
                topNode->symbol.type = TERMINAL;
                topNode->symbol.value = currentToken.type;
                topNode->tokenInfo = currentToken;
                tokenIdx++;
            } else {
                // Match failed - syntax error
                printf("Syntax error at line %u: expected token %d, got %d\n",
                       currentToken.lineNo, top, currentToken.type);
                syntaxErrorCount++;

                if (syntaxErrorCount >= maxErrors) {
                    printf("Too many syntax errors - aborting\n");
                    break;
                }

                // Try skipping token and continuing
                tokenIdx++;
            }
        }
    }

    // Report result
    if (syntaxErrorCount == 0) {
        printf("Input source code is syntactically correct...........\n");
    } else {
        printf("Input source code has %d syntax error(s)\n", syntaxErrorCount);
    }

    return PT;
}

/* ---------------- Parse Tree Printing ---------------- */
void printParseTree(ParseTree PT, char *outfile) {
    FILE* f = fopen(outfile, "w");
    if (!f) {
        printf("Error: Could not open output file %s\n", outfile);
        return;
    }

    // Recursive inorder traversal
    void inorder(ParseTreeNode* node) {
        if (!node) return;

        // Visit leftmost child first
        if (node->childCount > 0) {
            inorder(node->children[0]);
        }

        // Print current node
        fprintf(f, "%-10s %-5u %-15s %-10s %-15s %-5s %-20s\n",
            (node->symbol.type == TERMINAL) ? node->tokenInfo.lexeme : "----",
            node->tokenInfo.lineNo,
            (node->symbol.type == TERMINAL) ? "TERMINAL" : "NONTERMINAL",
            (node->symbol.type == TERMINAL &&
             (node->tokenInfo.type == TK_NUM || node->tokenInfo.type == TK_RNUM))
                ? node->tokenInfo.lexeme : "----",
            (node->parent) ? nonTerminalToString((NonTerminal)node->parent->symbol.value) : "ROOT",
            (node->symbol.type == TERMINAL) ? "yes" : "no",
            (node->symbol.type == NONTERMINAL) ? nonTerminalToString((NonTerminal)node->symbol.value) : "----"
        );

        // Visit remaining children (excluding leftmost already visited)
        for (int i = 1; i < node->childCount; i++) {
            inorder(node->children[i]);
        }
    }

    inorder(PT.root);
    fclose(f);
    printf("Parse tree printed to %s\n", outfile);
}
