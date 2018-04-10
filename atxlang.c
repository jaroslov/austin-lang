//usr/bin/clang -std=c11 -DATXLANG_TEST -g "$0" -o atx && exec ./atx "$@"
#ifndef ATXLANG_H
#define ATXLANG_H

#ifdef  ATXLANG_TEST
#define ATXLANG_STDLIB
#endif//ATXLANG_TEST

#ifdef  __cplusplus
extern "C"
{
#endif//__cplusplus

/*

    x ( x : t, x : t, ..., x : t ) : t { s* }
    if ( e ) { s* } else { s* }
    do { } for ( e ) { } while ( e ) { cases* }
    break name
    continue name
    return e

*/

#define ATXLANG_TYPES(X)                  \
    X(INT,          long long int)      \
    X(FLOAT,        double)             \
    X(STRING,       char*)              \
    /* */

typedef enum ATXLANG_TYPE
{
#undef ATXLANG_TYPES_ENTRY
#define ATXLANG_TYPES_ENTRY(X, T) ATXLANG_TYPE_ ## X,
ATXLANG_TYPES(ATXLANG_TYPES_ENTRY)
#undef ATXLANG_TYPES_ENTRY
} ATXLANG_TYPE;

typedef union ATXLangRawU
{
#undef ATXLANG_TYPES_ENTRY
#define ATXLANG_TYPES_ENTRY(X, T) T X;
ATXLANG_TYPES(ATXLANG_TYPES_ENTRY)
#undef ATXLANG_TYPES_ENTRY
} ATXLangRawU;

typedef struct ATXLangRaw
{
    ATXLANG_TYPE    Type;
    ATXLangRawU     U;
} ATXLangRaw;

typedef struct ATXLangCompiler ATXLangCompiler;

typedef struct ATXLangCompilerOptions
{
    int empty;
} ATXLangCompilerOptions;

typedef struct ATXLangModule ATXLangModule;

typedef struct ATXLangThing ATXLangThing;

typedef struct ATXLangClosure
{
    int         numEntries;
    int         curPosition;
    ATXLangRaw  stack[];
} ATXLangClosure;

typedef int (*ATXLangFPtr)(ATXLangClosure*);

ATXLangCompiler* atxlangMakeCompiler(ATXLangCompilerOptions*);
int atxlangFreeCompiler(ATXLangCompiler*);
ATXLangModule* atxlangCompile(ATXLangCompiler*, char*, char*);
int atxlangFreeModule(ATXLangModule*);
int atxlangLinkModule(ATXLangModule*, int numModules, ATXLangModule**);
ATXLangThing* atxlangGet(ATXLangModule*, char* beg, char* end);
int atxlangCall(ATXLangThing*, ATXLangClosure* atxclosure);
int atxlangInjectCFunction(ATXLangModule*, char* beg, char* end, ATXLangFPtr);

#ifdef  ATXLANG_STDLIB
void* atxlang_malloc_context();
void atxlang_malloc_release(void* handle);
void* atxlang_calloc(void* handle, unsigned long long bytes);
int atxlang_free(void* handle, void* p, unsigned long long bytes);
long atxlang_strncmp(char* left, char* right, unsigned long length);
#endif//ATXLANG_STDLIB

#ifndef ATX_DBG_LOG
#define ATX_DBG_LOG(...) fprintf(ATXLOGOUT, __VA_ARGS__)
#endif//ATX_DBG_LOG

#ifndef ATXLOGOUT
#define ATXLOGOUT stdout
#endif//ATXLOGOUT

////////////////////// ATXLANG STD LIB //////////////////////
int atxlangPrint(ATXLangClosure*);

////////////////////// ATXLANG DBG LIB //////////////////////
int atxlangPrintRawItem(ATXLangRaw*);

#ifdef  __cplusplus
}//extern "C"
#endif//__cplusplus

#endif//ATXLANG_H

#ifdef  ATXLANG_TEST

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    ATXLangCompilerOptions ATXO = { };
    ATXLangCompiler* atxc       = atxlangMakeCompiler(&ATXO);
    if (!atxc)
    {
        fprintf(stdout, ">>> Sorry, failed to make the compiler.%s", "\n");
        return 1;
    }

    char *programs[]            =
        {
            "print($HELLO_WORLD)",
            "print($HELLO_WORLD, $GOODBYE_CRUEL_WORLD)",
        };
    for (int PP = 0; PP < sizeof(programs)/sizeof(programs[0]); ++PP)
    {
        ATXLangModule* atxm     = atxlangCompile(atxc, &programs[PP][0], &programs[PP][0] + strlen(programs[PP]));
        if (!atxm)
        {
            fprintf(stdout, ">>> Sorry, failed to make the module.%s", "\n");
            return 1;
        }

        if (!atxlangFreeModule(atxm))
        {
            fprintf(stdout, ">>> Sorry, failed to free the module.%s", "\n");
            return 1;
        }
    }

    if (!atxlangFreeCompiler(atxc))
    {
        fprintf(stdout, ">>> Sorry, failed to free the compiler.%s", "\n");
        return 1;
    }

    ATXLangClosure *clozure = (ATXLangClosure*)calloc(sizeof(ATXLangClosure) + sizeof(ATXLangRaw) * 8, 1);
    clozure->numEntries     = 8;
    clozure->curPosition    = 0;

    char helloWorld[]       = "Hello, World!\n";
    clozure->stack[0].Type  = ATXLANG_TYPE_STRING;
    clozure->stack[0].U.STRING = &helloWorld[0];
    if (!atxlangPrint(clozure))
    {
        fprintf(stdout, ">>> Sorry, something went wrong with printing.%s", "\n");
    }
    else
    {
        fprintf(stdout, "*** Results, %s", "");
        atxlangPrintRawItem(&clozure->stack[0]);
        fprintf(stdout, "%s", "\n");
    }

    free(clozure);

    return 0;
}

#endif//ATXLANG_TEST

#ifdef  __cplusplus
extern "C"
{
#endif//__cplusplus

#ifdef  ATXLANG_STDLIB

#include <stdlib.h>
#include <string.h>

void* atxlang_malloc_context()
{
    return (void*)1;
}

void atxlang_malloc_release(void* handle)
{
}

void* atxlang_calloc(void* handle, unsigned long long bytes)
{
    return calloc(bytes, 1);
}

int atxlang_free(void* handle, void* p, unsigned long long bytes)
{
    free(p);
    return 1;
}

long atxlang_strncmp(char* left, char* right, unsigned long length)
{
    return strncmp(left, right, length);
}
#endif//ATXLANG_STDLIB

typedef struct ATXCmp
{
    void               *ATXH;
} ATXCmp;

typedef struct ATXThingString
{
    char               *begin;
    char               *end;
} ATXThingString;

typedef struct ATXThing
{
    int                 rsvd;
} ATXThing;

typedef struct ATXThingElt
{
    ATXThingString      Name;
    ATXThing           *Thing;
    struct ATXThingElt *Next;
} ATXThingElt;

typedef struct ATXThingEltList
{
    //
    // begin -> N ->Next
    //          N ->Next
    //          ...
    // end   -> N ->Next = 0
    //
    // End points to the last element; the
    // last element points to 0.
    //
    ATXThingElt        *begin;
    ATXThingElt        *end;
} ATXThingEltList;

typedef struct ATXMod
{
    void               *ATXH;
    ATXThingEltList     Things;
    ATXThingEltList     OwnedThings;
} ATXMod;

ATXLangCompiler* atxlangMakeCompiler(ATXLangCompilerOptions* ATXO)
{
    void* atxh          = atxlang_malloc_context();
    if (!atxh)
    {
        return 0;
    }

    ATXLangCompiler* atx    = atxlang_calloc(atxh, sizeof(ATXCmp));

    if (atx)
    {
        ATXCmp* atxc        = (ATXCmp*)atx;
        atxc->ATXH          = atxh;
    }
    else
    {
        atxlang_malloc_release(atxh);
    }

    return atx;
}

int atxlangFreeCompiler(ATXLangCompiler* atx)
{
    if (!atx)
    {
        return 0;
    }

    ATXCmp* atxc    = (ATXCmp*)atx;

    void* atxh      = atxc->ATXH;
    int fres        = atxlang_free(atxh, atx, sizeof(ATXCmp));
    atxlang_malloc_release(atxh);

    return fres;
}

typedef enum ATXLexemeType
{
    ATXTY_UNKNOWN,
    ATXTY_IDENTIFIER    = 0x1000,
    ATXTY_NUMBER,
    ATXTY_SYMBOL        = 0x2000,

    ATXTY_DO            = 0x4000,
    ATXTY_WHILE,
    ATXTY_FOR,
    ATXTY_CASE,
    ATXTY_IF,
    ATXTY_THEN,
    ATXTY_ELSE,
    ATXTY_SWITCH,
    ATXTY_BREAK,
    ATXTY_CONTINUE,

    ATXTY_OPERATOR_FLAG  = 0x1000000, // All operators are 3 chars or less; directly encode them.
} ATXLexemeType;

#define ATXTY_MK_OP1(O) (ATXTY_OPERATOR_FLAG | ((O) & 0xFF))
#define ATXTY_MK_OP2(OH, OL) (ATXTY_OPERATOR_FLAG | (((OH) & 0xFF) << 8) | ((OL) & 0xFF))
#define ATXTY_MK_OP3(OH, OL, OLB) (ATXTY_OPERATOR_FLAG | (((OH) & 0xFF) << 16) | (((OL) & 0xFF) << 8) | ((OB) & 0xFF))

typedef struct ATXLexeme
{
    ATXLexemeType   type;
    int             line;
    char           *lineBegin;
    char           *begin;
    char           *cur;
    char           *end;
} ATXLexeme;

ATXLexeme* atxlangLexNext(ATXLexeme* lex)
{
    if (!lex)
    {
        return 0;
    }
    if (!lex->begin || !lex->cur || !lex->end)
    {
        return 0;
    }
    if (!((lex->begin == lex->cur) && (lex->cur < lex->end)))
    {
        return 0;
    }

    // XXX: We've aliased the begin to the current pointer.
    // That's bad.

    lex->type       = ATXTY_UNKNOWN;

    // Eat WS.
    while ((lex->cur < lex->end) &&
            ((lex->cur[0] == ' ')   ||
             (lex->cur[0] == '\n')  ||
             (lex->cur[0] == '\t')  ||
             (lex->cur[0] == '\v')  ||
             (lex->cur[0] == '\f')  ||
              0))
    {
        if (lex->cur[0] == '\n')
        {
            lex->lineBegin  = lex->cur+1;
            ++lex->line;
        }
        ++lex->cur;
    }

    if (lex->cur >= lex->end)
    {
        return lex;
    }

    // Operators, first.
    switch (lex->cur[0])
    {
    case '('    :
    case ')'    :
    case '['    :
    case ']'    :
    case '{'    :
    case '}'    :
    case ':'    :
    case ','    :
    case ';'    :
    case '+'    :
    case '-'    :
    case '*'    :
    case '/'    :
    case '^'    :
    case '~'    :
    case '!'    :
        lex->type   = ATXTY_OPERATOR_FLAG | (int)lex->cur[0];
        ++lex->cur;
        if (lex->cur >= lex->end)
        {
            return lex;
        }
        if (lex->type == (ATXTY_OPERATOR_FLAG | (int)'!'))
        {
            switch (lex->cur[0])
            {
            case '='    :
                lex->type   = (ATXTY_OPERATOR_FLAG | (((int)'!') << 8) | (int)'=');
                ++lex->cur;
                break;
            }
        }
        return lex;
    case '='    :
    case '&'    :
    case '|'    :
    case '<'    :
    case '>'    :
        lex->type   = ATXTY_OPERATOR_FLAG | (int)lex->cur[0];
        ++lex->cur;
        if (lex->cur >= lex->end)
        {
            return lex;
        }
        if (lex->cur[0] == (lex->cur[-1]))
        {
            lex->type   = ATXTY_OPERATOR_FLAG | (((int)lex->cur[0]) << 8) | ((int)lex->cur[0]);
        }
        return lex;
    default     : break;
    }

    switch (lex->cur[0])
    {
    case 'b'    : // break
        if (((lex->end - lex->cur) >= 5)    &&
            (lex->cur[1] == 'r') && (lex->cur[2] == 'e') && (lex->cur[3] == 'a') && (lex->cur[4] == 'k'))
        {
            lex->type   = ATXTY_DO;
            lex->cur    += 5;
            return lex;
        }
        break;
    case 'c'    : // case, continue
        if (((lex->end - lex->cur) >= 4)    &&
            (lex->cur[1] == 'a') && (lex->cur[2] == 's') && (lex->cur[3] == 'e'))
        {
            lex->type   = ATXTY_CASE;
            lex->cur    += 4;
            return lex;
        }
        if (((lex->end - lex->cur) >= 8)    &&
            (lex->cur[1] == 'o') && (lex->cur[2] == 'n') && (lex->cur[3] == 't') && (lex->cur[4] == 'i') && (lex->cur[5] == 'n') && (lex->cur[6] == 'u') && (lex->cur[7] == 'e'))
        {
            lex->type   = ATXTY_CONTINUE;
            lex->cur    += 8;
            return lex;
        }
        break;
    case 'd'    : // do
        if (((lex->end - lex->cur) >= 2)    &&
            (lex->cur[1] == 'o'))
        {
            lex->type   = ATXTY_DO;
            lex->cur    += 2;
            return lex;
        }
    case 'e'    : // else
        if (((lex->end - lex->cur) >= 4)    &&
            (lex->cur[1] == 'l') && (lex->cur[2] == 's') && (lex->cur[3] == 'e'))
        {
            lex->type   = ATXTY_ELSE;
            lex->cur    += 4;
            return lex;
        }
    case 'f'    : // for
        if (((lex->end - lex->cur) >= 3)    &&
            (lex->cur[1] == 'o') && (lex->cur[2] == 'r'))
        {
            lex->type   = ATXTY_FOR;
            lex->cur    += 3;
            return lex;
        }
    case 'i'    : // if
        if (((lex->end - lex->cur) >= 2)    &&
            (lex->cur[1] == 'f'))
        {
            lex->type   = ATXTY_IF;
            lex->cur    += 4;
            return lex;
        }
    case 's'    : // switch
        if (((lex->end - lex->cur) >= 6)    &&
            (lex->cur[1] == 'w') && (lex->cur[2] == 'i') && (lex->cur[3] == 't') && (lex->cur[4] == 'c') && (lex->cur[5] == 'h'))
        {
            lex->type   = ATXTY_SWITCH;
            lex->cur    += 6;
            return lex;
        }
    case 't'    : // then
        if (((lex->end - lex->cur) >= 4)    &&
            (lex->cur[1] == 'h') && (lex->cur[2] == 'e') && (lex->cur[3] == 'n'))
        {
            lex->type   = ATXTY_THEN;
            lex->cur    += 4;
            return lex;
        }
    case 'w'    : // while
        if (((lex->end - lex->cur) >= 5)    &&
            (lex->cur[1] == 'h') && (lex->cur[2] == 'i') && (lex->cur[3] == 'l') && (lex->cur[4] == 'e'))
        {
            lex->type   = ATXTY_WHILE;
            lex->cur    += 5;
            return lex;
        }
    default     : break;
    }

    if (lex->cur[0] == '$')
    {
        lex->type   = ATXTY_SYMBOL;
        ++lex->cur;
        if (lex->cur >= lex->end)
        {
            return 0;
        }
    }
    char* first = lex->cur;
    while ( (lex->begin < lex->end) &&
            ((('a' <= lex->cur[0]) && (lex->cur[0] <= 'z')) ||
             (('A' <= lex->cur[0]) && (lex->cur[0] <= 'Z')) ||
              (lex->cur[0] == '_')                          ||
              0))
    {
        ++lex->cur;
    }
    if (first < lex->cur)
    {
        lex->type   |= ATXTY_IDENTIFIER;
        return lex;
    }
    while ( (lex->begin < lex->end) &&
            ((('a' <= lex->cur[0]) && (lex->cur[0] <= 'f')) ||
             (('A' <= lex->cur[0]) && (lex->cur[0] <= 'F')) ||
             (('0' <= lex->cur[0]) && (lex->cur[0] <= '9')) ||
              (lex->cur[0] == '_')                          ||
              0))
    {
        ++lex->cur;
    }
    if (first < lex->cur)
    {
        lex->type   |= ATXTY_NUMBER;
        return lex;
    }

    return 0;
}

typedef enum ATXParseState
{
    ATXPS_UNKNOWN,
    ATXPS_START,
    ATXPS_TOPFORM,      // TOPFORM ::= X `:` type `=` expr
    ATXPS_CHI,          //       X ::= <id> params?
    ATXPS_DEFINING,
    ATXPS_TYPING,

    ATXPS_PARAM_TUPLE,  // params ::= `(` (x [`,` x])? `)`
    ATXPS_PARAMS,
    ATXPS_PARAM,

    ATXPS_IDENTIFIER,
    ATXPS_COMMA,
    ATXPS_LPAREN,
    ATXPS_RPAREN,

    ATXPS_OPT       = 0x4000,
    ATXPS_MARK      = 0x8000,
} ATXParseState;

typedef struct ATXParseContext
{
    uint16_t   *StackBegin;
    uint16_t   *StackEnd;
    uint16_t   *StackPos;
} ATXParseContext;

int atxlangPushParseState(ATXParseContext* pctx, ATXParseState state)
{
    if ((pctx->StackPos+1) >= pctx->StackEnd)
    {
        return 0;
    }

    *pctx->StackPos     = state;
    ++pctx->StackPos;

    return 1;
}

int atxlParseStateEmpty(ATXParseContext* pctx)
{
    return pctx->StackPos == pctx->StackBegin;
}

ATXParseState atxlangPeekParseState(ATXParseContext* pctx)
{
    if ((pctx->StackPos-1) < pctx->StackBegin)
    {
        return ATXPS_UNKNOWN;
    }

    ATXParseState S = (ATXParseState)*(pctx->StackPos-1);

    return S;
}

int atxlangMarkParseState(ATXParseContext* pctx)
{
    if ((pctx->StackPos >= pctx->StackEnd))
    {
        return 0;
    }

    pctx->StackPos[-1]  |= ATXPS_MARK;
    return 1;
}

ATXParseState atxlangPopParseState(ATXParseContext* pctx)
{
    if ((pctx->StackPos-1) < pctx->StackBegin)
    {
        return ATXPS_UNKNOWN;
    }

    --pctx->StackPos;

    return (ATXParseState)*pctx->StackPos;
}

char* atxlangRecognizeTopForm(ATXParseContext* pctx, char* begin, char* end)
{
    ATXLexeme lex       =
    {
        .type           = ATXTY_UNKNOWN,
        .line           = 1,
        .lineBegin      = begin,
        .begin          = begin,
        .cur            = begin,
        .end            = end,
    };
    ATXLexemeType Op    = ATXTY_MK_OP1('\0');
    ATXLexeme* l        = 0;
    atxlangPushParseState(pctx, ATXPS_START);
    while (!atxlParseStateEmpty(pctx))
    {
        ATXParseState State = atxlangPeekParseState(pctx);
        int Marked          = !!(State & ATXPS_MARK);
        int Optional        = !!(State & ATXPS_OPT);
        State               &= ~(ATXPS_MARK | ATXPS_OPT);
        switch (State)
        {
        case ATXPS_UNKNOWN      :
            return 0; // ERROR
        case ATXPS_START        :
            ATX_DBG_LOG("START%s\n%s", !Marked ? "" : "!", "");
            if (!Marked)
            {
                atxlangMarkParseState(pctx);
                atxlangPushParseState(pctx, ATXPS_TOPFORM);
            }
            else
            {
                return 0;
            }
            break;
        case ATXPS_TOPFORM      :
            ATX_DBG_LOG("TOPFORM%s\n%s", !Marked ? "" : "!", "");
            if (!Marked)
            {
                atxlangMarkParseState(pctx);
                atxlangPushParseState(pctx, ATXPS_OPT | ATXPS_DEFINING);
                atxlangPushParseState(pctx, ATXPS_OPT | ATXPS_TYPING);
                atxlangPushParseState(pctx, ATXPS_CHI);
            }
            else
            {
                atxlangPopParseState(pctx);
            }
            break;
        case ATXPS_DEFINING     :
            ATX_DBG_LOG("DEFINING%s%s\n%s", !Marked ? "" : "!", Optional ? "?" : "", "");
            atxlangPopParseState(pctx);
            break;
        case ATXPS_TYPING       :
            ATX_DBG_LOG("TYPING%s%s\n%s", !Marked ? "" : "!", Optional ? "?" : "", "");
            atxlangPopParseState(pctx);
            break;
        case ATXPS_CHI          :
            ATX_DBG_LOG("CHI%s%s\n%s", !Marked ? "" : "!", Optional ? "?" : "", "");
            if (!Marked)
            {
                atxlangMarkParseState(pctx);
                atxlangPushParseState(pctx, ATXPS_OPT | ATXPS_PARAM_TUPLE);
                atxlangPushParseState(pctx, ATXPS_IDENTIFIER);
            }
            else
            {
                atxlangPopParseState(pctx);
            }
            break;
        case ATXPS_PARAM_TUPLE  :
            ATX_DBG_LOG("PARAM_TUPLE%s%s\n%s", !Marked ? "" : "!", Optional ? "?" : "", "");
            if (!Marked)
            {
                atxlangMarkParseState(pctx);
                atxlangPushParseState(pctx, ATXPS_RPAREN);
                atxlangPushParseState(pctx, ATXPS_PARAMS);
                atxlangPushParseState(pctx, ATXPS_LPAREN);
            }
            else
            {
                atxlangPopParseState(pctx);
            }
            break;
        case ATXPS_PARAMS       :
            ATX_DBG_LOG("PARAMS%s%s\n%s", !Marked ? "" : "!", Optional ? "?" : "", "");
            atxlangPopParseState(pctx);
            //if (!Marked)
            //{
            //    atxlangMarkParseState(pctx);
            //    atxlangPushParseState(pctx, ATXPS_PARAM);
            //}
            //else
            //{
            //    // Look for comma?
            //    if (comma)
            //    {
            //        atxlangPushParseState(pctx, ATXPS_PARAM);
            //    }
            //    else
            //    {
            //        atxlangPopParseState(pctx);
            //    }
            //}
            break;
        case ATXPS_PARAM        :
            ATX_DBG_LOG("PARAM%s%s\n%s", !Marked ? "" : "!", Optional ? "?" : "", "");
            atxlangPopParseState(pctx);
            break;

        case ATXPS_IDENTIFIER   :
            ATX_DBG_LOG("IDENTIFIER%s%s\n%s", !Marked ? "" : "!", Optional ? "?" : "", "");
            atxlangPopParseState(pctx);
            l                   = atxlangLexNext(&lex);
            if (!l)
            {
                return 0;
            }
            if (l->type != ATXTY_IDENTIFIER)
            {
                return 0;
            }
            lex.begin           = l->cur;
            break;
        case ATXPS_COMMA        :
            Op                  = ATXTY_MK_OP1(',');
            goto PARSE_OPERATOR;
        case ATXPS_LPAREN       :
            Op                  = ATXTY_MK_OP1('(');
            goto PARSE_OPERATOR;
        case ATXPS_RPAREN       :
            Op                  = ATXTY_MK_OP1(')');
            goto PARSE_OPERATOR;
PARSE_OPERATOR                  :
            ATX_DBG_LOG("OP(%c)%s%s\n%s", (char)Op, !Marked ? "" : "!", Optional ? "?" : "", "");
            atxlangPopParseState(pctx);
            l                   = atxlangLexNext(&lex);
            if (!l)
            {
                return 0;
            }
            if (l->type != Op)
            {
                return 0;
            }
            lex.begin           = l->cur;
            break;

        case ATXPS_OPT          :
            return 0; // ERROR.
        case ATXPS_MARK         :
            return 0; // ERROR.
        }
    }

    return 0;
}

char* atxlangParse(char* begin, char* end)
{
    char* cur               = begin;
    uint16_t Stack[1024]    = { };
    ATXParseContext pctx    = { };
    pctx.StackBegin         = &Stack[0];
    pctx.StackPos           = pctx.StackBegin;
    pctx.StackEnd           = pctx.StackBegin + sizeof(Stack);
    while ((cur = atxlangRecognizeTopForm(&pctx, cur, end)))
    {
    }
    return 0;
}

ATXLangModule* atxlangCompile(ATXLangCompiler* atx, char* begin, char* end)
{
    if (!atx)
    {
        return 0;
    }

    if (!begin || !end)
    {
        return 0;
    }

    if (begin >= end)
    {
        return 0;
    }

    ATXCmp* atxc        = (ATXCmp*)atx;

    ATXMod* module      = (ATXMod*)atxlang_calloc(atxc->ATXH, sizeof(ATXMod));
    module->ATXH        = atxc->ATXH;

    atxlangParse(begin, end);

    return (ATXLangModule*)module;
}

int atxlangFreeThing(ATXLangModule* atxm, ATXThing* thing)
{
    if (!atxm)
    {
        return 0;
    }
    if (!thing)
    {
        return 0;
    }

    ATXMod* parent      = (ATXMod*)atxm;

    atxlang_free(parent->ATXH, thing, sizeof(ATXThing));

    return 1;
}

int atxlangFreeModule(ATXLangModule* atxm)
{
    if (!atxm)
    {
        return 0;
    }

    ATXMod* myModule            = (ATXMod*)atxm;

    for (ATXThingElt* thing = myModule->Things.begin; thing; )
    {
        ATXThingElt* nextThing  = thing->Next;
        atxlang_free(myModule->ATXH, thing, sizeof(ATXThingElt));
        thing                   = nextThing;
    }
    myModule->Things.begin      = 0;
    myModule->Things.end        = 0;

    for (ATXThingElt* thing = myModule->OwnedThings.begin; thing; )
    {
        ATXThingElt* nextThing  = thing->Next;
        atxlang_free(myModule->ATXH, thing, sizeof(ATXThingElt));
        atxlang_free(myModule->ATXH, thing->Name.begin, thing->Name.end - thing->Name.begin);
        atxlangFreeThing(atxm, thing->Thing);
        thing                   = nextThing;
    }
    myModule->OwnedThings.begin = 0;
    myModule->OwnedThings.end   = 0;

    atxlang_free(myModule->ATXH, myModule, sizeof(ATXMod));

    return 1;
}

int atxlangLinkModule(ATXLangModule* atxm, int numModules, ATXLangModule** atxms)
{
    if (!atxm)
    {
        return 0;
    }

    ATXMod* myModule            = (ATXMod*)atxm;

    for (int ZZ = 0; ZZ < numModules; ++ZZ)
    {
        ATXMod* otherModule     = (ATXMod*)atxms[ZZ];
        ATXThingElt* myEnd      = myModule->Things.end;
        ATXThingElt* itsBeg     = otherModule->Things.begin;
        for (ATXThingElt* itsThing = itsBeg; itsThing; itsThing = itsThing->Next)
        {
            ATXThingElt* copy   = atxlang_calloc(myModule->ATXH, sizeof(ATXThingElt));
            copy->Name.begin    = itsThing->Name.begin;
            copy->Name.end      = itsThing->Name.end;
            copy->Thing         = itsThing->Thing;
            myEnd->Next         = copy;
            myEnd               = copy;
        }
        myModule->Things.end    = myEnd;
    }

    return 1;
}

ATXLangThing* atxlangGet(ATXLangModule* atxm, char* thingBeg, char* thingEnd)
{
    if (!atxm)
    {
        return 0;
    }

    ATXMod* atxmm   = (ATXMod*)atxm;
    int length      = thingEnd - thingBeg;
    for (ATXThingElt* thing = atxmm->Things.begin; thing; thing = thing->Next)
    {
        int tlen    = thing->Name.end - thing->Name.begin;
        if (tlen != length)
        {
            continue;
        }
        if (!strncmp(thing->Name.begin, thingBeg, tlen))
        {
            return (ATXLangThing*)thing;
        }
    }

    return 0;
}

int atxlangCall(ATXLangThing* atxt, ATXLangClosure* atxclosure)
{
    return 0;
}

////////////////////// ATXLANG STD LIB //////////////////////
int atxlangPrint(ATXLangClosure* atxclosure)
{
    if (!atxclosure)
    {
        return 0;
    }

    if (atxclosure->numEntries < 1)
    {
        return 0;
    }

    if (atxclosure->curPosition < 0)
    {
        return 0;
    }

    // Pop string, push number bytes printed.
    ATXLangRaw* top         = &atxclosure->stack[atxclosure->curPosition];
    if (top->Type != ATXLANG_TYPE_STRING)
    {
        return 0;
    }

    char* toPrint           = top->U.STRING;
    top->Type               = ATXLANG_TYPE_INT;
    top->U.INT              = fprintf(stdout, "%s", toPrint);

    return 1;
}

////////////////////// ATXLANG STD LIB //////////////////////
int atxlangPrintRawItem(ATXLangRaw* atxraw)
{
    if (!atxraw)
    {
        return 0;
    }

    switch (atxraw->Type)
    {
    case ATXLANG_TYPE_INT       : return fprintf(stdout, "%lld", atxraw->U.INT); break;
    case ATXLANG_TYPE_FLOAT     : return fprintf(stdout, "%lf", atxraw->U.FLOAT); break;
    case ATXLANG_TYPE_STRING    : return fprintf(stdout, "%s", atxraw->U.STRING); break;
    default                     : return 0;
    }
}

#ifdef  __cplusplus
}//end extern "C"
#endif//__cplusplus
