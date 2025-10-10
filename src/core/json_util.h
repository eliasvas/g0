#ifndef JSON_UTIL_H__
#define JSON_UTIL_H__

#include "base/helper.h"

typedef enum {
    JSON_TOKEN_KIND_COMMA,
    JSON_TOKEN_KIND_COLON,
    JSON_TOKEN_KIND_LPAREN,
    JSON_TOKEN_KIND_RPAREN,
    JSON_TOKEN_KIND_LBRACKET,
    JSON_TOKEN_KIND_RBRACKET,
    JSON_TOKEN_KIND_LCURLY,
    JSON_TOKEN_KIND_RCURLY,
    JSON_TOKEN_KIND_NUMBER,
    JSON_TOKEN_KIND_TRUE,
    JSON_TOKEN_KIND_FALSE,
    JSON_TOKEN_KIND_NULL,
    JSON_TOKEN_KIND_STRING,
} Json_Token_Kind;
typedef struct {
    char *v;
    int c;
    Json_Token_Kind kind;
} Json_Token;
typedef struct Json_Token_Node Json_Token_Node;
struct Json_Token_Node {
    Json_Token_Node *next;
    Json_Token tok;
};
typedef struct {
    Json_Token_Node *first;
    Json_Token_Node *last;
} Json_Tokens;


//@FIXME: This is not correct, FIX this
static int json_tok_count_number(char* num_str) {
    int count = 0;
    for(u32 i = 0; i < strlen(num_str); ++i, ++count) {
        char c = num_str[i];
        if (c != '.' && c != '-' && (c < '0' || c > '9'))break;
    }
    return count;
}

//@FIXME: This is not correct, FIX this
static int json_tok_count_string(char* s) {
    int count = 0;
    int quote_count = 0;
    for(u32 i = 0; i < strlen(s); ++i, ++count) {
        if (quote_count == 2)break;
        char c = s[i];
        if (c == '\"')quote_count+=1;
    }
    return count;
}


static Json_Tokens json_tokenize(char *json_str) {
    Json_Tokens tokens = {};

    int count = 0;
    for (u32 i = 0; i < strlen(json_str); i+=count) {

        Json_Token token = {};
        char *c = &json_str[i];
        count = 1;
        switch (c[0]) {
           case ',':
                token = (Json_Token) {c, count, JSON_TOKEN_KIND_COMMA};
                break;
            case ':':
                token = (Json_Token) {c, count, JSON_TOKEN_KIND_COLON};
                break;
            case '(':
                token = (Json_Token) {c, count, JSON_TOKEN_KIND_LPAREN};
                break;
            case ')':
                token = (Json_Token) {c, count, JSON_TOKEN_KIND_RPAREN};
                break;
            case '[':
                token = (Json_Token) {c, count, JSON_TOKEN_KIND_LBRACKET};
                break;
            case ']':
                token = (Json_Token) {c, count, JSON_TOKEN_KIND_LBRACKET};
                break;
            case '{':
                token = (Json_Token) {c, count, JSON_TOKEN_KIND_LCURLY};
                break;
            case '}':
                token = (Json_Token) {c, count, JSON_TOKEN_KIND_RCURLY};
                break;
            case '-':
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                count = json_tok_count_number(c);
                token = (Json_Token) {c, count, JSON_TOKEN_KIND_NUMBER};
                break;
            case 't':
                count = strlen("true");
                token = (Json_Token) {c, count, JSON_TOKEN_KIND_TRUE};
                break;
            case 'f':
                count = strlen("false");
                token = (Json_Token) {c, count, JSON_TOKEN_KIND_FALSE};
                break;
            case 'n':
                count = strlen("null");
                token = (Json_Token) {c, count, JSON_TOKEN_KIND_NULL};
                break;
            case '\"':
                count = json_tok_count_string(c);
                assert(count);
                token = (Json_Token) {c, count, JSON_TOKEN_KIND_STRING};
                break;
            case ' ':
            case '\\':
            case '\t':
            case '\n':
            default:
                //printf("unhandled char:%s\n", &json_str[i]);
                //exit(1);
                break;
        }
        if (token.c) {
            Json_Token_Node *tok = M_ALLOC(sizeof(Json_Token_Node));
            memset(tok, 0, sizeof(Json_Token_Node));
            tok->tok = token;
            sll_queue_push(tokens.first, tokens.last, tok);
        }
    }

    return tokens;
}

static void json_tok_print(Json_Tokens tokens) {
    for (Json_Token_Node *n = tokens.first; n != nullptr; n = n->next) {
        printf ("%.*s\n", n->tok.c, n->tok.v);
    }
}

typedef struct Json_Object Json_Object;
typedef struct {
    Json_Object *first;
    Json_Object *last;

    char *key;
    Json_Object* value;
} Json_Object_Node;
struct Json_Object{
    Json_Object_Node *buckets;
    int bucket_count;
};

struct Json_Value;
typedef struct {
    struct Json_Value *values;
    int count;
}Json_Array;

typedef enum {
    JSON_PRIM_KIND_INTEGER,
    JSON_PRIM_KIND_REAL,
    JSON_PRIM_KIND_STRING,
    JSON_PRIM_KIND_BOOLEAN,
} Json_Prim_Kind;
typedef struct {
    union {
        int integer;
        float real;
        char *str;
        bool b;
    };
    Json_Prim_Kind kind;
} Json_Prim;

typedef enum {
    JSON_VALUE_KIND_PRIMITIVE,
    JSON_VALUE_KIND_ARRAY,
    JSON_VALUE_KIND_OBJECT,
} Json_Value_Kind;

typedef struct {
    union {
        Json_Prim prim;
        Json_Array arr;
        Json_Object obj;
    };
    Json_Value_Kind kind;
} Json_Value;


static char *str = "{ \"msg-type\": [ \"0xdeadbeef\", \"irc log\" ], \
\"msg-from\": { \"class\": \"soldier\", \"name\": \"Wixilav\" }, \
\"msg-to\": { \"class\": \"supreme-commander\", \"name\": \"[Redacted]\" }, \
\"msg-log\": [ \
    \"soldier: Boss there is a slight problem with the piece offering to humans\", \
    \"supreme-commander: Explain yourself soldier!\", \
    \"soldier: Well they don't seem to move anymore...\", \
    \"supreme-commander: Oh snap, I came here to see them twerk!\" \
    ] \
}";

#endif
