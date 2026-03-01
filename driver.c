/*
Group 1
Romit Jain - 2023A7PS0021P
Soham Vinay Deshmukh - 2023A7PS0025P
Nishant Amarish Pradhan - 2023A7PS0030P
Devesh Saraogi - 2023A7PS0242P
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lexer.h"
#include "parser.h"

/* Forward declaration */
void displayStatus();
void showMenu();

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <testcase.txt> <parsetreeOutFile.txt>\n", argv[0]);
        return 1;
    }

    const char* testcaseFile = argv[1];
    const char* parseTreeFile = argv[2];

    displayStatus();

    int choice;
    while (1) {
        showMenu();
        printf("\nEnter your choice (0-4): ");
        if (scanf("%d", &choice) != 1) {
            fprintf(stderr, "Invalid input. Please enter a number.\n");
            while (getchar() != '\n');  // Clear input buffer
            continue;
        }

        switch (choice) {
            case 0:
                printf("\nExiting program...\n");
                return 0;

             case 1:
                printf("\n========== Option 1: Removing Comments ==========\n");
                removeComments((char*)testcaseFile, "removedComments.txt");
                printf("\nOutput also written to: removedComments.txt\n");
                break;

            case 2:
                printf("\n========== Option 2: Printing Token List ==========\n");
                printf("Tokens from: %s\n", testcaseFile);
                {
                    static const char *tokenTypeNames[] = {
                        "TK_ASSIGNOP", "TK_COMMENT", "TK_FIELDID", "TK_ID",
                        "TK_NUM", "TK_RNUM", "TK_FUNID", "TK_RUID",
                        "TK_WITH", "TK_PARAMETERS", "TK_END", "TK_WHILE",
                        "TK_UNION", "TK_ENDUNION", "TK_DEFINETYPE", "TK_AS",
                        "TK_TYPE", "TK_MAIN", "TK_GLOBAL", "TK_PARAMETER",
                        "TK_LIST", "TK_SQL", "TK_SQR", "TK_INPUT",
                        "TK_OUTPUT", "TK_INT", "TK_REAL",
                        "TK_COMMA", "TK_SEM", "TK_COLON", "TK_DOT",
                        "TK_ENDWHILE", "TK_OP", "TK_CL", "TK_IF",
                        "TK_THEN", "TK_ENDIF", "TK_READ", "TK_WRITE",
                        "TK_RETURN", "TK_PLUS", "TK_MINUS", "TK_MUL", "TK_DIV",
                        "TK_CALL", "TK_RECORD", "TK_ENDRECORD", "TK_ELSE",
                        "TK_AND", "TK_OR", "TK_NOT",
                        "TK_LT", "TK_LE", "TK_EQ", "TK_GT", "TK_GE", "TK_NE",
                        "TK_ERROR"
                    };
                    FILE *fp2 = fopen(testcaseFile, "r");
                    if (!fp2) {
                        fprintf(stderr, "Error: Cannot open file %s\n", testcaseFile);
                    } else {
                        getStream(fp2);
                        int tokenCount = 0, errorCount = 0;
                        while (1) {
                            tokenInfo tk = getNextToken(NULL);
                            if (tk.type == TK_ERROR && tk.errMsg && strcmp(tk.errMsg, "EOF") == 0)
                                break;
                            if (tk.type == TK_ERROR) {
                                printf("Line %-4u  LEXICAL ERROR: %s\n",
                                       tk.lineNo,
                                       tk.errMsg ? tk.errMsg : tk.lexeme);
                                errorCount++;
                            } else {
                                printf("Line %-4u  Lexeme: %-24s  Token: %s\n",
                                       tk.lineNo,
                                       tk.lexeme,
                                       tokenTypeNames[tk.type]);
                                tokenCount++;
                            }
                        }
                        fclose(fp2);
                        printf("\nTotal tokens: %d, Lexical errors: %d\n", tokenCount, errorCount);
                    }
                }
                break;

            case 3:
                printf("\n========== Option 3: Parsing and Syntax Verification ==========\n");
                {
                    printf("\nLoading grammar...\n");
                    Grammar G = loadGrammar("grammar.txt");

                    if (G.numRules == 0) {
                        fprintf(stderr, "Error: Grammar not loaded properly\n");
                        break;
                    }

                    printf("Computing FIRST and FOLLOW sets...\n");
                    FirstAndFollow F = computeFirstAndFollowSets(&G);

                    printf("Building predictive parsing table...\n");
                    ParseTable T = createParseTable(&G, &F);

                    printf("\nParsing source code: %s\n", testcaseFile);
                    ParseTree *PT = parseInputSourceCode(testcaseFile, &G, &T, &F);

                    printf("\nPrinting parse tree to: %s\n", parseTreeFile);
                    printParseTree(PT, parseTreeFile);

                    printf("\nParse tree successfully generated!\n");
                }
                break;

            case 4:
                printf("\n========== Option 4: Execution Time ==========\n");
                {
                    clock_t start_time, end_time;
                    double total_CPU_time, total_CPU_time_in_seconds;

                    printf("\nMeasuring execution time for lexer and parser...\n\n");

                    start_time = clock();

                    // Load grammar
                    Grammar G = loadGrammar("grammar.txt");

                    // Lexical analysis
                    State s = initializeState(testcaseFile);
                    TokenList tokens = scan(&s);

                    // Syntax analysis
                    FirstAndFollow F = computeFirstAndFollowSets(&G);
                    ParseTable T = createParseTable(&G, &F);
                    ParseTree *PT = parseInputSourceCode(testcaseFile, &G, &T, &F);

                    end_time = clock();

                    total_CPU_time = (double)(end_time - start_time);
                    total_CPU_time_in_seconds = total_CPU_time / CLOCKS_PER_SEC;

                    printf("═══════════════════════════════════════════════════\n");
                    printf("Execution Time Report:\n");
                    printf("─────────────────────────────────────────────────────\n");
                    printf("Total CPU Time (clock ticks): %.0f\n", total_CPU_time);
                    printf("Total CPU Time (seconds):     %.6f seconds\n", total_CPU_time_in_seconds);
                    printf("Total CPU Time (millisec):    %.3f ms\n", total_CPU_time_in_seconds * 1000);
                    printf("═══════════════════════════════════════════════════\n");
                    printf("\nTokens processed: %d\n", tokens.size);
                }
                break;

            default:
                fprintf(stderr, "Invalid choice. Please enter a number between 0 and 4.\n");
        }
        printf("\n");
    }

    return 0;
}

void displayStatus() {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║           COMPILER CONSTRUCTION - STAGE 1              ║\n");
    printf("║     Lexical Analyzer and Syntax Analyzer (Parser)      ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\nImplementation Status:\n");
    printf("  ✓ (a) FIRST and FOLLOW set automated (Fixed-point iteration)\n");
    printf("  ✓ (b) Lexical analyzer module fully developed\n");
    printf("  ✓ (c) Both lexical and syntax analysis modules implemented\n");
    printf("  ✓ (d) LL(1) Predictive Parser with error recovery\n");
    printf("  ✓ (e) Parse tree construction and inorder traversal\n");
    printf("  ✓ (f) Syntax error reporting with line numbers\n\n");
}

void showMenu() {
    printf("╔════════════════════════════════════════════╗\n");
    printf("║            MENU OPTIONS                    ║\n");
    printf("╠════════════════════════════════════════════╣\n");
    printf("║  0 : Exit                                  ║\n");
    printf("║  1 : Remove comments (print on console)    ║\n");
    printf("║  2 : Print token list (lexical analysis)   ║\n");
    printf("║  3 : Parse code (syntax verification)      ║\n");
    printf("║  4 : Print execution time                  ║\n");
    printf("╚════════════════════════════════════════════╝\n");
}
