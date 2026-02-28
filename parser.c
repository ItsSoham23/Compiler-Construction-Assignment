#include "parser.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>

/* ================= GLOBAL STATE ================= */

static TokenList tokens;
static int idx = 0;
static Token *lookahead;
static FILE *parseTreeFile;

/* ================= UTILITIES ================= */

static Token* LA() {
    if (idx < tokens.size) return &tokens.buf[idx];
    return NULL;
}

static void advance() {
    if (idx < tokens.size) idx++;
    lookahead = LA();
}

static void syntaxError(const char *msg) {
    if (lookahead)
        printf("Line %d : Syntax Error: %s (found %d)\n",
               lookahead->lineNo, msg, lookahead->type);
    else
        printf("Syntax Error at EOF: %s\n", msg);
    exit(1);
}

static void match(TokenType t) {
    if (lookahead && lookahead->type == t)
        advance();
    else
        syntaxError("unexpected token");
}

/* ================= PARSE TREE ================= */

typedef struct PTNode {
    char label[32];
    struct PTNode *child;
    struct PTNode *sibling;
} PTNode;

static PTNode* newNode(const char *s) {
    PTNode *n = malloc(sizeof(PTNode));
    strcpy(n->label, s);
    n->child = n->sibling = NULL;
    return n;
}

static void printTree(PTNode *root, int depth) {
    if (!root) return;
    for (int i = 0; i < depth; i++) fprintf(parseTreeFile, "  ");
    fprintf(parseTreeFile, "%s\n", root->label);
    printTree(root->child, depth + 1);
    printTree(root->sibling, depth);
}

/* ================= FORWARD DECLS ================= */

static PTNode* program();
static PTNode* otherFunctions();
static PTNode* functionDef();
static PTNode* mainFunction();
static PTNode* stmts();
static PTNode* stmt();
static PTNode* assignmentStmt();
static PTNode* ioStmt();
static PTNode* returnStmt();
static PTNode* arithmeticExpression();

/* ================= GRAMMAR ================= */

/* <program> → <otherFunctions> <mainFunction> */
static PTNode* program() {
    PTNode *n = newNode("program");
    n->child = otherFunctions();
    n->child->sibling = mainFunction();
    return n;
}

/* <otherFunctions> → <function> <otherFunctions> | ε */
static PTNode* otherFunctions() {
    PTNode *n = newNode("otherFunctions");
    if (lookahead && lookahead->type == TK_FUNID) {
        n->child = functionDef();
        n->child->sibling = otherFunctions();
    }
    return n;
}

/* <function> → TK_FUNID ... TK_END */
static PTNode* functionDef() {
    PTNode *n = newNode("function");
    match(TK_FUNID);

    /* input parameter list */
    match(TK_INPUT);
    match(TK_PARAMETER);
    match(TK_LIST);
    match(TK_SQL);
    while (lookahead && lookahead->type != TK_SQR)
        advance();
    match(TK_SQR);

    /* optional output parameters */
    if (lookahead->type == TK_OUTPUT) {
        advance();
        match(TK_PARAMETER);
        match(TK_LIST);
        match(TK_SQL);
        while (lookahead && lookahead->type != TK_SQR)
            advance();
        match(TK_SQR);
    }

    match(TK_SEM);
    n->child = stmts();
    match(TK_END);
    return n;
}

/* <mainFunction> → TK_MAIN <stmts> TK_END */
static PTNode* mainFunction() {
    PTNode *n = newNode("mainFunction");
    match(TK_MAIN);
    n->child = stmts();
    match(TK_END);
    return n;
}

/* <stmts> → stmt* returnStmt */
static PTNode* stmts() {
    PTNode *n = newNode("stmts");
    PTNode *cur = NULL;

    while (lookahead &&
           lookahead->type != TK_RETURN &&
           lookahead->type != TK_END) {

        PTNode *s = stmt();
        if (!n->child) n->child = s;
        else cur->sibling = s;
        cur = s;
    }

    PTNode *r = returnStmt();
    if (cur) cur->sibling = r;
    else n->child = r;

    return n;
}

/* <stmt> → assignment | io */
static PTNode* stmt() {
    if (lookahead->type == TK_ID)
        return assignmentStmt();
    if (lookahead->type == TK_READ || lookahead->type == TK_WRITE)
        return ioStmt();

    syntaxError("invalid statement");
    return NULL;
}

/* assignment → ID <--- expr ; */
static PTNode* assignmentStmt() {
    PTNode *n = newNode("assignment");
    match(TK_ID);
    match(TK_ASSIGNOP);
    n->child = arithmeticExpression();
    match(TK_SEM);
    return n;
}

/* read/write */
static PTNode* ioStmt() {
    PTNode *n = newNode("ioStmt");
    advance(); /* read/write */
    match(TK_OP);
    match(TK_ID);
    match(TK_CL);
    match(TK_SEM);
    return n;
}

/* return → return ; | return [ idlist ] ; */
static PTNode* returnStmt() {
    PTNode *n = newNode("returnStmt");
    match(TK_RETURN);
    if (lookahead->type == TK_SQL) {
        match(TK_SQL);
        while (lookahead->type != TK_SQR)
            advance();
        match(TK_SQR);
    }
    match(TK_SEM);
    return n;
}

/* VERY SIMPLE expr (can be expanded later) */
static PTNode* arithmeticExpression() {
    PTNode *n = newNode("expr");
    if (lookahead->type == TK_ID ||
        lookahead->type == TK_NUM ||
        lookahead->type == TK_RNUM) {
        advance();
        return n;
    }
    syntaxError("invalid expression");
    return NULL;
}

/* ================= ENTRY ================= */

void parseInputSourceCode(char *sourceFile, char *outFile) {

    State s = initializeState(sourceFile);
    tokens = scan(&s);

    /* report lexical errors */
    for (int i = 0; i < tokens.size; i++) {
        if (tokens.buf[i].type == TK_ERROR)
            printf("Line %d : Lexical Error: %s\n",
                   tokens.buf[i].lineNo,
                   tokens.buf[i].errMsg);
    }

    idx = 0;
    lookahead = LA();

    parseTreeFile = fopen(outFile, "w");
    if (!parseTreeFile) {
        printf("Cannot open parse tree output file\n");
        return;
    }

    PTNode *root = program();
    printTree(root, 0);

    fclose(parseTreeFile);
    printf("Parsing completed successfully.\n");
}