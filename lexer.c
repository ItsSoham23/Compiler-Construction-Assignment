#include "lexer.h"
#include <ctype.h>

static const char* tokenTypeNames[] = {
    "TK_ASSIGNOP",
    "TK_COMMENT",
    "TK_FIELDID",
    "TK_ID",
    "TK_NUM",
    "TK_RNUM",
    "TK_FUNID",
    "TK_RUID",
    "TK_WITH",
    "TK_PARAMETERS",
    "TK_END",
    "TK_WHILE",
    "TK_UNION",
    "TK_ENDUNION",
    "TK_DEFINETYPE",
    "TK_AS",
    "TK_TYPE",
    "TK_MAIN",
    "TK_GLOBAL",
    "TK_PARAMETER",
    "TK_LIST",
    "TK_SQL",
    "TK_SQR",
    "TK_INPUT",
    "TK_OUTPUT",
    "TK_INT",
    "TK_REAL",
    "TK_COMMA",
    "TK_SEM",
    "TK_COLON",
    "TK_DOT",
    "TK_ENDWHILE",
    "TK_OP",
    "TK_CL",
    "TK_IF",
    "TK_THEN",
    "TK_ENDIF",
    "TK_READ",
    "TK_WRITE",
    "TK_RETURN",
    "TK_PLUS",
    "TK_MINUS",
    "TK_MUL",
    "TK_DIV",
    "TK_CALL",
    "TK_RECORD",
    "TK_ENDRECORD",
    "TK_ELSE",
    "TK_AND",
    "TK_OR",
    "TK_NOT",
    "TK_LT",
    "TK_LE",
    "TK_EQ",
    "TK_GT",
    "TK_GE",
    "TK_NE",
    "TK_ERROR"
};

static const char* tokenTypeToString(TokenType type){
    size_t count = sizeof(tokenTypeNames)/sizeof(tokenTypeNames[0]);
    if(type < 0 || (size_t)type >= count)
        return "TK_UNKNOWN";
    return tokenTypeNames[type];
}

/* ---------------- Utility functions ---------------- */

void printError(const char *msg){
    perror(msg);
}

void printLexerError(const char *msg, State* s){
    printf("[LEXER-ERROR] at line %d: %s\n", s->line, msg);
}

int isSmallAlpha(char c){ return c >= 'a' && c <= 'z'; }
int isAlpha(char c){ return isSmallAlpha(c) || (c >= 'A' && c <= 'Z'); }
int isNum(char c){ return c >= '0' && c <= '9'; }
int isAlphaNum(char c){ return isAlpha(c) || isNum(c); }

int match(char a, char b, const char* msg, State* s){
    if(a == b) return 1;
    printLexerError(msg, s);
    s->scanNext = 0;
    return 0;
}

static int readChar(State *s){
    int c = fgetc(s->file);
    if(c == '\n')
        s->line++;
    if(c == EOF)
        s->isAtEnd = 1;
    return c;
}

static void unreadChar(State *s, int c){
    if(c == EOF) return;
    ungetc(c, s->file);
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

static void warnLongIdentifier(State *s, int len){
    if(len > 20)
        printf("Line %d Error: Variable Identifier is longer than the prescribed length of 20 characters.\n", s->line);
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

/* ---------------- State ---------------- */

State initializeState(const char *fileName){
    FILE* f = fopen(fileName,"r");
    if(!f) printError("File not found");

    State s;
    s.file = f;
    s.line = 1;
    s.isAtEnd = 0;
    s.scanNext = 1;
    s.tokenList = newTokenList(16);
    s.keywordMap = initializeKeywordMap();
    return s;
}

/* ---------------- Lexer ---------------- */

TokenList scan(State *s){
    int c;

    while((c = readChar(s)) != EOF){
        if(c == '\n') continue;
        if(c == ' ' || c == '\t' || c == '\r') continue;

        if(c == '%'){
            emitToken(s, TK_COMMENT, "%", 1);
            while((c = readChar(s)) != EOF && c != '\n');
            continue;
        }

        if(isAlpha(c)){
            char lexeme[MAX_LEXEME_LEN];
            int len = 0;
            int seenDigit = 0;
            int letterAfterDigit = 0;
            int firstDigitIndex = -1;

            lexeme[len++] = (char)c;
            while(len < MAX_LEXEME_LEN - 1){
                int next = peekChar(s);
                if(next == EOF) break;
                if(isAlpha(next) || isNum(next)){
                    char ch = (char)readChar(s);
                    if(isNum(ch)){
                        if(firstDigitIndex < 0)
                            firstDigitIndex = len;
                        seenDigit = 1;
                    } else if(seenDigit){
                        letterAfterDigit = 1;
                    }
                    lexeme[len++] = ch;
                    continue;
                }
                break;
            }
            lexeme[len] = '\0';
            char lower[MAX_LEXEME_LEN];
            for(int i = 0; i < len; i++)
                lower[i] = (char)tolower((unsigned char)lexeme[i]);
            lower[len] = '\0';

            TokenType type = lookupKeyword(&s->keywordMap, lower);
            if(type == TK_ERROR){
                if(isFieldId(lexeme))
                    type = TK_FIELDID;
                else
                    type = TK_ID;
            }

            if(seenDigit && !letterAfterDigit && firstDigitIndex > 0){
                char prefix[MAX_LEXEME_LEN];
                memcpy(prefix, lexeme, firstDigitIndex);
                prefix[firstDigitIndex] = '\0';
                TokenType prefixType = isFieldId(prefix) ? TK_FIELDID : TK_ID;
                warnLongIdentifier(s, firstDigitIndex);
                emitToken(s, prefixType, prefix, firstDigitIndex);
                const char *digits = lexeme + firstDigitIndex;
                emitToken(s, TK_NUM, digits, len - firstDigitIndex);
            } else {
                if(type == TK_ID || type == TK_FIELDID)
                    warnLongIdentifier(s, len);
                emitToken(s, type, lexeme, len);
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
            warnLongIdentifier(s, len);
            emitToken(s, type, lexeme, len);
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
            continue;
        }

        if(isNum(c)){
            char lexeme[MAX_LEXEME_LEN];
            int len = 0;
            int isReal = 0;
            lexeme[len++] = (char)c;
            while(len < MAX_LEXEME_LEN - 1){
                int next = peekChar(s);
                if(next == EOF) break;
                if(next == '.' && !isReal){
                    isReal = 1;
                    lexeme[len++] = (char)readChar(s);
                    continue;
                }
                if(next == 'E' || next == 'e'){
                    isReal = 1;
                    lexeme[len++] = (char)readChar(s);
                    int sign = peekChar(s);
                    if(sign == '+' || sign == '-')
                        lexeme[len++] = (char)readChar(s);
                    int digitsRead = 0;
                    while(len < MAX_LEXEME_LEN - 1){
                        int digit = peekChar(s);
                        if(digit >= '0' && digit <= '9'){
                            lexeme[len++] = (char)readChar(s);
                            digitsRead++;
                            continue;
                        }
                        break;
                    }
                    if(digitsRead == 0)
                        printLexerError("malformed exponent", s);
                    continue;
                }
                if(isNum(next)){
                    lexeme[len++] = (char)readChar(s);
                    continue;
                }
                break;
            }
            emitToken(s, isReal ? TK_RNUM : TK_NUM, lexeme, len);
            continue;
        }

        switch(c){
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
                if(peekChar(s) == '='){
                    readChar(s);
                    emitToken(s, TK_EQ, "==", 2);
                } else {
                    emitToken(s, TK_EQ, "=", 1);
                }
                break;
            }
            case '!':
                if(peekChar(s) == '='){
                    readChar(s);
                    emitToken(s, TK_NE, "!=", 2);
                } else {
                    printLexerError("unrecognized symbol", s);
                }
                break;
            case '&': {
                char lexeme[MAX_LEXEME_LEN];
                int len = 0;
                lexeme[len++] = '&';
                while(len < MAX_LEXEME_LEN - 1 && peekChar(s) == '&')
                    lexeme[len++] = (char)readChar(s);
                if(len == 3)
                    emitToken(s, TK_AND, lexeme, 3);
                else
                    printLexerError("invalid logical and", s);
                break;
            }
            case '|':
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
                    printLexerError("invalid logical or", s);
                break;
            }
            default:
                printLexerError("unrecognized symbol", s);
        }
    }

    fclose(s->file);
    s->isAtEnd = 1;
    return s->tokenList;
}

/* ---------------- Comment removal ---------------- */

void removeComments(const char* filename){
    FILE* f = fopen(filename,"r");
    if(!f) return;

    int c;
    int inComment = 0;
    while((c = fgetc(f)) != EOF){
        if(!inComment && c == '%'){
            inComment = 1;
            continue;
        }
        if(inComment){
            if(c == '\n'){
                putchar('\n');
                inComment = 0;
            }
            continue;
        }
        putchar(c);
    }
    fclose(f);
}

/* ---------------- Driver ---------------- */

void printTokens(const char* filename){
    State s = initializeState(filename);
    TokenList tl = scan(&s);

    for(int i=0;i<tl.size;i++){
        printf("Line %d\tLexeme %s\tToken %s\n",
               tl.buf[i].lineNo,
               tl.buf[i].lexeme,
               tokenTypeToString(tl.buf[i].type));
    }
}

int main(int argc, char** argv){
    if(argc < 2){
        fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
        return 1;
    }

    printTokens(argv[1]);
    printf("\n=== Source without comments ===\n");
    removeComments(argv[1]);
    return 0;
}