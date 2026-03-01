# LL(1) Predictive Parser - Compilation and Testing Guide

## Compilation Instructions

### On Ubuntu/Linux with GCC and make:

```bash
cd /path/to/Compiler-Construction-Assignment
make clean
make
```

This will produce the executable `stage1exe`.

### Manual Compilation (if make is not available):

```bash
gcc -Wall -Wextra -std=c99 -c stage1.c -o stage1.o
gcc -Wall -Wextra -std=c99 -c parser.c -o parser.o
gcc -Wall -Wextra -std=c99 -c lexer.c -o lexer.o
gcc -Wall -Wextra -std=c99 -o stage1exe stage1.o parser.o lexer.o -lm
```

## Execution

### Usage:
```bash
./stage1exe <testcase.txt> <parsetreeOutFile.txt>
```

### Example:
```bash
./stage1exe t1.txt output_tree.txt
```

## Menu Options

Once the program runs, you will see:
1. **Option 0**: Exit program
2. **Option 1**: Remove comments from source code (prints to console)
3. **Option 2**: Print token list (lexical analysis output)
4. **Option 3**: Parse code and verify syntax (generates parse tree in specified output file)
5. **Option 4**: Measure and print execution time for lexer and parser

## Test Files Provided

- `t1.txt`: Test case 1 (699 bytes)
- `t2.txt`: Test case 2 (554 bytes)
- `grammar.txt`: Grammar rules for the language

## Verification Steps

1. **Test Option 1 (Remove Comments)**:
   - Input: Source code with comments
   - Expected Output: Source code without comments printed to console

2. **Test Option 2 (Print Token List)**:
   - Input: Source code file
   - Expected Output: Table with columns: Line No, Token Type, Lexeme, Value

3. **Test Option 3 (Parse Code)**:
   - Input: Source code file + output file name
   - Expected Output:
     - If syntactically correct: "Input source code is syntactically correct"
     - If errors found: List of errors with line numbers and parse tree
   - Output file contains parse tree in inorder traversal format

4. **Test Option 4 (Execution Time)**:
   - Input: Source code file
   - Expected Output: CPU time in clock ticks, seconds, and milliseconds
   - Also prints: Total tokens processed

## Expected Output Format (Option 3)

Parse tree output file (parsetreeOutFile.txt):
```
lexeme      line_no  token_name    value  parent_symbol  is_leaf  node_symbol
-------     ------   ----------    -----  ------         -------  -----------
d2          1        TK_ID         0      singleOrRecId  yes      ----
5           1        TK_NUM        1      factor         yes      ----
...
```

## Implementation Status

The driver displays the following implementation status on startup:
- ✓ (a) FIRST and FOLLOW set automated (Fixed-point iteration)
- ✓ (b) Lexical analyzer module fully developed
- ✓ (c) Both lexical and syntax analysis modules implemented
- ✓ (d) LL(1) Predictive Parser with error recovery
- ✓ (e) Parse tree construction and inorder traversal
- ✓ (f) Syntax error reporting with line numbers

## Files Included

- `stage1.c`: Main driver program with menu interface
- `parser.c`: LL(1) predictive parser implementation (~700 lines)
- `parser.h`: Parser interface definitions
- `parserDef.h`: Data structures for grammar, parsing table, parse tree
- `lexer.c`: Lexical analyzer (pre-existing)
- `lexer.h`: Lexical analyzer interface
- `lexerDef.h`: Lexical analyzer data structures
- `grammar.txt`: Grammar rules in text format
- `makefile`: Build configuration for compilation

## Troubleshooting

If compilation fails:
1. Ensure all `.c` and `.h` files are present in the same directory
2. Verify `gcc` and `make` are installed: `gcc --version` and `make --version`
3. Check that `grammar.txt` is properly formatted
4. Verify file permissions allow reading/writing

If parsing fails:
1. Verify test case file (`t1.txt` or `t2.txt`) contains valid source code
2. Check `grammar.txt` is properly formatted with grammar rules
3. Ensure output file path is writable
4. Check console output for specific error messages
