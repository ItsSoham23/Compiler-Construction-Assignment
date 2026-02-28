#include "lexer.h"
#include <ctype.h>

/* ---------------- Public API state ---------------- */

static State g_apiState;
static int g_apiReady = 0;
static int g_apiTokenIndex = 0;
static int g_streamingScanMode = 0;

#define TWIN_BUFFER_CHUNK 50
#define TWIN_PUSHBACK_MAX 50

typedef struct TwinBufferContext {
    FILE *file;
    char block[2][TWIN_BUFFER_CHUNK];
    size_t fill[2];
    int activeBlock;
    size_t cursor;
    int initialized;
    int pushback[TWIN_PUSHBACK_MAX];
    int pushbackTop;
} TwinBufferContext;

static TwinBufferContext g_twin = {0};

static void resetTwinBuffer(void){
    g_twin.file = NULL;
    g_twin.fill[0] = g_twin.fill[1] = 0;
    g_twin.activeBlock = 0;
    g_twin.cursor = 0;
    g_twin.initialized = 0;
    g_twin.pushbackTop = 0;
}

static void initTwinBuffer(FILE *fp){
    resetTwinBuffer();
    g_twin.file = fp;
    g_twin.fill[0] = fread(g_twin.block[0], 1, TWIN_BUFFER_CHUNK, fp);
    g_twin.fill[1] = 0;
    g_twin.activeBlock = 0;
    g_twin.cursor = 0;
    g_twin.initialized = 1;
}

static void clearTokenErrors(TokenList *tl){
    for(int i = 0; i < tl->size; i++){
        if(tl->buf[i].errMsg){
            free(tl->buf[i].errMsg);
            tl->buf[i].errMsg = NULL;
        }
    }
    tl->size = 0;
}

/* Forward declarations for internal functions used before definition */
Token newToken(TokenType type, State* s);
TokenList newTokenList(int initialCapacity);
void appendToTokenList(Token c, TokenList* t);
Hashmap initializeKeywordMap();
TokenList scan(State *s);

static void resetApiState(void){
    if(!g_apiReady) return;
    clearTokenErrors(&g_apiState.tokenList);
    free(g_apiState.tokenList.buf);
    g_apiState.tokenList.buf = NULL;
    g_apiState.tokenList.size = 0;
    g_apiState.tokenList.capacity = 0;
    g_apiReady = 0;
    g_apiTokenIndex = 0;
    g_streamingScanMode = 0;
    resetTwinBuffer();
}

/* ---------------- Utility functions ---------------- */

static void appendErrorToTokenList(State *s, const char *msg){
    Token t = newToken(TK_ERROR, s);
    size_t need = strlen(msg) + 1;
    t.errMsg = malloc(need);
    if(t.errMsg) memcpy(t.errMsg, msg, need);
    appendToTokenList(t, &s->tokenList);
}

void printLexerError(const char *msg, State* s){
    appendErrorToTokenList(s, msg);
}

static void printUnknownSymbol(State *s, const char *symbol){
    char buf[128];
    snprintf(buf, sizeof(buf), "Unknown Symbol %s", symbol);
    appendErrorToTokenList(s, buf);
}

int isSmallAlpha(char c){ return c >= 'a' && c <= 'z'; }
int isAlpha(char c){ return isSmallAlpha(c) || (c >= 'A' && c <= 'Z'); }
int isNum(char c){ return c >= '0' && c <= '9'; }
int isAlphaNum(char c){ return isAlpha(c) || isNum(c); }

static int readChar(State *s){
    if(!g_twin.initialized || g_twin.file != s->file)
        initTwinBuffer(s->file);

    if(g_twin.pushbackTop > 0){
        int c = g_twin.pushback[--g_twin.pushbackTop];
        if(c == '\n') s->line++;
        s->isAtEnd = 0;
        return c;
    }

    while(g_twin.cursor >= g_twin.fill[g_twin.activeBlock]){
        int next = 1 - g_twin.activeBlock;
        g_twin.fill[next] = fread(g_twin.block[next], 1, TWIN_BUFFER_CHUNK, s->file);
        g_twin.activeBlock = next;
        g_twin.cursor = 0;
        if(g_twin.fill[g_twin.activeBlock] == 0){
            s->isAtEnd = 1;
            return EOF;
        }
    }

    int c = (unsigned char)g_twin.block[g_twin.activeBlock][g_twin.cursor++];
    if(c == '\n')
        s->line++;
    s->isAtEnd = 0;
    return c;
}

static void unreadChar(State *s, int c){
    if(c == EOF) return;
    if(g_twin.pushbackTop < TWIN_PUSHBACK_MAX)
        g_twin.pushback[g_twin.pushbackTop++] = c;
    if(c == '\n' && s->line > 1)
        s->line--;
    s->isAtEnd = 0;
}

static int peekChar(State *s){
    int c = readChar(s);
    if(c != EOF)
        unreadChar(s, c);
    return c;
}

static void emitToken(State *s, TokenType type, const char *lexeme, size_t len){
    Token token = newToken(type, s);
    size_t copy = len;
    if(copy >= MAX_LEXEME_LEN)
        copy = MAX_LEXEME_LEN - 1;
    memcpy(token.lexeme, lexeme, copy);
    token.lexeme[copy] = '\0';
    token.lexemeSize = copy;
    appendToTokenList(token, &s->tokenList);
}

static int isFieldId(const char *lexeme){
    if(!lexeme || !lexeme[0]) return 0;
    for(int i = 0; lexeme[i]; i++){
        if(!islower((unsigned char)lexeme[i]))
            return 0;
    }
    return 1;
}

/* ---------------- Token handling ---------------- */

Token newToken(TokenType type, State* s){
    Token t;
    t.type = type;
    t.lexeme[0] = '\0';
    t.lexemeSize = 0;
    t.lineNo = s->line;
    t.errMsg = NULL;
    return t;
}

TokenList newTokenList(int initialCapacity){
    TokenList tl;
    tl.buf = malloc(sizeof(Token) * initialCapacity);
    tl.capacity = initialCapacity;
    tl.size = 0;
    return tl;
}

void appendToTokenList(Token c, TokenList* t){
    if(t->size == t->capacity){
        t->capacity *= 2;
        t->buf = realloc(t->buf, sizeof(Token) * t->capacity);
    }
    t->buf[t->size++] = c;
}

/* ---------------- Keyword map ---------------- */

unsigned int hash(const char *s){
    unsigned int h = 0;
    while(*s) h = h * 31 + (unsigned char)(*s++);
    return h % HASH_SIZE;
}

void insertInHashmap(Hashmap *h, const char *key, TokenType token){
    unsigned int idx = hash(key);
    while(h->table[idx].occupied)
        idx = (idx + 1) % HASH_SIZE;
    h->table[idx].key = key;
    h->table[idx].token = token;
    h->table[idx].occupied = 1;
}

Hashmap initializeKeywordMap(){
    Hashmap m = {0};
    insertInHashmap(&m,"with",TK_WITH);
    insertInHashmap(&m,"parameters",TK_PARAMETERS);
    insertInHashmap(&m,"end",TK_END);
    insertInHashmap(&m,"while",TK_WHILE);
    insertInHashmap(&m,"union",TK_UNION);
    insertInHashmap(&m,"endunion",TK_ENDUNION);
    insertInHashmap(&m,"definetype",TK_DEFINETYPE);
    insertInHashmap(&m,"as",TK_AS);
    insertInHashmap(&m,"type",TK_TYPE);
    insertInHashmap(&m,"global",TK_GLOBAL);
    insertInHashmap(&m,"parameter",TK_PARAMETER);
    insertInHashmap(&m,"list",TK_LIST);
    insertInHashmap(&m,"input",TK_INPUT);
    insertInHashmap(&m,"output",TK_OUTPUT);
    insertInHashmap(&m,"int",TK_INT);
    insertInHashmap(&m,"real",TK_REAL);
    insertInHashmap(&m,"endwhile",TK_ENDWHILE);
    insertInHashmap(&m,"if",TK_IF);
    insertInHashmap(&m,"then",TK_THEN);
    insertInHashmap(&m,"endif",TK_ENDIF);
    insertInHashmap(&m,"read",TK_READ);
    insertInHashmap(&m,"write",TK_WRITE);
    insertInHashmap(&m,"return",TK_RETURN);
    insertInHashmap(&m,"call",TK_CALL);
    insertInHashmap(&m,"record",TK_RECORD);
    insertInHashmap(&m,"endrecord",TK_ENDRECORD);
    insertInHashmap(&m,"else",TK_ELSE);
    return m;
}

TokenType lookupKeyword(Hashmap *h, const char *key){
    unsigned int idx = hash(key);
    while(h->table[idx].occupied){
        if(strcmp(h->table[idx].key,key)==0)
            return h->table[idx].token;
        idx = (idx + 1) % HASH_SIZE;
    }
    return TK_ERROR;
}

/* ---------------- Lexer ---------------- */

TokenList scan(State *s){
    int c;

    while((c = readChar(s)) != EOF){
        int startSize = s->tokenList.size;
        if(c == '\n') continue;
        if(c == ' ' || c == '\t' || c == '\r') continue;

        if(c == '%'){
            emitToken(s, TK_COMMENT, "%", 1);
            while((c = readChar(s)) != EOF && c != '\n');
            if(g_streamingScanMode && s->tokenList.size > startSize) return s->tokenList;
            if(g_streamingScanMode && s->tokenList.size > startSize) return s->tokenList;
            continue;
        }

        if(isAlpha(c)){
            char lexeme[MAX_LEXEME_LEN];
            int len = 0;

            lexeme[len++] = (char)c;
            while(len < MAX_LEXEME_LEN - 1){
                int next = peekChar(s);
                if(next == EOF) break;
                if(isAlpha(next) || isNum(next)){
                    lexeme[len++] = (char)readChar(s);
                    continue;
                }
                break;
            }
            lexeme[len] = '\0';
            /* If identifier is longer than 20 characters, report error and
               skip the rest of the alphanumeric sequence immediately.
               Exception: keep splittable mixed lexemes (letters+digits+letters...)
               for downstream splitting (e.g. endrecorb2c3endrecord). */
            if(len > 20){
                int firstDigitPos = -1;
                int hasAlphaAfterDigit = 0;
                for(int i = 0; i < len; i++){
                    if(isNum((unsigned char)lexeme[i])){
                        firstDigitPos = i;
                        break;
                    }
                }
                if(firstDigitPos >= 2){
                    for(int j = firstDigitPos + 1; j < len; j++){
                        if(isAlpha((unsigned char)lexeme[j])){
                            hasAlphaAfterDigit = 1;
                            break;
                        }
                    }
                }
                if(!(firstDigitPos >= 2 && hasAlphaAfterDigit)){
                    appendErrorToTokenList(s, "Variable Identifier is longer than the prescribed length of 20 characters.");
                    while(isAlphaNum(peekChar(s))) readChar(s);
                    continue;
                }
            }
            /* Iteratively split mixed letter/digit lexemes into sensible tokens.
               Rules:
               - Parse runs of letters then optional run of digits, left-to-right.
               - If a letters run is all-lowercase and length>1 and immediately
                 followed by a digit run that continues until a non-digit, emit
                 TK_FIELDID(letters) then TK_NUM(digits).
               - If a letters run has length==1 and is followed by digits, emit
                 TK_ID for letters+digits (e.g. c3, d4).
               - Otherwise, treat the letters run as keyword/ID/fieldid by
                 lookup: keywords first, then TK_FIELDID (all-lowercase), else TK_ID.
               - If any emitted identifier (letters run) exceeds 20 chars, emit
                 the long-identifier error and skip remaining alnums.
            */
            /* If lexeme has the form: single-letter + digits + letters... then
               prefer to treat the whole lexeme as an identifier (TK_ID).
               This prevents splitting names like 'd5cb34567' into d5/cb/34567.
            */
            int firstDigit = -1;
            for(int i=0;i<len;i++) if(isNum((unsigned char)lexeme[i])){ firstDigit = i; break; }
            if(firstDigit == 1){
                int hasAlphaAfter = 0;
                for(int j = firstDigit; j < len; j++) if(isAlpha((unsigned char)lexeme[j])){ hasAlphaAfter = 1; break; }
                if(hasAlphaAfter){
                    emitToken(s, TK_ID, lexeme, len);
                    continue;
                }
            }

            int pos = 0;
            while(pos < len){
                /* collect letters run */
                int lstart = pos;
                while(pos < len && isAlpha((unsigned char)lexeme[pos])) pos++;
                int llen = pos - lstart;
                if(llen <= 0){
                    /* shouldn't happen — fall back to emitting the remainder as ID */
                    emitToken(s, TK_ID, lexeme + pos, len - pos);
                    break;
                }

                /* collect digit run immediately after letters */
                int dstart = pos;
                while(pos < len && isNum((unsigned char)lexeme[pos])) pos++;
                int dlen = pos - dstart;

                if(llen > 20){
                    appendErrorToTokenList(s, "Variable Identifier is longer than the prescribed length of 20 characters.");
                    /* skip remaining alnum characters */
                    while(isAlphaNum(peekChar(s))) readChar(s);
                    break;
                }

                if(dlen > 0){
                    /* letters followed by digits */
                    /* build substrings */
                    char letters[MAX_LEXEME_LEN];
                    memcpy(letters, lexeme + lstart, llen); letters[llen] = '\0';
                    if(llen == 1){
                        /* emit single-letter + digits as TK_ID */
                        char combined[MAX_LEXEME_LEN];
                        int clen = llen + dlen;
                        memcpy(combined, lexeme + lstart, clen); combined[clen] = '\0';
                        emitToken(s, TK_ID, combined, clen);
                        continue;
                    }
                    /* llen > 1 */
                    if(isFieldId(letters)){
                        emitToken(s, TK_FIELDID, letters, llen);
                        char digits[MAX_LEXEME_LEN];
                        memcpy(digits, lexeme + dstart, dlen); digits[dlen] = '\0';
                        emitToken(s, TK_NUM, digits, dlen);
                        continue;
                    } else {
                        /* not a pure-fieldid prefix; emit whole letters+digits as ID */
                        char combined[MAX_LEXEME_LEN];
                        int clen = llen + dlen;
                        memcpy(combined, lexeme + lstart, clen); combined[clen] = '\0';
                        emitToken(s, TK_ID, combined, clen);
                        continue;
                    }
                } else {
                    /* letters not followed by digits: treat as keyword/fieldid/ID */
                    char letters[MAX_LEXEME_LEN];
                    memcpy(letters, lexeme + lstart, llen); letters[llen] = '\0';
                    char lower[MAX_LEXEME_LEN];
                    for(int i = 0; i < llen; i++) lower[i] = (char)tolower((unsigned char)letters[i]);
                    lower[llen] = '\0';
                    TokenType type = lookupKeyword(&s->keywordMap, lower);
                    if(type == TK_ERROR){
                        if(isFieldId(letters)) type = TK_FIELDID; else type = TK_ID;
                    }
                    emitToken(s, type, letters, llen);
                    continue;
                }
            }
            continue;
        }

        if(c == '_'){
            char lexeme[MAX_LEXEME_LEN];
            int len = 0;
            lexeme[len++] = '_';
            if(peekChar(s) == EOF || !isAlpha(peekChar(s))){
                printLexerError("malformed function identifier", s);
                continue;
            }
            while(len < MAX_LEXEME_LEN - 1 && isAlpha(peekChar(s)))
                lexeme[len++] = (char)readChar(s);
            while(len < MAX_LEXEME_LEN - 1 && isNum(peekChar(s)))
                lexeme[len++] = (char)readChar(s);
            lexeme[len] = '\0';
            TokenType type = TK_FUNID;
            if(strcmp(lexeme, "_main") == 0)
                type = TK_MAIN;
            emitToken(s, type, lexeme, len);
            if(g_streamingScanMode && s->tokenList.size > startSize) return s->tokenList;
            continue;
        }

        if(c == '#'){
            char lexeme[MAX_LEXEME_LEN];
            int len = 0;
            lexeme[len++] = '#';
            while(len < MAX_LEXEME_LEN - 1 && isAlpha(peekChar(s)))
                lexeme[len++] = (char)readChar(s);
            if(len > 1)
                emitToken(s, TK_RUID, lexeme, len);
            else
                printLexerError("malformed reference identifier", s);
            if(g_streamingScanMode && s->tokenList.size > startSize) return s->tokenList;
            continue;
        }

        if(isNum(c)){
            char lexeme[MAX_LEXEME_LEN];
            int len = 0;
            int isReal = 0;
            int malformedReal = 0;
            lexeme[len++] = (char)c;
            while(len < MAX_LEXEME_LEN - 1){
                int next = peekChar(s);
                if(next == EOF) break;

                if(next == '.' && !isReal){
                    /* consume '.' and read fractional digits */
                    lexeme[len++] = (char)readChar(s);
                    int fracDigits = 0;
                    while(len < MAX_LEXEME_LEN - 1){
                        int pk = peekChar(s);
                        if(pk >= '0' && pk <= '9'){
                            lexeme[len++] = (char)readChar(s);
                            fracDigits++;
                            continue;
                        }
                        break;
                    }
                    if(fracDigits == 0){
                        /* treat trailing dot with no fractional digits as Unknown pattern */
                        lexeme[len] = '\0';
                        char buf[128];
                        snprintf(buf, sizeof(buf), "Unknown pattern <%s>", lexeme);
                        appendErrorToTokenList(s, buf);
                        malformedReal = 1;
                        break;
                    }
                    /* If fractional part has exactly one digit, treat as Unknown pattern */
                    if(fracDigits == 1){
                        lexeme[len] = '\0';
                        char buf[128];
                        snprintf(buf, sizeof(buf), "Unknown pattern <%s>", lexeme);
                        appendErrorToTokenList(s, buf);
                        /* do not consume any further characters for this token */
                        break;
                    }
                    /* if another '.' follows, treat the prefix (e.g. 123.5) as unknown pattern */
                    if(peekChar(s) == '.'){
                        lexeme[len] = '\0';
                        char buf[128];
                        snprintf(buf, sizeof(buf), "Unknown pattern <%s>", lexeme);
                        appendErrorToTokenList(s, buf);
                        /* do not consume the next '.' - let main loop handle it */
                        break;
                    }
                    isReal = 1;
                    continue;
                }

                if(next == 'E' || next == 'e'){
                    isReal = 1;
                    lexeme[len++] = (char)readChar(s);
                    int sign = peekChar(s);
                    if(sign == '+' || sign == '-')
                        lexeme[len++] = (char)readChar(s);
                    int digitsRead = 0;
                    /* Read at most two exponent digits into the lexeme; leave any
                       further digits unread so they form subsequent tokens. */
                    while(len < MAX_LEXEME_LEN - 1){
                        int digit = peekChar(s);
                        if(digit >= '0' && digit <= '9'){
                            if(digitsRead < 2){
                                lexeme[len++] = (char)readChar(s);
                                digitsRead++;
                                continue;
                            }
                            /* stop reading more digits here; leave them for next token */
                            break;
                        }
                        break;
                    }
                    if(digitsRead == 0)
                        printLexerError("malformed exponent", s);
                    /* after reading exponent digits, stop numeric scanning so
                       any extra digits remain for the next token */
                    break;
                }

                if(isNum(next)){
                    lexeme[len++] = (char)readChar(s);
                    continue;
                }
                break;
            }
            /* ensure null-termination */
            if(len >= MAX_LEXEME_LEN) len = MAX_LEXEME_LEN - 1;
            lexeme[len] = '\0';
            /* if the last appended token was an Unknown pattern error, skip emitting a numeric token */
            int emit = 1;
            if(s->tokenList.size > 0){
                Token *last = &s->tokenList.buf[s->tokenList.size-1];
                if(last->type == TK_ERROR && last->errMsg && strstr(last->errMsg, "Unknown pattern <") == last->errMsg){
                    emit = 0;
                }
            }
            if(malformedReal) emit = 0;
            if(emit)
                emitToken(s, isReal ? TK_RNUM : TK_NUM, lexeme, len);
            if(g_streamingScanMode && s->tokenList.size > startSize) return s->tokenList;
            continue;
        }

        switch(c){
            case '!': {
                if(peekChar(s) == '='){
                    readChar(s);
                    emitToken(s, TK_NE, "!=", 2);
                } else {
                    printLexerError("unrecognized symbol", s);
                }
                break;
            }
            case '+': emitToken(s, TK_PLUS, "+", 1); break;
            case '-': emitToken(s, TK_MINUS, "-", 1); break;
            case '*': emitToken(s, TK_MUL, "*", 1); break;
            case '/': emitToken(s, TK_DIV, "/", 1); break;
            case ',': emitToken(s, TK_COMMA, ",", 1); break;
            case ';': emitToken(s, TK_SEM, ";", 1); break;
            case ':': emitToken(s, TK_COLON, ":", 1); break;
            case '.': emitToken(s, TK_DOT, ".", 1); break;
            case '(': emitToken(s, TK_OP, "(", 1); break;
            case ')': emitToken(s, TK_CL, ")", 1); break;
            case '[': emitToken(s, TK_SQL, "[", 1); break;
            case ']': emitToken(s, TK_SQR, "]", 1); break;
            case '~': emitToken(s, TK_NOT, "~", 1); break;
            case '<': {
                int dashCount = 0;
                int success = 1;
                for(int i = 0; i < 3; i++){
                    int dash = readChar(s);
                    if(dash != '-'){
                        if(dash != EOF)
                            unreadChar(s, dash);
                        success = 0;
                        break;
                    }
                    dashCount++;
                }
                if(success && dashCount == 3){
                    emitToken(s, TK_ASSIGNOP, "<---", 4);
                    break;
                }
                for(int i = 0; i < dashCount; i++)
                    unreadChar(s, '-');
                if(peekChar(s) == '='){
                    readChar(s);
                    if(peekChar(s) == '>'){
                        readChar(s);
                        printUnknownSymbol(s, "<=>");
                        break;
                    }
                    emitToken(s, TK_LE, "<=", 2);
                    break;
                }
                emitToken(s, TK_LT, "<", 1);
                break;
            }
            case '>': {
                if(peekChar(s) == '='){
                    readChar(s);
                    emitToken(s, TK_GE, ">=", 2);
                    break;
                }
                emitToken(s, TK_GT, ">", 1);
                break;
            }
            case '=': {
                int next = peekChar(s);
                if(next == '='){
                    readChar(s);
                    emitToken(s, TK_EQ, "==", 2);
                } else if(next == '<'){
                    printUnknownSymbol(s, "<=>");
                } else {
                    printUnknownSymbol(s, "=");
                }
                break;
            }
            case '&': {
                char lexeme[MAX_LEXEME_LEN];
                int len = 0;
                lexeme[len++] = '&';
                while(len < MAX_LEXEME_LEN - 1 && peekChar(s) == '&')
                    lexeme[len++] = (char)readChar(s);
                    if(len == 3)
                        emitToken(s, TK_AND, lexeme, 3);
                    else
                        printLexerError("Unknown pattern <&&>", s);
                break;
            }
                case '|': {
                    /* Each '|' is an unknown symbol in this language (only @@@ is TK_OR) */
                    printUnknownSymbol(s, "<|>");
                    break;
                }
                case '@': {
                    char lexeme[MAX_LEXEME_LEN];
                    int len = 0;
                    char symbol = (char)c;
                    lexeme[len++] = symbol;
                    while(len < MAX_LEXEME_LEN - 1 && peekChar(s) == symbol)
                        lexeme[len++] = (char)readChar(s);
                    if(len == 3)
                        emitToken(s, TK_OR, lexeme, 3);
                    else
                        printLexerError("Unknown pattern <@@@>", s);
                    break;
                }
            case '$':
                printUnknownSymbol(s, "<$>");
                break;
            default:
                printLexerError("unrecognized symbol", s);
        }

        if(g_streamingScanMode && s->tokenList.size > startSize) return s->tokenList;
    }

    if(!g_streamingScanMode){
        fclose(s->file);
        resetTwinBuffer();
    }
    s->isAtEnd = 1;
    return s->tokenList;
}

/* ---------------- Public API ---------------- */

void removeComments(char *testcaseFile, char *cleanFile){
    if(!testcaseFile) return;
    FILE *in = fopen(testcaseFile, "r");
    if(!in) return;
    FILE *out = cleanFile ? fopen(cleanFile, "w") : stdout;
    if(!out){
        fclose(in);
        return;
    }

    int c;
    int inComment = 0;
    int lineHasContent = 0;
    int pendingSpaces = 0;
    while((c = fgetc(in)) != EOF){
        if(!inComment && c == '%'){
            inComment = 1;
            continue;
        }
        if(inComment){
            if(c == '\n'){
                if(lineHasContent)
                    fputc('\n', out);
                inComment = 0;
                lineHasContent = 0;
                pendingSpaces = 0;
            }
            continue;
        }

        if(c == '\r')
            continue;

        if(c == '\n'){
            if(lineHasContent)
                fputc('\n', out);
            lineHasContent = 0;
            pendingSpaces = 0;
            continue;
        }

        if(c == ' ' || c == '\t'){
            pendingSpaces++;
            continue;
        }

        while(pendingSpaces-- > 0)
            fputc(' ', out);
        pendingSpaces = 0;
        fputc(c, out);
        lineHasContent = 1;
    }

    fclose(in);
    if(out != stdout) fclose(out);
}

/* ---------------- Required lexer API ---------------- */

FILE *getStream(FILE *fp){
    if(!fp) return NULL;

    resetApiState();
    resetTwinBuffer();

    g_apiState.file = fp;
    g_apiState.line = 1;
    g_apiState.isAtEnd = 0;
    g_apiState.scanNext = 1;
    g_apiState.tokenList = newTokenList(16);
    g_apiState.keywordMap = initializeKeywordMap();
    g_apiReady = 1;
    g_apiTokenIndex = 0;

    return fp;
}

tokenInfo getNextToken(twinBuffer B){
    State fallbackState;
    fallbackState.line = 1;

    if(!g_apiReady && B != NULL)
        getStream(B);

    if(!g_apiReady){
        tokenInfo t = newToken(TK_ERROR, &fallbackState);
        const char *msg = "Scanner not initialized";
        t.errMsg = malloc(strlen(msg) + 1);
        if(t.errMsg) strcpy(t.errMsg, msg);
        return t;
    }

    if(g_apiTokenIndex >= g_apiState.tokenList.size){
        clearTokenErrors(&g_apiState.tokenList);
        g_apiTokenIndex = 0;
        g_streamingScanMode = 1;
        scan(&g_apiState);
        g_streamingScanMode = 0;
    }

    if(g_apiTokenIndex >= g_apiState.tokenList.size){
        tokenInfo t = newToken(TK_ERROR, &g_apiState);
        const char *msg = "EOF";
        t.errMsg = malloc(strlen(msg) + 1);
        if(t.errMsg) strcpy(t.errMsg, msg);
        return t;
    }

    return g_apiState.tokenList.buf[g_apiTokenIndex++];
}