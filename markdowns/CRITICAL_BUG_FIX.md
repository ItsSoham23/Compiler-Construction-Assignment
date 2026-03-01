# CRITICAL BUG FIX: Symbol Encoding Collision

## The Bug

The parser was **never expanding any nonterminals**, which is why the parse tree only contained the root node.

### Root Cause: Encoding Collision

The problem was in how symbols were encoded in production rules:

```c
#define EPSILON -1

static int encodeNonTerminal(int nt) {
    return -(nt + 1);  // ❌ WRONG
}

static int decodeNonTerminal(int symbol) {
    return -(symbol + 1);  // ❌ WRONG
}

static int isNonTerminal(int symbol) {
    return symbol < -1;  // This only accepts -2, -3, -4, ...
}
```

**The Collision:**
- `NT_PROGRAM` has enum value `0`
- When encoded: `-(0 + 1) = -1`
- But `EPSILON = -1`
- So `NT_PROGRAM` was encoded as `-1`, which is also `EPSILON`!

**The Result:**
- When the parser encountered `NT_PROGRAM` encoded as `-1`
- `isNonTerminal(-1)` returned **false** (because `< -1` check failed)
- The parser treated `NT_PROGRAM` as a terminal, not a nonterminal
- The parser never expanded `NT_PROGRAM` with any production rule
- The parse tree remained as just the root node with no children

## The Fix

Changed the nonterminal encoding to use `-(nt + 2)` instead of `-(nt + 1)`:

```c
static int encodeNonTerminal(int nt) {
    return -(nt + 2);  // ✓ CORRECT
}

static int decodeNonTerminal(int symbol) {
    return -(symbol + 2);  // ✓ CORRECT
}

static int isNonTerminal(int symbol) {
    return symbol < -1;  // Still works: -2, -3, -4, ...
}
```

**New Encoding:**
- `EPSILON = -1` (unchanged)
- `NT_PROGRAM (0)` → `-(0 + 2) = -2` ✓
- `NT_MAINFUNCTION (1)` → `-(1 + 2) = -3` ✓
- `NT_OTHERFUNCTIONS (2)` → `-(2 + 2) = -4` ✓
- All nonterminals now encode to `-2`, `-3`, `-4`, ... (all `< -1`)
- `isNonTerminal()` correctly identifies them

## Files Modified

1. **parser.c: Line 283** - Changed `decodeNonTerminal()` calculation
2. **parser.c: Line 287** - Changed `encodeNonTerminal()` calculation
3. **parser.c: Line 260** - Changed grammar loading encoding from `-(int)nt - 1` to `-(int)nt - 2`

## Expected Result

After recompiling and testing:
- The parse tree should now properly expand the root `program` nonterminal
- Child nodes (like `otherFunctions`, `mainFunction`) should be created
- Further expansions should continue recursively
- The `output_t3.txt` and `output_t4.txt` files should now contain full parse trees instead of just one line

## Recompilation

On your Ubuntu/Linux terminal:

```bash
cd "/c/Nishant/Work/BITS - Engineering/3-2/CoCo/Code/Compiler-Construction-Assignment"
make clean
make
```

Then test:

```bash
./stage1exe t3.txt output_t3.txt
```

Select **option 3** to parse, then check:

```bash
cat output_t3.txt
```

You should now see a proper parse tree with multiple lines!
