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
    Rule *r = &G->rules[G->numRules++];
    r->lhs    = lhs;
    r->rhsLen = rhsLen;
    for (int i = 0; i < rhsLen; i++) r->rhs[i] = rhs[i];
}

/* ============================================================
 * loadGrammar – hard-coded grammar matching grammar.txt
 * (We hard-code for correctness; grammar.txt is the human-readable ref)
 * ============================================================ */
Grammar loadGrammar(const char *grammarFile) {
    (void)grammarFile; /* not parsed; grammar is hard-coded */
    Grammar G;
    memset(&G, 0, sizeof(Grammar));
    GrammarSymbol rhs[MAX_RHS_LEN];
    int n;

    /* R0: <program> ===> <otherFunctions> <mainFunction> */
    n=0; rhs[n++]=NT(NT_OTHER_FUNCTIONS); rhs[n++]=NT(NT_MAIN_FUNCTION);
    addRule(&G, NT_PROGRAM, rhs, n);

    /* R1: <mainFunction> ===> TK_MAIN <stmts> TK_END */
    n=0; rhs[n++]=T(TK_MAIN); rhs[n++]=NT(NT_STMTS); rhs[n++]=T(TK_END);
    addRule(&G, NT_MAIN_FUNCTION, rhs, n);

    /* R2: <otherFunctions> ===> <function> <otherFunctions> */
    n=0; rhs[n++]=NT(NT_FUNCTION); rhs[n++]=NT(NT_OTHER_FUNCTIONS);
    addRule(&G, NT_OTHER_FUNCTIONS, rhs, n);

    /* R3: <otherFunctions> ===> eps */
    addRule(&G, NT_OTHER_FUNCTIONS, rhs, 0);

    /* R4: <function> ===> TK_FUNID <input_par> <output_par> TK_SEM <stmts> TK_END */
    n=0; rhs[n++]=T(TK_FUNID); rhs[n++]=NT(NT_INPUT_PAR); rhs[n++]=NT(NT_OUTPUT_PAR);
    rhs[n++]=T(TK_SEM); rhs[n++]=NT(NT_STMTS); rhs[n++]=T(TK_END);
    addRule(&G, NT_FUNCTION, rhs, n);

    /* R5: <input_par> ===> TK_INPUT TK_PARAMETER TK_LIST TK_SQL <parameter_list> TK_SQR */
    n=0; rhs[n++]=T(TK_INPUT); rhs[n++]=T(TK_PARAMETER); rhs[n++]=T(TK_LIST);
    rhs[n++]=T(TK_SQL); rhs[n++]=NT(NT_PARAMETER_LIST); rhs[n++]=T(TK_SQR);
    addRule(&G, NT_INPUT_PAR, rhs, n);

    /* R6: <output_par> ===> TK_OUTPUT TK_PARAMETER TK_LIST TK_SQL <parameter_list> TK_SQR */
    n=0; rhs[n++]=T(TK_OUTPUT); rhs[n++]=T(TK_PARAMETER); rhs[n++]=T(TK_LIST);
    rhs[n++]=T(TK_SQL); rhs[n++]=NT(NT_PARAMETER_LIST); rhs[n++]=T(TK_SQR);
    addRule(&G, NT_OUTPUT_PAR, rhs, n);

    /* R7: <output_par> ===> eps */
    addRule(&G, NT_OUTPUT_PAR, rhs, 0);

    /* R8: <parameter_list> ===> <dataType> TK_ID <remaining_list> */
    n=0; rhs[n++]=NT(NT_DATATYPE); rhs[n++]=T(TK_ID); rhs[n++]=NT(NT_REMAINING_LIST);
    addRule(&G, NT_PARAMETER_LIST, rhs, n);

    /* R9: <dataType> ===> <primitiveDatatype> */
    n=0; rhs[n++]=NT(NT_PRIMITIVE_DATATYPE);
    addRule(&G, NT_DATATYPE, rhs, n);

    /* R10: <dataType> ===> <constructedDatatype> */
    n=0; rhs[n++]=NT(NT_CONSTRUCTED_DATATYPE);
    addRule(&G, NT_DATATYPE, rhs, n);

    /* R11: <primitiveDatatype> ===> TK_INT */
    n=0; rhs[n++]=T(TK_INT);
    addRule(&G, NT_PRIMITIVE_DATATYPE, rhs, n);

    /* R12: <primitiveDatatype> ===> TK_REAL */
    n=0; rhs[n++]=T(TK_REAL);
    addRule(&G, NT_PRIMITIVE_DATATYPE, rhs, n);

    /* R13: <constructedDatatype> ===> TK_RECORD TK_RUID */
    n=0; rhs[n++]=T(TK_RECORD); rhs[n++]=T(TK_RUID);
    addRule(&G, NT_CONSTRUCTED_DATATYPE, rhs, n);

    /* R14: <constructedDatatype> ===> TK_UNION TK_RUID */
    n=0; rhs[n++]=T(TK_UNION); rhs[n++]=T(TK_RUID);
    addRule(&G, NT_CONSTRUCTED_DATATYPE, rhs, n);

    /* R15: <constructedDatatype> ===> TK_RUID */
    n=0; rhs[n++]=T(TK_RUID);
    addRule(&G, NT_CONSTRUCTED_DATATYPE, rhs, n);

    /* R16: <remaining_list> ===> TK_COMMA <parameter_list> */
    n=0; rhs[n++]=T(TK_COMMA); rhs[n++]=NT(NT_PARAMETER_LIST);
    addRule(&G, NT_REMAINING_LIST, rhs, n);

    /* R17: <remaining_list> ===> eps */
    addRule(&G, NT_REMAINING_LIST, rhs, 0);

    /* R18: <stmts> ===> <typeDefinitions> <declarations> <otherStmts> <returnStmt> */
    n=0; rhs[n++]=NT(NT_TYPE_DEFINITIONS); rhs[n++]=NT(NT_DECLARATIONS);
    rhs[n++]=NT(NT_OTHER_STMTS); rhs[n++]=NT(NT_RETURN_STMT);
    addRule(&G, NT_STMTS, rhs, n);

    /* R19: <typeDefinitions> ===> <actualOrRedefined> <typeDefinitions> */
    n=0; rhs[n++]=NT(NT_ACTUAL_OR_REDEFINED); rhs[n++]=NT(NT_TYPE_DEFINITIONS);
    addRule(&G, NT_TYPE_DEFINITIONS, rhs, n);

    /* R20: <typeDefinitions> ===> eps */
    addRule(&G, NT_TYPE_DEFINITIONS, rhs, 0);

    /* R21: <actualOrRedefined> ===> <typeDefinition> */
    n=0; rhs[n++]=NT(NT_TYPE_DEFINITION);
    addRule(&G, NT_ACTUAL_OR_REDEFINED, rhs, n);

    /* R22: <actualOrRedefined> ===> <definetypestmt> */
    n=0; rhs[n++]=NT(NT_DEFINE_TYPE_STMT);
    addRule(&G, NT_ACTUAL_OR_REDEFINED, rhs, n);

    /* R23: <typeDefinition> ===> TK_RECORD TK_RUID <fieldDefinitions> TK_ENDRECORD */
    n=0; rhs[n++]=T(TK_RECORD); rhs[n++]=T(TK_RUID); rhs[n++]=NT(NT_FIELD_DEFINITIONS); rhs[n++]=T(TK_ENDRECORD);
    addRule(&G, NT_TYPE_DEFINITION, rhs, n);

    /* R24: <typeDefinition> ===> TK_UNION TK_RUID <fieldDefinitions> TK_ENDUNION */
    n=0; rhs[n++]=T(TK_UNION); rhs[n++]=T(TK_RUID); rhs[n++]=NT(NT_FIELD_DEFINITIONS); rhs[n++]=T(TK_ENDUNION);
    addRule(&G, NT_TYPE_DEFINITION, rhs, n);

    /* R25: <fieldDefinitions> ===> <fieldDefinition> <fieldDefinition> <moreFields> */
    n=0; rhs[n++]=NT(NT_FIELD_DEFINITION); rhs[n++]=NT(NT_FIELD_DEFINITION); rhs[n++]=NT(NT_MORE_FIELDS);
    addRule(&G, NT_FIELD_DEFINITIONS, rhs, n);

    /* R26: <fieldDefinition> ===> TK_TYPE <fieldType> TK_COLON TK_FIELDID TK_SEM */
    n=0; rhs[n++]=T(TK_TYPE); rhs[n++]=NT(NT_FIELD_TYPE); rhs[n++]=T(TK_COLON); rhs[n++]=T(TK_FIELDID); rhs[n++]=T(TK_SEM);
    addRule(&G, NT_FIELD_DEFINITION, rhs, n);

    /* R27: <fieldType> ===> <primitiveDatatype> */
    n=0; rhs[n++]=NT(NT_PRIMITIVE_DATATYPE);
    addRule(&G, NT_FIELD_TYPE, rhs, n);

    /* R28: <fieldType> ===> <constructedDatatype> */
    n=0; rhs[n++]=NT(NT_CONSTRUCTED_DATATYPE);
    addRule(&G, NT_FIELD_TYPE, rhs, n);

    /* R29: <moreFields> ===> <fieldDefinition> <moreFields> */
    n=0; rhs[n++]=NT(NT_FIELD_DEFINITION); rhs[n++]=NT(NT_MORE_FIELDS);
    addRule(&G, NT_MORE_FIELDS, rhs, n);

    /* R30: <moreFields> ===> eps */
    addRule(&G, NT_MORE_FIELDS, rhs, 0);

    /* R31: <declarations> ===> <declaration> <declarations> */
    n=0; rhs[n++]=NT(NT_DECLARATION); rhs[n++]=NT(NT_DECLARATIONS);
    addRule(&G, NT_DECLARATIONS, rhs, n);

    /* R32: <declarations> ===> eps */
    addRule(&G, NT_DECLARATIONS, rhs, 0);

    /* R33: <declaration> ===> TK_TYPE <dataType> TK_COLON TK_ID <global_or_not> TK_SEM */
    n=0; rhs[n++]=T(TK_TYPE); rhs[n++]=NT(NT_DATATYPE); rhs[n++]=T(TK_COLON);
    rhs[n++]=T(TK_ID); rhs[n++]=NT(NT_GLOBAL_OR_NOT); rhs[n++]=T(TK_SEM);
    addRule(&G, NT_DECLARATION, rhs, n);

    /* R34: <global_or_not> ===> TK_COLON TK_GLOBAL */
    n=0; rhs[n++]=T(TK_COLON); rhs[n++]=T(TK_GLOBAL);
    addRule(&G, NT_GLOBAL_OR_NOT, rhs, n);

    /* R35: <global_or_not> ===> eps */
    addRule(&G, NT_GLOBAL_OR_NOT, rhs, 0);

    /* R36: <otherStmts> ===> <stmt> <otherStmts> */
    n=0; rhs[n++]=NT(NT_STMT); rhs[n++]=NT(NT_OTHER_STMTS);
    addRule(&G, NT_OTHER_STMTS, rhs, n);

    /* R37: <otherStmts> ===> eps */
    addRule(&G, NT_OTHER_STMTS, rhs, 0);

    /* R38: <stmt> ===> <assignmentStmt> */
    n=0; rhs[n++]=NT(NT_ASSIGNMENT_STMT);
    addRule(&G, NT_STMT, rhs, n);

    /* R39: <stmt> ===> <iterativeStmt> */
    n=0; rhs[n++]=NT(NT_ITERATIVE_STMT);
    addRule(&G, NT_STMT, rhs, n);

    /* R40: <stmt> ===> <conditionalStmt> */
    n=0; rhs[n++]=NT(NT_CONDITIONAL_STMT);
    addRule(&G, NT_STMT, rhs, n);

    /* R41: <stmt> ===> <ioStmt> */
    n=0; rhs[n++]=NT(NT_IO_STMT);
    addRule(&G, NT_STMT, rhs, n);

    /* R42: <stmt> ===> <funCallStmt> */
    n=0; rhs[n++]=NT(NT_FUN_CALL_STMT);
    addRule(&G, NT_STMT, rhs, n);

    /* R43: <assignmentStmt> ===> <singleOrRecId> TK_ASSIGNOP <arithmeticExpression> TK_SEM */
    n=0; rhs[n++]=NT(NT_SINGLE_OR_REC_ID); rhs[n++]=T(TK_ASSIGNOP);
    rhs[n++]=NT(NT_ARITHMETIC_EXPRESSION); rhs[n++]=T(TK_SEM);
    addRule(&G, NT_ASSIGNMENT_STMT, rhs, n);

    /* R44: <singleOrRecId> ===> TK_ID <option_single_constructed> */
    n=0; rhs[n++]=T(TK_ID); rhs[n++]=NT(NT_OPTION_SINGLE_CONSTRUCTED);
    addRule(&G, NT_SINGLE_OR_REC_ID, rhs, n);

    /* R45: <option_single_constructed> ===> eps */
    addRule(&G, NT_OPTION_SINGLE_CONSTRUCTED, rhs, 0);

    /* R46: <option_single_constructed> ===> <oneExpansion> <moreExpansions> */
    n=0; rhs[n++]=NT(NT_ONE_EXPANSION); rhs[n++]=NT(NT_MORE_EXPANSIONS);
    addRule(&G, NT_OPTION_SINGLE_CONSTRUCTED, rhs, n);

    /* R47: <oneExpansion> ===> TK_DOT TK_FIELDID */
    n=0; rhs[n++]=T(TK_DOT); rhs[n++]=T(TK_FIELDID);
    addRule(&G, NT_ONE_EXPANSION, rhs, n);

    /* R48: <moreExpansions> ===> <oneExpansion> <moreExpansions> */
    n=0; rhs[n++]=NT(NT_ONE_EXPANSION); rhs[n++]=NT(NT_MORE_EXPANSIONS);
    addRule(&G, NT_MORE_EXPANSIONS, rhs, n);

    /* R49: <moreExpansions> ===> eps */
    addRule(&G, NT_MORE_EXPANSIONS, rhs, 0);

    /* R50: <funCallStmt> ===> <outputParameters> TK_CALL TK_FUNID TK_WITH TK_PARAMETERS <inputParameters> TK_SEM */
    n=0; rhs[n++]=NT(NT_OUTPUT_PARAMETERS); rhs[n++]=T(TK_CALL); rhs[n++]=T(TK_FUNID);
    rhs[n++]=T(TK_WITH); rhs[n++]=T(TK_PARAMETERS); rhs[n++]=NT(NT_INPUT_PARAMETERS); rhs[n++]=T(TK_SEM);
    addRule(&G, NT_FUN_CALL_STMT, rhs, n);

    /* R51: <outputParameters> ===> TK_SQL <idList> TK_SQR TK_ASSIGNOP */
    n=0; rhs[n++]=T(TK_SQL); rhs[n++]=NT(NT_ID_LIST); rhs[n++]=T(TK_SQR); rhs[n++]=T(TK_ASSIGNOP);
    addRule(&G, NT_OUTPUT_PARAMETERS, rhs, n);

    /* R52: <outputParameters> ===> eps */
    addRule(&G, NT_OUTPUT_PARAMETERS, rhs, 0);

    /* R53: <inputParameters> ===> TK_SQL <idList> TK_SQR */
    n=0; rhs[n++]=T(TK_SQL); rhs[n++]=NT(NT_ID_LIST); rhs[n++]=T(TK_SQR);
    addRule(&G, NT_INPUT_PARAMETERS, rhs, n);

    /* R54: <iterativeStmt> ===> TK_WHILE TK_OP <booleanExpression> TK_CL <stmt> <otherStmts> TK_ENDWHILE */
    n=0; rhs[n++]=T(TK_WHILE); rhs[n++]=T(TK_OP); rhs[n++]=NT(NT_BOOLEAN_EXPRESSION);
    rhs[n++]=T(TK_CL); rhs[n++]=NT(NT_STMT); rhs[n++]=NT(NT_OTHER_STMTS); rhs[n++]=T(TK_ENDWHILE);
    addRule(&G, NT_ITERATIVE_STMT, rhs, n);

    /* R55: <conditionalStmt> ===> TK_IF TK_OP <booleanExpression> TK_CL TK_THEN <stmt> <otherStmts> <elsePart> */
    n=0; rhs[n++]=T(TK_IF); rhs[n++]=T(TK_OP); rhs[n++]=NT(NT_BOOLEAN_EXPRESSION);
    rhs[n++]=T(TK_CL); rhs[n++]=T(TK_THEN); rhs[n++]=NT(NT_STMT);
    rhs[n++]=NT(NT_OTHER_STMTS); rhs[n++]=NT(NT_ELSE_PART);
    addRule(&G, NT_CONDITIONAL_STMT, rhs, n);

    /* R56: <elsePart> ===> TK_ELSE <stmt> <otherStmts> TK_ENDIF */
    n=0; rhs[n++]=T(TK_ELSE); rhs[n++]=NT(NT_STMT); rhs[n++]=NT(NT_OTHER_STMTS); rhs[n++]=T(TK_ENDIF);
    addRule(&G, NT_ELSE_PART, rhs, n);

    /* R57: <elsePart> ===> TK_ENDIF */
    n=0; rhs[n++]=T(TK_ENDIF);
    addRule(&G, NT_ELSE_PART, rhs, n);

    /* R58: <ioStmt> ===> TK_READ TK_OP <var> TK_CL TK_SEM */
    n=0; rhs[n++]=T(TK_READ); rhs[n++]=T(TK_OP); rhs[n++]=NT(NT_VAR); rhs[n++]=T(TK_CL); rhs[n++]=T(TK_SEM);
    addRule(&G, NT_IO_STMT, rhs, n);

    /* R59: <ioStmt> ===> TK_WRITE TK_OP <var> TK_CL TK_SEM */
    n=0; rhs[n++]=T(TK_WRITE); rhs[n++]=T(TK_OP); rhs[n++]=NT(NT_VAR); rhs[n++]=T(TK_CL); rhs[n++]=T(TK_SEM);
    addRule(&G, NT_IO_STMT, rhs, n);

    /* B1: <arithmeticExpression> ===> <term> <expPrime> */
    n=0; rhs[n++]=NT(NT_TERM); rhs[n++]=NT(NT_EXP_PRIME);
    addRule(&G, NT_ARITHMETIC_EXPRESSION, rhs, n);

    /* B2a: <expPrime> ===> <lowPrecedenceOperators> <term> <expPrime> */
    n=0; rhs[n++]=NT(NT_LOW_PREC_OPERATORS); rhs[n++]=NT(NT_TERM); rhs[n++]=NT(NT_EXP_PRIME);
    addRule(&G, NT_EXP_PRIME, rhs, n);

    /* B2b: <expPrime> ===> eps */
    addRule(&G, NT_EXP_PRIME, rhs, 0);

    /* B3: <term> ===> <factor> <termPrime> */
    n=0; rhs[n++]=NT(NT_FACTOR); rhs[n++]=NT(NT_TERM_PRIME);
    addRule(&G, NT_TERM, rhs, n);

    /* B4a: <termPrime> ===> <highPrecedenceOperators> <factor> <termPrime> */
    n=0; rhs[n++]=NT(NT_HIGH_PREC_OPERATORS); rhs[n++]=NT(NT_FACTOR); rhs[n++]=NT(NT_TERM_PRIME);
    addRule(&G, NT_TERM_PRIME, rhs, n);

    /* B4b: <termPrime> ===> eps */
    addRule(&G, NT_TERM_PRIME, rhs, 0);

    /* B5a: <factor> ===> TK_OP <arithmeticExpression> TK_CL */
    n=0; rhs[n++]=T(TK_OP); rhs[n++]=NT(NT_ARITHMETIC_EXPRESSION); rhs[n++]=T(TK_CL);
    addRule(&G, NT_FACTOR, rhs, n);

    /* B5b: <factor> ===> <var> */
    n=0; rhs[n++]=NT(NT_VAR);
    addRule(&G, NT_FACTOR, rhs, n);

    /* B6a: <highPrecedenceOperators> ===> TK_MUL */
    n=0; rhs[n++]=T(TK_MUL);
    addRule(&G, NT_HIGH_PREC_OPERATORS, rhs, n);

    /* B6b: <highPrecedenceOperators> ===> TK_DIV */
    n=0; rhs[n++]=T(TK_DIV);
    addRule(&G, NT_HIGH_PREC_OPERATORS, rhs, n);

    /* B7a: <lowPrecedenceOperators> ===> TK_PLUS */
    n=0; rhs[n++]=T(TK_PLUS);
    addRule(&G, NT_LOW_PREC_OPERATORS, rhs, n);

    /* B7b: <lowPrecedenceOperators> ===> TK_MINUS */
    n=0; rhs[n++]=T(TK_MINUS);
    addRule(&G, NT_LOW_PREC_OPERATORS, rhs, n);

    /* R60: <booleanExpression> ===> TK_OP <booleanExpression> TK_CL <logicalOp> TK_OP <booleanExpression> TK_CL */
    n=0; rhs[n++]=T(TK_OP); rhs[n++]=NT(NT_BOOLEAN_EXPRESSION); rhs[n++]=T(TK_CL);
    rhs[n++]=NT(NT_LOGICAL_OP); rhs[n++]=T(TK_OP); rhs[n++]=NT(NT_BOOLEAN_EXPRESSION); rhs[n++]=T(TK_CL);
    addRule(&G, NT_BOOLEAN_EXPRESSION, rhs, n);

    /* R61: <booleanExpression> ===> <var> <relationalOp> <var> */
    n=0; rhs[n++]=NT(NT_VAR); rhs[n++]=NT(NT_RELATIONAL_OP); rhs[n++]=NT(NT_VAR);
    addRule(&G, NT_BOOLEAN_EXPRESSION, rhs, n);

    /* R62: <booleanExpression> ===> TK_NOT TK_OP <booleanExpression> TK_CL */
    n=0; rhs[n++]=T(TK_NOT); rhs[n++]=T(TK_OP); rhs[n++]=NT(NT_BOOLEAN_EXPRESSION); rhs[n++]=T(TK_CL);
    addRule(&G, NT_BOOLEAN_EXPRESSION, rhs, n);

    /* R63: <var> ===> <singleOrRecId> */
    n=0; rhs[n++]=NT(NT_SINGLE_OR_REC_ID);
    addRule(&G, NT_VAR, rhs, n);

    /* R64: <var> ===> TK_NUM */
    n=0; rhs[n++]=T(TK_NUM);
    addRule(&G, NT_VAR, rhs, n);

    /* R65: <var> ===> TK_RNUM */
    n=0; rhs[n++]=T(TK_RNUM);
    addRule(&G, NT_VAR, rhs, n);

    /* R66: <logicalOp> ===> TK_AND */
    n=0; rhs[n++]=T(TK_AND);
    addRule(&G, NT_LOGICAL_OP, rhs, n);

    /* R67: <logicalOp> ===> TK_OR */
    n=0; rhs[n++]=T(TK_OR);
    addRule(&G, NT_LOGICAL_OP, rhs, n);

    /* R68: <relationalOp> ===> TK_LT */
    n=0; rhs[n++]=T(TK_LT);  addRule(&G, NT_RELATIONAL_OP, rhs, n);

    /* R69: <relationalOp> ===> TK_LE */
    n=0; rhs[n++]=T(TK_LE);  addRule(&G, NT_RELATIONAL_OP, rhs, n);

    /* R70: <relationalOp> ===> TK_EQ */
    n=0; rhs[n++]=T(TK_EQ);  addRule(&G, NT_RELATIONAL_OP, rhs, n);

    /* R71: <relationalOp> ===> TK_GT */
    n=0; rhs[n++]=T(TK_GT);  addRule(&G, NT_RELATIONAL_OP, rhs, n);

    /* R72: <relationalOp> ===> TK_GE */
    n=0; rhs[n++]=T(TK_GE);  addRule(&G, NT_RELATIONAL_OP, rhs, n);

    /* R73: <relationalOp> ===> TK_NE */
    n=0; rhs[n++]=T(TK_NE);  addRule(&G, NT_RELATIONAL_OP, rhs, n);

    /* R74: <returnStmt> ===> TK_RETURN <optionalReturn> TK_SEM */
    n=0; rhs[n++]=T(TK_RETURN); rhs[n++]=NT(NT_OPTIONAL_RETURN); rhs[n++]=T(TK_SEM);
    addRule(&G, NT_RETURN_STMT, rhs, n);

    /* R75: <optionalReturn> ===> TK_SQL <idList> TK_SQR */
    n=0; rhs[n++]=T(TK_SQL); rhs[n++]=NT(NT_ID_LIST); rhs[n++]=T(TK_SQR);
    addRule(&G, NT_OPTIONAL_RETURN, rhs, n);

    /* R76: <optionalReturn> ===> eps */
    addRule(&G, NT_OPTIONAL_RETURN, rhs, 0);

    /* R77: <idList> ===> TK_ID <more_ids> */
    n=0; rhs[n++]=T(TK_ID); rhs[n++]=NT(NT_MORE_IDS);
    addRule(&G, NT_ID_LIST, rhs, n);

    /* R78: <more_ids> ===> TK_COMMA <idList> */
    n=0; rhs[n++]=T(TK_COMMA); rhs[n++]=NT(NT_ID_LIST);
    addRule(&G, NT_MORE_IDS, rhs, n);

    /* R79: <more_ids> ===> eps */
    addRule(&G, NT_MORE_IDS, rhs, 0);

    /* R80: <definetypestmt> ===> TK_DEFINETYPE <A> TK_RUID TK_AS TK_RUID */
    n=0; rhs[n++]=T(TK_DEFINETYPE); rhs[n++]=NT(NT_A); rhs[n++]=T(TK_RUID);
    rhs[n++]=T(TK_AS); rhs[n++]=T(TK_RUID);
    addRule(&G, NT_DEFINE_TYPE_STMT, rhs, n);

    /* R81: <A> ===> TK_RECORD */
    n=0; rhs[n++]=T(TK_RECORD);
    addRule(&G, NT_A, rhs, n);

    /* R82: <A> ===> TK_UNION */
    n=0; rhs[n++]=T(TK_UNION);
    addRule(&G, NT_A, rhs, n);

    return G;
}

/* ============================================================
 * FIRST / FOLLOW computation (automated)
 * ============================================================ */

/* Set a bit */
static void setBit(int *arr, int idx) { arr[idx] = 1; }
/* Merge src into dst, return 1 if dst changed */
static int mergeSets(int *dst, const int *src, int size) {
    int changed = 0;
    for (int i = 0; i < size; i++) {
        if (src[i] && !dst[i]) { dst[i] = 1; changed = 1; }
    }
    return changed;
}

static void computeFirst(Grammar *G, FirstAndFollow *FF) {
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int ri = 0; ri < G->numRules; ri++) {
            Rule *r = &G->rules[ri];
            NonTerminal lhs = r->lhs;
            if (r->rhsLen == 0) {
                if (!FF->nullable[lhs]) { FF->nullable[lhs] = 1; changed = 1; }
                continue;
            }
            /* Walk RHS and collect FIRST */
            int allNullable = 1;
            for (int k = 0; k < r->rhsLen; k++) {
                GrammarSymbol sym = r->rhs[k];
                if (sym.kind == SYM_TERMINAL) {
                    if (!FF->firstSet[lhs][sym.sym.terminal]) {
                        setBit(FF->firstSet[lhs], sym.sym.terminal);
                        changed = 1;
                    }
                    allNullable = 0;
                    break;
                } else {
                    NonTerminal nt = sym.sym.nonTerminal;
                    if (mergeSets(FF->firstSet[lhs], FF->firstSet[nt], NUM_TERMINALS)) changed = 1;
                    if (!FF->nullable[nt]) { allNullable = 0; break; }
                }
            }
            if (allNullable && !FF->nullable[lhs]) { FF->nullable[lhs] = 1; changed = 1; }
        }
    }
}

static void computeFollow(Grammar *G, FirstAndFollow *FF) {
    /* FOLLOW(start symbol) contains $ (TK_EOF) */
    FF->followSet[NT_PROGRAM][TK_EOF] = 1;

    int changed = 1;
    while (changed) {
        changed = 0;
        for (int ri = 0; ri < G->numRules; ri++) {
            Rule *r = &G->rules[ri];
            NonTerminal lhs = r->lhs;
            for (int k = 0; k < r->rhsLen; k++) {
                GrammarSymbol sym = r->rhs[k];
                if (sym.kind != SYM_NON_TERMINAL) continue;
                NonTerminal B = sym.sym.nonTerminal;
                /* Add FIRST of remaining symbols (skipping nullables) */
                int restNullable = 1;
                for (int j = k + 1; j < r->rhsLen; j++) {
                    GrammarSymbol next = r->rhs[j];
                    if (next.kind == SYM_TERMINAL) {
                        if (!FF->followSet[B][next.sym.terminal]) {
                            FF->followSet[B][next.sym.terminal] = 1; changed = 1;
                        }
                        restNullable = 0; break;
                    } else {
                        NonTerminal ntj = next.sym.nonTerminal;
                        if (mergeSets(FF->followSet[B], FF->firstSet[ntj], NUM_TERMINALS)) changed = 1;
                        if (!FF->nullable[ntj]) { restNullable = 0; break; }
                    }
                }
                if (restNullable) {
                    if (mergeSets(FF->followSet[B], FF->followSet[lhs], NUM_TERMINALS)) changed = 1;
                }
            }
        }
    }
}

FirstAndFollow computeFirstAndFollowSets(Grammar *G) {
    FirstAndFollow FF;
    memset(&FF, 0, sizeof(FirstAndFollow));
    computeFirst(G, &FF);
    computeFollow(G, &FF);
    return FF;
}

/* ============================================================
 * Parse Table construction
 *
 * Standard LL(1) table filling, then a second pass to stamp
 * PT_SYNCH on every PT_ERROR cell (A, a) where a is in FOLLOW(A).
 * ============================================================ */
ParseTable createParseTable(Grammar *G, FirstAndFollow *F) {
    ParseTable PT;
    /* Initialize all entries to PT_ERROR */
    for (int i = 0; i < NUM_NON_TERMINALS; i++)
        for (int j = 0; j < NUM_TERMINALS; j++)
            PT.table[i][j] = PT_ERROR;

    for (int ri = 0; ri < G->numRules; ri++) {
        Rule *r = &G->rules[ri];
        NonTerminal lhs = r->lhs;

        if (r->rhsLen == 0) {
            /* epsilon production: for each terminal in FOLLOW(lhs) */
            for (int t = 0; t < NUM_TERMINALS; t++) {
                if (F->followSet[lhs][t]) {
                    if (PT.table[lhs][t] == PT_ERROR)
                        PT.table[lhs][t] = ri;
                    else
                        fprintf(stderr, "[PARSER-WARNING] Parse table conflict at [%s][%s]\n",
                                nonTerminalName(lhs), tokenTypeName((TokenType)t));
                }
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
/* Helper to fetch next non-comment token from a TokenList buffer */
static tokenInfo nextFromList(TokenList *tl, int *idx) {
    tokenInfo tok;
    while (1) {
        if (*idx >= tl->size) {
            tok.type = TK_EOF;
            tok.lexeme[0] = '\0';
            tok.lineNo = 0;
            return tok;
        }
        tok = tl->buf[(*idx)++];
        if (tok.type != TK_COMMENT) return tok;
        /* skip comments */
    }
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
    /* Reset deduplication state */
    numReported  = 0;
    lastPanicLine = -1;
    memset(errLineCount, 0, sizeof(errLineCount));

    /* Initialize lexer state and obtain token list */
    State s = initializeState(testcaseFile);
    TokenList tokens = scan(&s);
    int tidx = 0; /* index into tokens */
    int syntaxErrors = 0;

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
    tokenInfo lookahead = nextFromList(&tokens, &tidx);
    /* Skip initial lexer errors */
    while (lookahead.type == TK_ERROR) lookahead = nextFromList(&tokens, &tidx);

    /* ======================================================== */
    while (!stackEmpty(&S)) {

        StackEntry top = S.items[S.top];

        /* Skip TK_ERROR tokens from lexer – already reported by lexer */
        if (lookahead.type == TK_ERROR) {
            lookahead = nextFromList(&tokens, &tidx);
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
                lookahead = nextFromList(&tokens, &tidx);
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
                lookahead = nextFromList(&tokens, &tidx);
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
                lookahead = nextFromList(&tokens, &tidx);
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

    /* lexer 'scan' closed the input FILE; nothing to close here */

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

        /* Value if number: use lexeme for numeric tokens */
        char valStr[32] = "---";
        if (node->token.type == TK_NUM || node->token.type == TK_RNUM) {
            strncpy(valStr, node->token.lexeme, sizeof(valStr)-1);
            valStr[sizeof(valStr)-1] = '\0';
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
