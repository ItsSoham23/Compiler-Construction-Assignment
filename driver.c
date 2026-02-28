#include "lexer.h"
#include "parser.h"
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: ./stage1exe <sourcefile> <parsetreefile>\n");
        return 0;
    }

    printf("STATUS:\n");
    printf("(c) Both lexical and syntax analysis modules implemented\n\n");

    int choice = -1;
    while (choice != 0) {

        printf("\n0 : Exit\n");
        printf("1 : Remove comments\n");
        printf("2 : Print token list\n");
        printf("3 : Parse source code\n");
        printf("4 : Measure execution time\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            removeComments(argv[1]);
        }
        else if (choice == 2) {
            printTokens(argv[1]);
        }
        else if (choice == 3) {
            parseInputSourceCode(argv[1], argv[2]);
        }
        else if (choice == 4) {
            clock_t start = clock();
            parseInputSourceCode(argv[1], argv[2]);
            clock_t end = clock();
            double ticks = (double)(end - start);
            printf("CPU ticks: %.0f\n", ticks);
            printf("Time in seconds: %lf\n", ticks / CLOCKS_PER_SEC);
        }
    }
    return 0;
}