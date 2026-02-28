# Parse Tree Construction Bug Fix

## Issue Found
The parse tree was not being constructed properly. Only the root node was appearing in `output_t3.txt`.

## Root Cause
In the original `parseInputSourceCode()` function:
- When a nonterminal was expanded, the code pushed the **parent node** onto the nodeStack for each RHS symbol
- **Child nodes were never created** - only the parent was reused
- Terminals were matched against the parent node instead of having their own nodes

This caused the parse tree to remain as just a single root node with no children.

## Solution Applied
Fixed the parsing logic in `parser.c` (lines 601-684):

1. **Create child nodes**: For each RHS symbol, create a proper `ParseTreeNode` with the correct symbol type (TERMINAL or NONTERMINAL)
2. **Link to parent**: Add all child nodes to the parent's `children` array
3. **Push child nodes**: Push each child node onto the nodeStack so they can be further processed or matched with input tokens
4. **Terminal matching**: When a terminal matches, store the token information in the correct child node

## Changes Made
```c
// OLD (BROKEN):
for (int i = prod.rhsCount - 1; i >= 0; i--) {
    if (prod.rhs[i] != EPSILON) {
        stack[stackPtr] = prod.rhs[i];
        nodeStack[nodeStackPtr] = topNode;  // ❌ WRONG: Reusing parent!
        stackPtr++;
        nodeStackPtr++;
    }
}

// NEW (FIXED):
// Create child nodes for RHS
ParseTreeNode* childNodes[MAX_TERMINALS];
int childCount = 0;
for (int i = 0; i < prod.rhsCount; i++) {
    if (prod.rhs[i] != EPSILON) {
        // Create actual child node ✓
        ParseTreeNode* childNode = newParseTreeNode(childSymbol, topNode);
        childNodes[childCount++] = childNode;
    }
}

// Add child nodes to parent
topNode->children = (ParseTreeNode**)malloc(...);
topNode->childCount = childCount;

// Push child nodes to nodeStack ✓
for (int i = prod.rhsCount - 1, childIdx = childCount - 1; i >= 0; i--) {
    if (prod.rhs[i] != EPSILON) {
        stack[stackPtr] = prod.rhs[i];
        nodeStack[nodeStackPtr] = childNodes[childIdx];  // ✓ CORRECT: Child node!
        ...
    }
}
```

## How to Recompile

On your Ubuntu/Linux terminal:

```bash
cd "/c/Nishant/Work/BITS - Engineering/3-2/CoCo/Code/Compiler-Construction-Assignment"
make clean
make
```

This will recompile with the fixed parser.c.

## Test Again

After recompilation:

```bash
./stage1exe t3.txt output_t3.txt
```

Then select option 3 to parse, and check the output:

```bash
cat output_t3.txt
```

You should now see a full parse tree with multiple nodes instead of just the root.

## Expected Output
Instead of:
```
----       0     NONTERMINAL     ----       ROOT            no    program
```

You should now see something like:
```
----       0     NONTERMINAL     ----       ROOT            no    program
----       0     NONTERMINAL     program    no    otherFunctions
----       1     NONTERMINAL     otherFunctions    no    function
_main      1     TERMINAL        1          function    yes    ----
...
```

Let me know when you've recompiled and tested, and share the output!
