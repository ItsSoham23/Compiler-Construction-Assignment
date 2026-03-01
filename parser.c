#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>  /* for strcasecmp */
#include <ctype.h>

#include "parser.h"
#include "lexer.h"

/* ------------------------------------------------------------------
 * Nonterminal name table (must match enum order in parserDef.h)
 * Used for both printing and lookup during grammar loading.
 * ------------------------------------------------------------------ */
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

/* ------------------------------------------------------------------
 * Utility functions for symbol management
 * ------------------------------------------------------------------ */

/* convert nonterminal string (as found in grammar.txt) to enum value */
static int lookupNonTerminal(const char *s) {
    if (!s) return -1;
    for (int i = 0; i < NUM_NON_TERMINALS; ++i) {
        if (strcmp(NT_NAMES[i], s) == 0) {
            return i;
        }
    }
    return -1;
}

const char *nonTerminalName(NonTerminal nt) {
    if (nt < 0 || nt >= NUM_NON_TERMINALS) return "<unknown>";
    return NT_NAMES[nt];
}

/* convert token string (TK_...) to TokenType enum */
static TokenType tokenTypeFromString(const char *s) {
    if (!s) return TK_ERROR;
    /* use lexer helper for name list */
    for (int t = 0; t <= TK_EOF; ++t) {
        if (strcmp(tokenTypeName((TokenType)t), s) == 0) {
            return (TokenType)t;
        }
    }
    return TK_ERROR;
}

/* trim leading and trailing whitespace in-place */
static void trim(char *s) {
    char *start = s;
    /* skip leading whitespace */
    while (isspace((unsigned char)*start)) start++;
    if (start != s) {
        /* slide the trimmed string back to the original buffer */
        memmove(s, start, strlen(start) + 1);
    }
    /* now remove trailing whitespace */
    char *end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
}

/* check if alternative stands for epsilon
 * The original grammar used the Greek letter epsilon (ε, U+03B5)
 * which is two bytes in UTF-8.  Instead of forcing the grammar
 * file to use the ASCII string "eps" we recognise the unicode
 * character here.  We also accept the common spellings for
 * backwards compatibility.
 */
static int isEpsilon(const char *s) {
    if (!s) return 0;
    if (s[0] == '\0') return 1;          /* empty right–hand side */
    if (strcasecmp(s, "eps") == 0 ||
        strcasecmp(s, "epsilon") == 0) {
        return 1;
    }
    /* compare directly against the utf-8 epsilon literal; the source
       file is saved as UTF‑8 so this comparison works. */
    if (strcmp(s, "ε") == 0 || strcmp(s, "Ε") == 0) {
        return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------
 * Grammar loading
 * ------------------------------------------------------------------ */

Grammar loadGrammar(const char *grammarFile) {
    Grammar G;
    memset(&G, 0, sizeof(G));

    FILE *fp = fopen(grammarFile, "r");
    if (!fp) {
        fprintf(stderr, "Unable to open grammar file '%s'\n", grammarFile);
        return G;
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        char *p = line;
        /* strip newline */
        char *nl = strchr(p, '\n');
        if (nl) *nl = '\0';
        trim(p);
        if (p[0] == '\0' || p[0] == '#') continue; /* skip blank/comment */

        /* find production arrow (::=, ->, ==> etc) */
        char *sep = strstr(p, "::=");
        if (!sep) sep = strstr(p, "->");
        if (!sep) sep = strstr(p, "==>");
        if (!sep) {
            /* malformed line, skip */
            continue;
        }
        *sep = '\0';
        char *lhs = p;
        trim(lhs);
        char *rhs = sep + 3;
        if (strncmp(sep + 1, "=", 1) == 0) rhs = sep + 3; /* for ::= */
        trim(rhs);

        int nt = lookupNonTerminal(lhs);
        if (nt < 0) {
            fprintf(stderr, "Warning: unknown nonterminal '%s' in grammar\n", lhs);
            continue;
        }

        /* split alternatives on '|' */
        char *alt = rhs;
        while (alt) {
            char *nextAlt = strchr(alt, '|');
            if (nextAlt) {
                *nextAlt = '\0';
            }
            char temp[512];
            strncpy(temp, alt, sizeof(temp) - 1);
            temp[sizeof(temp) - 1] = '\0';
            trim(temp);

            Rule rule;
            rule.lhs = (NonTerminal)nt;
            rule.rhsLen = 0;

            if (isEpsilon(temp)) {
                /* empty production */
                rule.rhsLen = 0;
            } else {
                /* split tokens by whitespace */
                char *tok = strtok(temp, " \t");
                while (tok) {
                    GrammarSymbol gs;
                    if (strncmp(tok, "TK_", 3) == 0) {
                        TokenType tt = tokenTypeFromString(tok);
                        gs.kind = SYM_TERMINAL;
                        gs.sym.terminal = tt;
                    } else {
                        int tnt = lookupNonTerminal(tok);
                        if (tnt < 0) {
                            fprintf(stderr, "Warning: unknown symbol '%s'\n", tok);
                            /* treat as terminal error to avoid crash */
                            gs.kind = SYM_TERMINAL;
                            gs.sym.terminal = TK_ERROR;
                        } else {
                            gs.kind = SYM_NON_TERMINAL;
                            gs.sym.nonTerminal = (NonTerminal)tnt;
                        }
                    }
                    if (rule.rhsLen < MAX_RHS_LEN) {
                        rule.rhs[rule.rhsLen++] = gs;
                    }
                    tok = strtok(NULL, " \t");
                }
            }

            if (G.numRules < MAX_RULES) {
                G.rules[G.numRules++] = rule;
            }

            if (nextAlt) {
                alt = nextAlt + 1;
            } else {
                alt = NULL;
            }
        }
    }

    fclose(fp);
    G.ruleCount = G.numRules;
    return G;
}

/* ------------------------------------------------------------------
 * FIRST / FOLLOW computation helpers
 * ------------------------------------------------------------------ */

static int isNullable(FirstAndFollow *F, NonTerminal nt) {
    return F->nullable[nt];
}

static int addFirst(FirstAndFollow *F, NonTerminal nt, int t) {
    if (t < 0 || t >= NUM_TERMINALS) return 0;
    if (!F->firstSet[nt][t]) {
        F->firstSet[nt][t] = 1;
        return 1;
    }
    return 0;
}

static int unionFirstRow(int dst[NUM_TERMINALS], int src[NUM_TERMINALS]) {
    int changed = 0;
    for (int i = 0; i < NUM_TERMINALS; ++i) {
        if (src[i] && !dst[i]) {
            dst[i] = 1;
            changed = 1;
        }
    }
    return changed;
}

static void computeFirstOfSequence(Grammar *G, FirstAndFollow *F,
                                   GrammarSymbol rhs[], int len,
                                   int out[NUM_TERMINALS], int *nullable) {
    (void)G; /* parameter unused in current implementation */
    memset(out, 0, sizeof(int) * NUM_TERMINALS);
    *nullable = 1;
    for (int i = 0; i < len; ++i) {
        GrammarSymbol gs = rhs[i];
        if (gs.kind == SYM_TERMINAL) {
            out[gs.sym.terminal] = 1;
            *nullable = 0;
            return;
        } else {
            int nt = gs.sym.nonTerminal;
            unionFirstRow(out, F->firstSet[nt]);
            if (!isNullable(F, nt)) {
                *nullable = 0;
                return;
            }
        }
    }
    *nullable = 1;
}

static int addFollow(FirstAndFollow *F, NonTerminal nt, int t) {
    if (t < 0 || t >= NUM_TERMINALS) return 0;
    if (!F->followSet[nt][t]) {
        F->followSet[nt][t] = 1;
        return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------
 * FIRST and FOLLOW set computation
 * ------------------------------------------------------------------ */

FirstAndFollow computeFirstAndFollowSets(Grammar *G) {
    FirstAndFollow F;
    memset(&F, 0, sizeof(F));

    /* Phase 1: FIRST sets and nullability */
    int changed = 1, iter = 0;
    while (changed && iter < 100) {
        changed = 0; ++iter;
        for (int ri = 0; ri < G->numRules; ++ri) {
            Rule *r = &G->rules[ri];
            NonTerminal A = r->lhs;
            if (r->rhsLen == 0) {
                if (!F.nullable[A]) {
                    F.nullable[A] = 1;
                    changed = 1;
                }
                continue;
            }
            int rhsNullable = 1;
            for (int k = 0; k < r->rhsLen; ++k) {
                GrammarSymbol gs = r->rhs[k];
                if (gs.kind == SYM_TERMINAL) {
                    if (addFirst(&F, A, gs.sym.terminal)) changed = 1;
                    rhsNullable = 0;
                    break;
                } else {
                    int B = gs.sym.nonTerminal;
                    if (unionFirstRow(F.firstSet[A], F.firstSet[B])) changed = 1;
                    if (!F.nullable[B]) {
                        rhsNullable = 0;
                        break;
                    }
                }
            }
            if (rhsNullable) {
                if (!F.nullable[A]) {
                    F.nullable[A] = 1;
                    changed = 1;
                }
            }
        }
    }

    /* Phase 2: FOLLOW sets */
    addFollow(&F, NT_PROGRAM, TK_EOF);
    changed = 1; iter = 0;
    while (changed && iter < 100) {
        changed = 0; ++iter;
        for (int ri = 0; ri < G->numRules; ++ri) {
            Rule *r = &G->rules[ri];
            NonTerminal A = r->lhs;
            for (int i = 0; i < r->rhsLen; ++i) {
                GrammarSymbol gs = r->rhs[i];
                if (gs.kind == SYM_NON_TERMINAL) {
                    NonTerminal B = gs.sym.nonTerminal;
                    int firstBeta[NUM_TERMINALS];
                    int betaNullable;
                    computeFirstOfSequence(G, &F, &r->rhs[i+1], r->rhsLen - (i+1),
                                           firstBeta, &betaNullable);
                    for (int t = 0; t < NUM_TERMINALS; ++t) {
                        if (firstBeta[t]) {
                            if (addFollow(&F, B, t)) changed = 1;
                        }
                    }
                    if (betaNullable) {
                        if (unionFirstRow(F.followSet[B], F.followSet[A])) changed = 1;
                    }
                }
            }
        }
    }
    return F;
}

/* ------------------------------------------------------------------
 * Parse table construction
 * ------------------------------------------------------------------ */

ParseTable createParseTable(Grammar *G, FirstAndFollow *F) {
    ParseTable T;
    for (int i = 0; i < NUM_NON_TERMINALS; ++i)
        for (int j = 0; j < NUM_TERMINALS; ++j)
            T.table[i][j] = PT_ERROR;

    for (int ri = 0; ri < G->numRules; ++ri) {
        Rule *r = &G->rules[ri];
        NonTerminal A = r->lhs;
        int firstAlpha[NUM_TERMINALS];
        int alphaNullable;
        computeFirstOfSequence(G, F, r->rhs, r->rhsLen, firstAlpha, &alphaNullable);
        for (int t = 0; t < NUM_TERMINALS; ++t) {
            if (firstAlpha[t]) {
                if (T.table[A][t] != PT_ERROR) {
                    fprintf(stderr, "Parse table conflict for [%s,%s]\n", 
                            nonTerminalName(A), tokenTypeName((TokenType)t));
                }
                T.table[A][t] = ri;
            }
        }
        if (alphaNullable) {
            for (int t = 0; t < NUM_TERMINALS; ++t) {
                if (F->followSet[A][t]) {
                    if (T.table[A][t] != PT_ERROR) {
                        fprintf(stderr, "Parse table conflict for [%s,%s] (follow)\n", 
                                nonTerminalName(A), tokenTypeName((TokenType)t));
                    }
                    T.table[A][t] = ri;
                }
            }
        }
    }
    for (int A = 0; A < NUM_NON_TERMINALS; ++A) {
        for (int t = 0; t < NUM_TERMINALS; ++t) {
            if (T.table[A][t] == PT_ERROR && F->followSet[A][t]) {
                T.table[A][t] = PT_SYNCH;
            }
        }
    }
    return T;
}

/* ------------------------------------------------------------------
 * Parse tree support helpers
 * ------------------------------------------------------------------ */

static ParseTreeNode *newNode(int sym) {
    ParseTreeNode *node = (ParseTreeNode *)calloc(1, sizeof(ParseTreeNode));
    node->parent = NULL;
    node->firstChild = NULL;
    node->nextSibling = NULL;

    if (sym >= 0) {
        node->isLeaf = 1;
        node->token.type = (TokenType)sym;
    } else {
        node->isLeaf = 0;
        node->nt = (NonTerminal)(-sym - 1);
    }
    return node;
}

void freeParseTree(ParseTreeNode *node) {
    if (!node) return;
    freeParseTree(node->firstChild);
    freeParseTree(node->nextSibling);
    free(node);
}

/* ------------------------------------------------------------------
 * Parsing algorithm (LL1 predictive parser)
 * ------------------------------------------------------------------ */

ParseTree *parseInputSourceCode(const char *testcaseFile, Grammar *G,
                                ParseTable *T, FirstAndFollow *F) {
    State s = initializeState(testcaseFile);
    TokenList tokens = scan(&s);

    /* Drop comment tokens; keep errors and valid tokens in stream order.
       Lexical errors will be printed inline during the parse loop below. */
    int write_pos = 0;
    for (int i = 0; i < tokens.size; ++i) {
        if (tokens.buf[i].type != TK_COMMENT)
            tokens.buf[write_pos++] = tokens.buf[i];
    }
    tokens.size = write_pos;

    /* Append EOF sentinel */
    Token eofTk;
    memset(&eofTk, 0, sizeof(eofTk));
    eofTk.type = TK_EOF;
    tokens.buf = (Token *)realloc(tokens.buf, sizeof(Token) * (tokens.size + 1));
    tokens.buf[tokens.size++] = eofTk;

    ParseTree *PT = (ParseTree *)malloc(sizeof(ParseTree));
    PT->root = newNode(-(NT_PROGRAM + 1));

    typedef struct { int sym; ParseTreeNode *node; } StackEntry;
    StackEntry stack[10000];
    int sp = 0;
    stack[sp].sym  = -(NT_PROGRAM + 1);
    stack[sp].node = PT->root;
    sp++;

    int idx          = 0;
    int syntaxErrors = 0;
    const int MAX_ERRORS = 1000;
    unsigned int prevLine = 1; /* line of the last successfully consumed terminal */

    while (sp > 0 && idx < tokens.size && syntaxErrors < MAX_ERRORS) {

        /* --- Print and skip all pending lexical errors inline --- */
        while (idx < tokens.size && tokens.buf[idx].type == TK_ERROR) {
            Token *eTk = &tokens.buf[idx];
            if (eTk->errMsg && strcmp(eTk->errMsg, "EOF") != 0)
                fprintf(stderr, "Line %u\tError: %s\n", eTk->lineNo,
                        eTk->errMsg ? eTk->errMsg : eTk->lexeme);
            idx++;
        }
        /* Recovery after lexical error: if the expected terminal is
         * TK_ASSIGNOP and it was garbled by the lexer, collapse the whole
         * broken assignment RHS silently: pop TK_ASSIGNOP (and the
         * arithmeticExpression below it) from the stack and skip tokens
         * up to (but not including) TK_SEM so the statement ends cleanly
         * with only the one lexical error. */
        if (idx < tokens.size && sp >= 2) {
            int stackTop = stack[sp - 1].sym;
            if (stackTop == (int)TK_ASSIGNOP &&
                tokens.buf[idx].type != TK_ASSIGNOP) {
                /* skip token garbage before the closing ; */
                while (idx < tokens.size &&
                       tokens.buf[idx].type != TK_SEM &&
                       tokens.buf[idx].type != TK_EOF)
                    idx++;
                /* pop TK_ASSIGNOP off the stack (no error reported) */
                sp--;
                /* if arithmeticExpression is now on top, pop it too */
                if (sp > 0 && stack[sp-1].sym == -(NT_ARITHMETIC_EXPRESSION + 1))
                    sp--;
                /* TK_SEM now matches the TK_SEM waiting on the stack */
            }
        }
        if (idx >= tokens.size) break;

        Token cur = tokens.buf[idx];
        StackEntry top = stack[--sp];
        int sym        = top.sym;
        ParseTreeNode *node = top.node;

        /* ---- Terminal on stack ---- */
        if (sym >= 0) {
            if (sym == (int)cur.type) {
                /* Normal match */
                node->isLeaf   = 1;
                node->token    = cur;
                prevLine       = cur.lineNo;
                idx++;
            } else {
                /* Terminal mismatch.
                 * C1 (missing terminal): lookahead belongs to the outer context
                 *   → insert synthetic terminal, do NOT consume lookahead.
                 * C2 (spurious lookahead): lookahead is garbage
                 *   → consume lookahead, synthetic terminal stays.
                 *
                 * Use prevLine as the error line when the mismatched token is
                 * on a later line (catches missing `;` at end-of-line). */
                unsigned int errLine = (cur.lineNo > prevLine) ? prevLine : cur.lineNo;

                /* Synthetic terminal in parse tree */
                node->isLeaf         = 1;
                node->token.type     = (TokenType)sym;
                node->token.lineNo   = errLine;
                node->token.lexeme[0]= '\0';
                node->token.errMsg   = NULL;

                /* Decide C1 vs C2: check if lookahead is in FOLLOW of any ancestor NT,
                 * OR if lookahead directly matches a terminal already waiting in the
                 * top few stack slots (strong signal the expected token is simply absent),
                 * OR if lookahead is in FIRST of the immediate next NT on the stack
                 * (indicates the expected terminal is missing and the NT's content follows). */
                int isC1 = 0;
                int scanLimit = (sp > 8) ? sp - 8 : 0;
                for (int si = sp - 1; si >= scanLimit && !isC1; si--) {
                    if (stack[si].sym < 0) {
                        /* Non-terminal: check FOLLOW set */
                        NonTerminal anc = (NonTerminal)(-(stack[si].sym) - 1);
                        if (cur.type < NUM_TERMINALS && F->followSet[anc][cur.type])
                            isC1 = 1;
                    } else {
                        /* Terminal: lookahead matches something waiting on stack */
                        if (stack[si].sym == (int)cur.type)
                            isC1 = 1;
                    }
                }
                /* Also C1 if lookahead is in FIRST of the very next NT just below:
                 * the expected terminal is absent, but the NT that follows can
                 * start with cur (e.g. TK_ASSIGNOP missing, TK_NUM starts arithExpr). */
                if (!isC1 && sp - 1 >= 0 && stack[sp-1].sym < 0) {
                    NonTerminal nextNT = (NonTerminal)(-(stack[sp-1].sym) - 1);
                    if (cur.type < NUM_TERMINALS && F->firstSet[nextNT][cur.type])
                        isC1 = 1;
                }

                fprintf(stderr,
                    "Line %u\tError: The token %s for lexeme %s  does not match"
                    " with the expected token %s\n",
                    errLine,
                    tokenTypeName(cur.type), cur.lexeme,
                    tokenTypeName((TokenType)sym));
                syntaxErrors++;

                if (!isC1)
                    idx++; /* C2: consume spurious token */
                /* C1: don't consume; let parent context handle it */
            }

        /* ---- Non-terminal on stack ---- */
        } else {
            NonTerminal nt  = (NonTerminal)(-sym - 1);
            int prod = PT_ERROR;
            if (cur.type >= 0 && cur.type < NUM_TERMINALS)
                prod = T->table[nt][cur.type];

            if (prod >= 0) {
                /* Normal: expand the rule */
                Rule *r = &G->rules[prod];
                if (r->rhsLen == 0) {
                    /* epsilon expansion.
                     * Special case: when otherStmts expands to epsilon with
                     * TK_ENDIF as lookahead, check whether TK_ENDIF is already
                     * waiting on the stack (meaning we are inside an elsePart
                     * that legitimately expects endif).  If TK_ENDIF is nowhere
                     * on the stack, endif is in the wrong scope — report it. */
                    if (nt == NT_OTHER_STMTS && cur.type == TK_ENDIF) {
                        int endifOnStack = 0;
                        for (int si = sp - 1; si >= 0; si--) {
                            /* TK_ENDIF terminal already pushed (elsePart expanded) */
                            if (stack[si].sym == (int)TK_ENDIF) {
                                endifOnStack = 1; break;
                            }
                            /* elsePart NT not yet expanded — it will produce TK_ENDIF */
                            if (stack[si].sym == -(NT_ELSE_PART + 1)) {
                                endifOnStack = 1; break;
                            }
                        }
                        if (!endifOnStack) {
                            fprintf(stderr,
                                "Line %u\tError: Invalid token %s encountered"
                                " with value %s stack top %s\n",
                                cur.lineNo,
                                tokenTypeName(cur.type), cur.lexeme,
                                nonTerminalName(nt));
                            syntaxErrors++;
                        }
                    }
                    /* epsilon – nothing to push */
                } else {
                    ParseTreeNode *children[MAX_RHS_LEN];
                    int            childSym[MAX_RHS_LEN];
                    int childCount = 0;
                    for (int i = 0; i < r->rhsLen; ++i) {
                        GrammarSymbol gs = r->rhs[i];
                        int sVal;
                        ParseTreeNode *child;
                        if (gs.kind == SYM_TERMINAL) {
                            sVal  = gs.sym.terminal;
                            child = newNode(sVal);
                        } else {
                            sVal  = -(gs.sym.nonTerminal + 1);
                            child = newNode(sVal);
                        }
                        child->parent     = node;
                        children[childCount] = child;
                        childSym[childCount] = sVal;
                        childCount++;
                    }
                    for (int i = 0; i < childCount; ++i) {
                        if (i == 0) node->firstChild = children[i];
                        else children[i-1]->nextSibling = children[i];
                    }
                    for (int i = childCount - 1; i >= 0; --i) {
                        stack[sp].sym  = childSym[i];
                        stack[sp].node = children[i];
                        sp++;
                    }
                }

            } else if (prod == PT_SYNCH) {
                int endAnchor = (cur.type == TK_END      || cur.type == TK_ENDWHILE  ||
                                cur.type == TK_ENDRECORD || cur.type == TK_ENDUNION  ||
                                cur.type == TK_EOF);
                if (!endAnchor || nt == NT_RETURN_STMT) {
                    // For missing returnStmt, blame prevLine (where return should have been),
                    // not cur.lineNo (the 'end' token that revealed the absence).
                    unsigned int errLine = (nt == NT_RETURN_STMT) ? prevLine : cur.lineNo;
                    fprintf(stderr,
                        "Line %u\tError: Invalid token %s encountered with value %s"
                        " stack top %s\n",
                        errLine,
                        tokenTypeName(cur.type), cur.lexeme,
                        nonTerminalName(nt));
                    syntaxErrors++;
                }
            } else {
                /* PT_ERROR: lookahead not in FIRST(nt) or FOLLOW(nt).
                 *
                 * If the lookahead is a structural anchor (else, endif, end,
                 * then, while-end, ;, return, etc.) it almost certainly belongs
                 * to an outer context and is NOT garbage to skip.  Pop the NT
                 * silently so the outer rule can consume the anchor cleanly.
                 * This prevents expression-level NTs (termPrime, expPrime, …)
                 * from eating structural tokens and causing error cascades.
                 *
                 * For true garbage tokens, report and consume. */
                int isAnchor = (cur.type == TK_ELSE   || cur.type == TK_ENDIF  ||
                                cur.type == TK_THEN   || cur.type == TK_END    ||
                                cur.type == TK_ENDWHILE|| cur.type == TK_ENDRECORD ||
                                cur.type == TK_ENDUNION|| cur.type == TK_SQR   ||
                                cur.type == TK_RETURN || cur.type == TK_SEM    ||
                                cur.type == TK_EOF);
                if (isAnchor) {
                    /* silently pop NT; anchor stays for parent context */
                } else {
                    fprintf(stderr,
                        "Line %u\tError: Invalid token %s encountered with value %s"
                        " stack top %s\n",
                        cur.lineNo,
                        tokenTypeName(cur.type), cur.lexeme,
                        nonTerminalName(nt));
                    syntaxErrors++;
                    idx++; /* consume garbage token */
                }
            }
        }
    }

    if (syntaxErrors == 0)
        printf("Input source code is syntactically correct...........\n");
    else
        printf("Input source code has %d syntax error(s)\n", syntaxErrors);

    return PT;
}

/* ------------------------------------------------------------------
 * Parse tree printing (inorder traversal)
 * ------------------------------------------------------------------ */

static void printNodeInfo(FILE *out, ParseTreeNode *node) {
    if (!node) return;

    char lexeme[64] = "----";
    unsigned int lineno = 0;
    char tokName[32] = "";
    char value[64] = "";
    char parentSym[64] = "ROOT";
    char isLeafStr[4] = "no";
    char nodeSym[64] = "";

    if (node->parent) {
        if (node->parent->isLeaf) {
            strcpy(parentSym, "TERMINAL");
        } else {
            strcpy(parentSym, nonTerminalName(node->parent->nt));
        }
    }

    if (node->isLeaf) {
        strcpy(lexeme, node->token.lexeme);
        lineno = node->token.lineNo;
        strcpy(tokName, tokenTypeName(node->token.type));
        if (node->token.type == TK_NUM || node->token.type == TK_RNUM) {
            strcpy(value, node->token.lexeme);
        }
        strcpy(isLeafStr, "yes");
        strcpy(nodeSym, "----");
    } else {
        strcpy(tokName, "NONTERMINAL");
        strcpy(nodeSym, nonTerminalName(node->nt));
    }

    fprintf(out, "%-10s %-5u %-15s %-10s %-12s %-3s %-s\n",
            lexeme, lineno, tokName, value, parentSym, isLeafStr, nodeSym);
}

static void inorderTraverse(FILE *out, ParseTreeNode *node) {
    if (!node) return;
    inorderTraverse(out, node->firstChild);
    printNodeInfo(out, node);
    inorderTraverse(out, node->nextSibling);
}

void printParseTree(ParseTree *PT, const char *outfile) {
    if (!PT || !outfile) return;
    FILE *out = fopen(outfile, "w");
    if (!out) return;

    fprintf(out, "lexeme     lineno tokenName       value      parentNode  leaf nodeSymbol\n");
    fprintf(out, "-------------------------------------------------------------------\n");
    inorderTraverse(out, PT->root);
    fclose(out);
}