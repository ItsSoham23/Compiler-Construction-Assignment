#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parserDef.h"
#include "lexerDef.h"
#include "lexer.h"
#include "parser.h"

/* ============================================================
 * Non-terminal name table
 * ============================================================ */
const char *NT_NAMES[] = {
    "program",
    "mainFunction",
    "otherFunctions",
    "function",
    "input_par",
    "output_par",
    "parameter_list",
    "dataType",
    "primitiveDatatype",
    "constructedDatatype",
    "remaining_list",
    "stmts",
    "typeDefinitions",
    "actualOrRedefined",
    "typeDefinition",
    "fieldDefinitions",
    "fieldDefinition",
    "fieldType",
    "moreFields",
    "declarations",
    "declaration",
    "global_or_not",
    "otherStmts",
    "stmt",
    "assignmentStmt",
    "singleOrRecId",
    "option_single_constructed",
    "oneExpansion",
    "moreExpansions",
    "funCallStmt",
    "outputParameters",
    "inputParameters",
    "iterativeStmt",
    "conditionalStmt",
    "elsePart",
    "ioStmt",
    "arithmeticExpression",
    "expPrime",
    "term",
    "termPrime",
    "factor",
    "highPrecedenceOperators",
    "lowPrecedenceOperators",
    "booleanExpression",
    "var",
    "logicalOp",
    "relationalOp",
    "returnStmt",
    "optionalReturn",
    "idList",
    "more_ids",
    "definetypestmt",
    "A"
};

const char *nonTerminalName(NonTerminal nt) {
    if (nt < 0 || nt >= NUM_NON_TERMINALS) return "UNKNOWN_NT";
    return NT_NAMES[nt];
}

/* ============================================================
 * Helpers to build GrammarSymbol
 * ============================================================ */
static GrammarSymbol T(TokenType t) {
    GrammarSymbol s;
    s.kind = SYM_TERMINAL;
    s.sym.terminal = t;
    return s;
}

static GrammarSymbol NT(NonTerminal nt) {
    GrammarSymbol s;
    s.kind = SYM_NON_TERMINAL;
    s.sym.nonTerminal = nt;
    return s;
}

static void addRule(Grammar *G, NonTerminal lhs, GrammarSymbol *rhs, int rhsLen) {
    int idx = G->numRules;
    Rule *r = &G->rules[idx];
    r->lhs    = lhs;
    r->rhsLen = rhsLen;
    for (int i = 0; i < rhsLen; i++) r->rhs[i] = rhs[i];
    G->numRules = idx + 1;
    /* keep both counters in sync for compatibility */
    G->ruleCount = G->numRules;
}

/* ============================================================
 * loadGrammar – hard-coded grammar matching grammar.txt
 * (We hard-code for correctness; grammar.txt is the human-readable ref)
 * ============================================================ */
Grammar loadGrammar(const char *grammarFile) {
    (void)grammarFile; /* not parsed; grammar is hard-coded */
    Grammar G;
    memset(&G, 0, sizeof(Grammar));

    FILE *f = fopen(grammarFile, "r");
    if (!f) {
        fprintf(stderr, "Error: Could not open grammar file %s\n", grammarFile);
        return G;
    }

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        char *lhs = strtok(line, ":=");
        if (!lhs) continue;
        lhs = trim(lhs);

        char *rhs = strtok(NULL, "\n");
        if (!rhs) continue;
        rhs = trim(rhs);

        NonTerminal lhsNT = lookupNonTerminal(lhs);

        // Split RHS by '|'
        char *alt = strtok(rhs, "|");
        while (alt) {
            GrammarSymbol rhsSymbols[MAX_RHS_LEN];
            int n = 0;

            alt = trim(alt);
            char *sym = strtok(alt, " ");
            while (sym) {
                TokenType t = lookupTerminal(sym);
                if (t != TK_ERROR) {
                    rhsSymbols[n++] = T(t);
                } else {
                    NonTerminal nt = lookupNonTerminal(sym);
                    rhsSymbols[n++] = NT(nt);
                }
                sym = strtok(NULL, " ");
            }

            // Use addRule() instead of manual assignment
            addRule(&G, lhsNT, rhsSymbols, n);

            alt = strtok(NULL, "|");
        }
    }


    /* B2a: <expPrime> ===> <lowPrecedenceOperators> <term> <expPrime> */
    static int isHardSynch(TokenType t) {
        /* reuse existing helper above */
        return (t == TK_SEM || t == TK_ENDRECORD || t == TK_ENDUNION ||
                t == TK_ENDIF || t == TK_ENDWHILE || t == TK_ELSE ||
                t == TK_THEN || t == TK_END || t == TK_CL || t == TK_SQR ||
                t == TK_EOF);
    }

    ParseTree *parseInputSourceCode(const char *testcaseFile, Grammar *G,
                                    ParseTable *T, FirstAndFollow *F) {
        /* Use existing lexer API */
        State s = initializeState(testcaseFile);
        TokenList tokens = scan(&s);
        int tokenIdx = 0;

        /* Build parse tree root */
        ParseTree *PT = (ParseTree *)calloc(1, sizeof(ParseTree));
        ParseTreeNode *root = newNonLeafNode(NT_PROGRAM);
        PT->root = root;

        /* Initialise stack */
        PStack S; S.top = -1;
        GrammarSymbol dollarSym; dollarSym.kind = SYM_TERMINAL; dollarSym.sym.terminal = TK_EOF;
        stackPush(&S, dollarSym, NULL);
        GrammarSymbol startSym; startSym.kind = SYM_NON_TERMINAL; startSym.sym.nonTerminal = NT_PROGRAM;
        stackPush(&S, startSym, root);

        int syntaxErrors = 0;

        /* Seed lookahead */
        Token lookahead;
        if (tokens.size > 0) lookahead = tokens.buf[0]; else { lookahead.type = TK_EOF; lookahead.lexeme[0]='\0'; lookahead.lineNo=0; }

        while (!stackEmpty(&S)) {
            StackEntry top = S.items[S.top];

            /* Skip lexer error tokens */
            if (lookahead.type == TK_ERROR) {
                tokenIdx++;
                if (tokenIdx < tokens.size) lookahead = tokens.buf[tokenIdx]; else { lookahead.type = TK_EOF; }
                continue;
            }

            if (top.sym.kind == SYM_TERMINAL) {
                TokenType expected = top.sym.sym.terminal;
                if (expected == lookahead.type) {
                    stackPop(&S);
                    if (top.treeNode) {
                        top.treeNode->token = lookahead;
                        top.treeNode->isLeaf = 1;
                    }
                    if (expected == TK_EOF) break;
                    tokenIdx++;
                    if (tokenIdx < tokens.size) lookahead = tokens.buf[tokenIdx]; else lookahead.type = TK_EOF;
                    continue;
                }

                if (expected == TK_EOF) {
                    int code = 9000 + (int)lookahead.type;
                    if (!alreadyReported(lookahead.lineNo, code)) {
                        fprintf(stderr, "Line %d\tError: The token %s for lexeme %s  does not match with the expected token %s\n",
                                lookahead.lineNo, tokenTypeName(lookahead.type), lookahead.lexeme, tokenTypeName(expected));
                        markReported(lookahead.lineNo, code);
                        syntaxErrors++;
                    }
                    break;
                }

                /* Recovery C */
                int missingTerminal = 0;
                for (int si = S.top - 1; si >= 0; si--) {
                    if (S.items[si].sym.kind == SYM_NON_TERMINAL) {
                        NonTerminal parentNT = S.items[si].sym.sym.nonTerminal;
                        if (F->followSet[parentNT][lookahead.type] || T->table[parentNT][lookahead.type] >= 0) missingTerminal = 1;
                        break;
                    }
                }
                if (isHardSynch(lookahead.type)) missingTerminal = 1;

                if (missingTerminal) {
                    int code = 1000 + (int)expected;
                    if (!alreadyReported(lookahead.lineNo, code)) {
                        fprintf(stderr, "Line %d\tError: The token %s for lexeme %s  does not match with the expected token %s\n",
                                lookahead.lineNo, tokenTypeName(lookahead.type), lookahead.lexeme, tokenTypeName(expected));
                        markReported(lookahead.lineNo, code);
                        syntaxErrors++;
                    }
                    stackPop(&S);
                } else {
                    int code = 2000 + (int)lookahead.type;
                    if (!alreadyReported(lookahead.lineNo, code)) {
                        fprintf(stderr, "Line %d\tError: The token %s for lexeme %s  does not match with the expected token %s\n",
                                lookahead.lineNo, tokenTypeName(lookahead.type), lookahead.lexeme, tokenTypeName(expected));
                        markReported(lookahead.lineNo, code);
                        syntaxErrors++;
                    }
                    tokenIdx++;
                    if (tokenIdx < tokens.size) lookahead = tokens.buf[tokenIdx]; else lookahead.type = TK_EOF;
                    if (lookahead.type == TK_EOF) stackPop(&S);
                }
                continue;
            }

            /* Nonterminal */
            NonTerminal nt = top.sym.sym.nonTerminal;
            TokenType la = lookahead.type;
            int ruleIdx = T->table[nt][la];

            if (ruleIdx >= 0) {
                stackPop(&S);
                Rule *r = &G->rules[ruleIdx];
                if (r->rhsLen == 0) {
                    /* epsilon node */
                    ParseTreeNode *epsNode = (ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
                    epsNode->isLeaf = 1;
                    epsNode->token.type = TK_EOF;
                    strncpy(epsNode->token.lexeme, "eps", MAX_LEXEME_LEN-1);
                    epsNode->token.lineNo = lookahead.lineNo;
                    if (top.treeNode) addChild(top.treeNode, epsNode);
                } else {
                    ParseTreeNode *childNodes[MAX_RHS_LEN];
                    for (int k = 0; k < r->rhsLen; k++) {
                        GrammarSymbol sym = r->rhs[k];
                        if (sym.kind == SYM_TERMINAL) {
                            Token placeholder = {0};
                            placeholder.type = sym.sym.terminal;
                            placeholder.lineNo = lookahead.lineNo;
                            childNodes[k] = newLeafNode(placeholder);
                        } else {
                            childNodes[k] = newNonLeafNode(sym.sym.nonTerminal);
                        }
                        if (top.treeNode) addChild(top.treeNode, childNodes[k]);
                    }
                    for (int k = r->rhsLen-1; k >= 0; k--) {
                        GrammarSymbol sym = r->rhs[k];
                        StackEntry e; e.sym = sym; e.treeNode = childNodes[k];
                        stackPush(&S, sym, childNodes[k]);
                    }
                }
                continue;
            }

            if (ruleIdx == PT_SYNCH) {
                int code = 3000 + (int)nt;
                if (!alreadyReported(lookahead.lineNo, code)) {
                    fprintf(stderr, "Line %d\tError: Invalid token %s encountered with value %s stack top %s\n",
                            lookahead.lineNo, tokenTypeName(la), lookahead.lexeme, nonTerminalName(nt));
                    markReported(lookahead.lineNo, code);
                    syntaxErrors++;
                }
                stackPop(&S);
                continue;
            }

            /* PT_ERROR panic */
            {
                int code = 4000 + (int)nt * 100 + (int)la % 100;
                if (!alreadyReported(lookahead.lineNo, code)) {
                    fprintf(stderr, "Line %d\tError: Invalid token %s encountered with value %s stack top %s\n",
                            lookahead.lineNo, tokenTypeName(la), lookahead.lexeme, nonTerminalName(nt));
                    markReported(lookahead.lineNo, code);
                    syntaxErrors++;
                }
                while (lookahead.type != TK_EOF) {
                    tokenIdx++;
                    if (tokenIdx < tokens.size) lookahead = tokens.buf[tokenIdx]; else { lookahead.type = TK_EOF; break; }
                    if (lookahead.type == TK_ERROR) continue;
                    la = lookahead.type;
                    if (isHardSynch(la)) break;
                    if (T->table[nt][la] != PT_ERROR) break;
                    int foundSynch = 0;
                    for (int si = S.top; si >= 0; si--) {
                        if (S.items[si].sym.kind == SYM_NON_TERMINAL) {
                            NonTerminal snt = S.items[si].sym.sym.nonTerminal;
                            if (F->followSet[snt][la] || T->table[snt][la] >= 0) { foundSynch = 1; break; }
                        }
                    }
                    if (foundSynch) break;
                }
                if (lookahead.type == TK_EOF) stackPop(&S);
            }
        }

        if (syntaxErrors == 0) printf("Input source code is syntactically correct...........\n");
        else { printf("\nTotal syntax errors found: %d\n", syntaxErrors); printf("NOTE: Parse tree may be incomplete due to errors.\n"); }

        return PT;
    }
        } else {
            /* Compute FIRST of this production's RHS */
            int rhsFirstSet[NUM_TERMINALS];
            memset(rhsFirstSet, 0, sizeof(rhsFirstSet));
            int allNullable = 1;
            for (int k = 0; k < r->rhsLen && allNullable; k++) {
                GrammarSymbol sym = r->rhs[k];
                if (sym.kind == SYM_TERMINAL) {
                    rhsFirstSet[sym.sym.terminal] = 1;
                    allNullable = 0;
                } else {
                    NonTerminal nt = sym.sym.nonTerminal;
                    for (int t = 0; t < NUM_TERMINALS; t++)
                        if (F->firstSet[nt][t]) rhsFirstSet[t] = 1;
                    if (!F->nullable[nt]) allNullable = 0;
                }
            }
            for (int t = 0; t < NUM_TERMINALS; t++) {
                if (rhsFirstSet[t]) {
                    if (PT.table[lhs][t] == PT_ERROR)
                        PT.table[lhs][t] = ri;
                    else
                        fprintf(stderr, "[PARSER-WARNING] Parse table conflict at [%s][%s]\n",
                                nonTerminalName(lhs), tokenTypeName((TokenType)t));
                }
            }
            if (allNullable) {
                for (int t = 0; t < NUM_TERMINALS; t++) {
                    if (F->followSet[lhs][t]) {
                        if (PT.table[lhs][t] == PT_ERROR)
                            PT.table[lhs][t] = ri;
                    }
                }
            }
        }
    }

    /* ----------------------------------------------------------------
     * Synch-set pass:
     * For every (A, a) still PT_ERROR where a in FOLLOW(A), mark
     * PT_SYNCH.  During parsing this means: "pop A and continue" —
     * the lookahead was expected after A so A is simply absent.
     * Also mark $ (TK_EOF) as PT_SYNCH for every non-terminal so
     * the parser always terminates gracefully on unexpected EOF.
     * ---------------------------------------------------------------- */
    for (int i = 0; i < NUM_NON_TERMINALS; i++) {
        for (int t = 0; t < NUM_TERMINALS; t++) {
            if (PT.table[i][t] == PT_ERROR && F->followSet[i][t]) {
                PT.table[i][t] = PT_SYNCH;
            }
        }
        /* Always synch on EOF */
        if (PT.table[i][TK_EOF] == PT_ERROR)
            PT.table[i][TK_EOF] = PT_SYNCH;
    }

    return PT;
}

/* ============================================================
 * Parse Tree helpers
 * ============================================================ */
static ParseTreeNode *newNonLeafNode(NonTerminal nt) {
    ParseTreeNode *node = (ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
    node->isLeaf = 0;
    node->nt     = nt;
    return node;
}

static ParseTreeNode *newLeafNode(tokenInfo tok) {
    ParseTreeNode *node = (ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
    node->isLeaf = 1;
    node->token  = tok;
    return node;
}

static void addChild(ParseTreeNode *parent, ParseTreeNode *child) {
    child->parent = parent;
    if (!parent->firstChild) {
        parent->firstChild = child;
        return;
    }
    /* Append to sibling list */
    ParseTreeNode *sib = parent->firstChild;
    while (sib->nextSibling) sib = sib->nextSibling;
    sib->nextSibling = child;
}

void freeParseTree(ParseTreeNode *node) {
    if (!node) return;
    freeParseTree(node->firstChild);
    freeParseTree(node->nextSibling);
    free(node);
}

/* ============================================================
 * Stack for LL(1) parsing
 * ============================================================ */
#define STACK_MAX 2048

typedef struct {
    GrammarSymbol  sym;
    ParseTreeNode *treeNode; /* corresponding tree node */
} StackEntry;

typedef struct {
    StackEntry items[STACK_MAX];
    int        top;
} PStack;

static void stackPush(PStack *S, GrammarSymbol sym, ParseTreeNode *node) {
    if (S->top >= STACK_MAX - 1) { fprintf(stderr, "[PARSER] Stack overflow\n"); return; }
    S->items[++(S->top)].sym      = sym;
    S->items[S->top].treeNode = node;
}

static StackEntry stackPop(PStack *S) {
    return S->items[(S->top)--];
}

static int stackEmpty(PStack *S) { return S->top < 0; }

/* ============================================================
 * Advance helper – get next non-comment, non-error token
 * Errors are printed by the lexer; we just skip them here
 * but still return them so the parser can account for them.
 * ============================================================ */
static tokenInfo nextNonComment(twinBuffer *B, Hashmap *kw, int *lineNo) {
    tokenInfo tok;
    do { tok = getNextToken(B, kw, lineNo); }
    while (tok.type == TK_COMMENT);
    return tok;
}

/* ============================================================
 * Hard synch tokens – the parser ALWAYS stops skipping when
 * it sees one of these, regardless of current NT on stack.
 * These are the structural "anchor" tokens of the language.
 * ============================================================ */
static int isHardSynch(TokenType t) {
    switch (t) {
        case TK_SEM:       /* ;          */
        case TK_ENDRECORD: /* endrecord  */
        case TK_ENDUNION:  /* endunion   */
        case TK_ENDIF:     /* endif      */
        case TK_ENDWHILE:  /* endwhile   */
        case TK_ELSE:      /* else       */
        case TK_THEN:      /* then       */
        case TK_END:       /* end        */
        case TK_CL:        /* )          */
        case TK_SQR:       /* ]          */
        case TK_EOF:
            return 1;
        default:
            return 0;
    }
}

/* ============================================================
 * Error deduplication helper
 * Tracks (lineNo, errorCode) pairs already reported.
 * Also tracks how many errors were reported per line to cap cascades.
 * ============================================================ */
#define MAX_ERR_DEDUP 512
#define MAX_ERRORS_PER_LINE 3

static int reportedLines[MAX_ERR_DEDUP];
static int reportedCodes[MAX_ERR_DEDUP];
static int numReported   = 0;
/* Per-line error count for cascade suppression */
static int errLineCount[8192];   /* indexed by line number */
static int lastPanicLine = -1;   /* line where last panic-mode recovery happened */

static int alreadyReported(int line, int code) {
    /* Suppress if too many errors already on this line */
    if (line < 8192 && errLineCount[line] >= MAX_ERRORS_PER_LINE) return 1;
    for (int i = 0; i < numReported; i++)
        if (reportedLines[i] == line && reportedCodes[i] == code) return 1;
    return 0;
}
static void markReported(int line, int code) {
    if (numReported < MAX_ERR_DEDUP) {
        reportedLines[numReported]   = line;
        reportedCodes[numReported++] = code;
    }
    if (line < 8192) errLineCount[line]++;
}

/* ============================================================
 * parseInputSourceCode  –  LL(1) with synch-set error recovery
 *
 * Error-message format matches listoferrors_t6.txt:
 *   "Line N\tError: The token X for lexeme Y does not match with the expected token Z"
 *   "Line N\tError: Invalid token X encountered with value Y stack top Z"
 *
 * Three recovery paths:
 *
 * (A) NT on top, T[NT][la] == PT_SYNCH  (la ∈ FOLLOW(NT))
 *     → Pop NT without consuming lookahead.  Report only if
 *       the NT is "significant" (not a nullable helper).
 *
 * (B) NT on top, T[NT][la] == PT_ERROR  (la ∉ FIRST(NT)∪FOLLOW(NT))
 *     → Panic mode: skip tokens until hard-synch token or until
 *       T[NT][la] is no longer PT_ERROR.  Report once per line.
 *
 * (C) Terminal t on top, t ≠ la
 *     C1: la ∈ FOLLOW of nearest NT ancestor  → missing terminal,
 *         pop t, do not consume lookahead.
 *     C2: spurious la  → skip la, keep t on stack.
 * ============================================================ */
ParseTree *parseInputSourceCode(const char *testcaseFile, Grammar *G,
                                ParseTable *T, FirstAndFollow *F) {
    FILE *fp = getStream(testcaseFile);
    if (!fp) return NULL;

    /* Reset deduplication state */
    numReported  = 0;
    lastPanicLine = -1;
    memset(errLineCount, 0, sizeof(errLineCount));

    twinBuffer B  = initTwinBuffer(fp);
    Hashmap    kw = initKeywordMap();
    int        lineNo       = 1;
    int        syntaxErrors = 0;

    /* Build parse tree root */
    ParseTree     *PT   = (ParseTree *)calloc(1, sizeof(ParseTree));
    ParseTreeNode *root = newNonLeafNode(NT_PROGRAM);
    PT->root = root;

    /* ------ Initialise stack -------------------------------- */
    PStack S;
    S.top = -1;

    GrammarSymbol dollarSym;
    dollarSym.kind         = SYM_TERMINAL;
    dollarSym.sym.terminal = TK_EOF;
    stackPush(&S, dollarSym, NULL);

    GrammarSymbol startSym;
    startSym.kind               = SYM_NON_TERMINAL;
    startSym.sym.nonTerminal    = NT_PROGRAM;
    stackPush(&S, startSym, root);

    /* ------ Seed lookahead ---------------------------------- */
    tokenInfo lookahead = nextNonComment(&B, &kw, &lineNo);
    /* Skip initial lexer errors */
    while (lookahead.type == TK_ERROR)
        lookahead = nextNonComment(&B, &kw, &lineNo);

    /* ======================================================== */
    while (!stackEmpty(&S)) {

        StackEntry top = S.items[S.top];

        /* Skip TK_ERROR tokens from lexer – already reported by lexer */
        if (lookahead.type == TK_ERROR) {
            lookahead = nextNonComment(&B, &kw, &lineNo);
            continue;
        }

        /* -------------------------------------------------- */
        /* CASE 1: Terminal on top of stack                   */
        /* -------------------------------------------------- */
        if (top.sym.kind == SYM_TERMINAL) {
            TokenType expected = top.sym.sym.terminal;

            /* Normal match */
            if (expected == lookahead.type) {
                stackPop(&S);
                if (top.treeNode) {
                    top.treeNode->token  = lookahead;
                    top.treeNode->isLeaf = 1;
                }
                if (expected == TK_EOF) break;
                lookahead = nextNonComment(&B, &kw, &lineNo);
                continue;
            }

            /* $ mismatch – terminate */
            if (expected == TK_EOF) {
                int code = 9000 + (int)lookahead.type;
                if (!alreadyReported(lookahead.lineNo, code)) {
                    fprintf(stderr,
                        "Line %d\tError: The token %s for lexeme %s"
                        "  does not match with the expected token %s\n",
                        lookahead.lineNo,
                        tokenTypeName(lookahead.type), lookahead.lexeme,
                        tokenTypeName(expected));
                    markReported(lookahead.lineNo, code);
                    syntaxErrors++;
                }
                break;
            }

            /* ---- Recovery C: terminal mismatch ---- */
            /* Check if la appears in FOLLOW of the nearest NT on stack
               → expected terminal is simply absent (missing token case) */
            int missingTerminal = 0;
            for (int si = S.top - 1; si >= 0; si--) {
                if (S.items[si].sym.kind == SYM_NON_TERMINAL) {
                    NonTerminal parentNT = S.items[si].sym.sym.nonTerminal;
                    if (F->followSet[parentNT][lookahead.type] ||
                        T->table[parentNT][lookahead.type] >= 0) {
                        missingTerminal = 1;
                    }
                    break;
                }
            }
            /* Also treat as missing if la is a hard-synch anchor */
            if (isHardSynch(lookahead.type)) missingTerminal = 1;

            if (missingTerminal) {
                /* C1: missing terminal */
                int code = 1000 + (int)expected;
                if (!alreadyReported(lookahead.lineNo, code)) {
                    fprintf(stderr,
                        "Line %d\tError: The token %s for lexeme %s"
                        "  does not match with the expected token %s\n",
                        lookahead.lineNo,
                        tokenTypeName(lookahead.type), lookahead.lexeme,
                        tokenTypeName(expected));
                    markReported(lookahead.lineNo, code);
                    syntaxErrors++;
                }
                stackPop(&S);
                /* Do NOT consume lookahead */
            } else {
                /* C2: spurious lookahead – skip it */
                int code = 2000 + (int)lookahead.type;
                if (!alreadyReported(lookahead.lineNo, code)) {
                    fprintf(stderr,
                        "Line %d\tError: The token %s for lexeme %s"
                        "  does not match with the expected token %s\n",
                        lookahead.lineNo,
                        tokenTypeName(lookahead.type), lookahead.lexeme,
                        tokenTypeName(expected));
                    markReported(lookahead.lineNo, code);
                    syntaxErrors++;
                }
                lookahead = nextNonComment(&B, &kw, &lineNo);
                if (lookahead.type == TK_EOF) stackPop(&S);
            }
            continue;
        }

        /* -------------------------------------------------- */
        /* CASE 2: Non-terminal on top of stack               */
        /* -------------------------------------------------- */
        NonTerminal nt      = top.sym.sym.nonTerminal;
        TokenType   la      = lookahead.type;
        int         ruleIdx = T->table[nt][la];

        /* ---- Normal: valid rule ---- */
        if (ruleIdx >= 0) {
            stackPop(&S);
            Rule *r = &G->rules[ruleIdx];

            if (r->rhsLen == 0) {
                /* Epsilon */
                ParseTreeNode *epsNode = (ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
                epsNode->isLeaf        = 0;
                epsNode->token.type    = TK_EOF;
                strncpy(epsNode->token.lexeme, "eps", MAX_LEXEME_LEN - 1);
                epsNode->token.lineNo  = lookahead.lineNo;
                if (top.treeNode) addChild(top.treeNode, epsNode);
            } else {
                ParseTreeNode *childNodes[MAX_RHS_LEN];
                for (int k = 0; k < r->rhsLen; k++) {
                    GrammarSymbol sym = r->rhs[k];
                    if (sym.kind == SYM_TERMINAL) {
                        tokenInfo placeholder;
                        memset(&placeholder, 0, sizeof(tokenInfo));
                        placeholder.type   = sym.sym.terminal;
                        placeholder.lineNo = lookahead.lineNo;
                        childNodes[k] = newLeafNode(placeholder);
                    } else {
                        childNodes[k] = newNonLeafNode(sym.sym.nonTerminal);
                    }
                    if (top.treeNode) addChild(top.treeNode, childNodes[k]);
                }
                for (int k = r->rhsLen - 1; k >= 0; k--)
                    stackPush(&S, r->rhs[k], childNodes[k]);
            }
            continue;
        }

        /* ---- Recovery A: PT_SYNCH  (la ∈ FOLLOW(NT)) ---- */
        if (ruleIdx == PT_SYNCH) {
            /* Only report for meaningful non-terminals, not internal helpers */
            int code = 3000 + (int)nt;
            if (!alreadyReported(lookahead.lineNo, code)) {
                fprintf(stderr,
                    "Line %d\tError: Invalid token %s encountered with value %s"
                    " stack top %s\n",
                    lookahead.lineNo,
                    tokenTypeName(la), lookahead.lexeme,
                    nonTerminalName(nt));
                markReported(lookahead.lineNo, code);
                syntaxErrors++;
            }
            stackPop(&S);
            /* Do NOT consume lookahead */
            continue;
        }

        /* ---- Recovery B: PT_ERROR  (la ∉ FIRST(NT) ∪ FOLLOW(NT)) ---- */
        {
            int code = 4000 + (int)nt * 100 + (int)la % 100;
            if (!alreadyReported(lookahead.lineNo, code)) {
                fprintf(stderr,
                    "Line %d\tError: Invalid token %s encountered with value %s"
                    " stack top %s\n",
                    lookahead.lineNo,
                    tokenTypeName(la), lookahead.lexeme,
                    nonTerminalName(nt));
                markReported(lookahead.lineNo, code);
                syntaxErrors++;
            }

            /* Panic mode: skip until hard-synch or until T[NT][la] != PT_ERROR */
            while (lookahead.type != TK_EOF) {
                lookahead = nextNonComment(&B, &kw, &lineNo);
                /* Skip over lexer errors silently */
                if (lookahead.type == TK_ERROR) continue;
                la = lookahead.type;
                /* Always stop at hard-synch anchors */
                if (isHardSynch(la)) break;
                /* Stop if we can resume parsing this NT */
                if (T->table[nt][la] != PT_ERROR) break;
                /* Stop if lookahead is in FOLLOW of any NT on stack */
                int foundSynch = 0;
                for (int si = S.top; si >= 0; si--) {
                    if (S.items[si].sym.kind == SYM_NON_TERMINAL) {
                        NonTerminal snt = S.items[si].sym.sym.nonTerminal;
                        if (F->followSet[snt][la] ||
                            T->table[snt][la] >= 0) {
                            foundSynch = 1; break;
                        }
                    }
                }
                if (foundSynch) break;
            }

            if (lookahead.type == TK_EOF) stackPop(&S);
            /* Re-enter loop with new lookahead */
        }
    }
    /* ======================================================== */

    fclose(fp);

    if (syntaxErrors == 0) {
        printf("Input source code is syntactically correct...........\n");
    } else {
        printf("\nTotal syntax errors found: %d\n", syntaxErrors);
        printf("NOTE: Parse tree may be incomplete due to errors.\n");
    }

    return PT;
}


/* ============================================================
 * printParseTree – inorder traversal for n-ary tree
 * Inorder for n-ary: leftmost child -> parent -> remaining siblings
 * Output format per node:
 * lexeme  CurrentNode  lineno  tokenName  valueIfNumber  parentNodeSymbol  isLeafNode  NodeSymbol
 * ============================================================ */
static void inorderPrint(ParseTreeNode *node, FILE *out) {
    if (!node) return;

    /* Case: leaf node (terminal) – also includes epsilon markers */
    if (node->isLeaf || (node->firstChild == NULL && node->isLeaf == 0)) {
        /* Determine lexeme */
        char lexemeStr[MAX_LEXEME_LEN + 4];
        if (node->isLeaf) {
            strncpy(lexemeStr, node->token.lexeme, MAX_LEXEME_LEN);
        } else {
            strncpy(lexemeStr, "eps", MAX_LEXEME_LEN);
        }

        /* Parent symbol */
        char parentSym[64];
        if (!node->parent) {
            strncpy(parentSym, "ROOT", 64);
        } else if (node->parent->isLeaf == 0 && node->parent->firstChild != NULL) {
            strncpy(parentSym, nonTerminalName(node->parent->nt), 64);
        } else {
            strncpy(parentSym, tokenTypeName(node->parent->token.type), 64);
        }

        /* Token name */
        const char *tokName = tokenTypeName(node->token.type);
        /* Value if number: print lexeme for numeric tokens */
        const char *valStr = "---";
        char numBuf[64];
        if (node->token.type == TK_NUM || node->token.type == TK_RNUM) {
            snprintf(numBuf, sizeof(numBuf), "%s", node->token.lexeme);
            valStr = numBuf;
        }

        fprintf(out, "%-20s  %-30s  %-6d  %-22s  %-12s  %-30s  %-6s  %s\n",
                lexemeStr,
                "----",
                node->token.lineNo,
                tokName,
                valStr,
                parentSym,
                "yes",
                "----");
        return;
    }

    /* Non-leaf: inorder traversal */
    ParseTreeNode *child = node->firstChild;

    /* Print first child subtree */
    if (child) inorderPrint(child, out);

    /* Print current (parent) node */
    char parentSym[64];
    if (!node->parent) {
        strncpy(parentSym, "ROOT", 64);
    } else {
        strncpy(parentSym, nonTerminalName(node->parent->nt), 64);
    }

    fprintf(out, "%-20s  %-30s  %-6d  %-22s  %-12s  %-30s  %-6s  %s\n",
            "----",
            nonTerminalName(node->nt),
            (child ? child->token.lineNo : 0),
            "----",
            "---",
            parentSym,
            "no",
            nonTerminalName(node->nt));

    /* Print remaining siblings of first child */
    if (child) {
        ParseTreeNode *sib = child->nextSibling;
        while (sib) {
            inorderPrint(sib, out);
            sib = sib->nextSibling;
        }
    }
}

void printParseTree(ParseTree *PT, const char *outfile) {
    if (!PT || !PT->root) {
        fprintf(stderr, "[ERROR] Parse tree is NULL\n");
        return;
    }
    FILE *out = fopen(outfile, "w");
    if (!out) {
        fprintf(stderr, "[ERROR] Cannot open output file: %s\n", outfile);
        return;
    }

    /* Print header */
    fprintf(out, "%-20s  %-30s  %-6s  %-22s  %-12s  %-30s  %-6s  %s\n",
            "Lexeme", "CurrentNode", "LineNo", "TokenName",
            "ValueIfNum", "ParentNodeSymbol", "IsLeaf", "NodeSymbol");
    fprintf(out, "%s\n", "--------------------------------------------------------------"
                         "--------------------------------------------------------------");

    inorderPrint(PT->root, out);
    fclose(out);
    printf("Parse tree written to: %s\n", outfile);
}
