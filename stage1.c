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
                removeComments(testcaseFile);
                break;

            case 2:
                printf("\n========== Option 2: Printing Token List ==========\n");
                printf("Tokens from: %s\n", testcaseFile);
                printTokens(testcaseFile);
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
