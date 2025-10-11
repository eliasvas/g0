#ifndef JSON_UTIL_H__
#define JSON_UTIL_H__

#include "base/helper.h"
// TODO: Start thinking about error cases, rn we can parse only valid JSON files..
// TODO: remove strlen and strcmp! NO STDLIB, MAKE OUR OWN STRING_THING

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
// TODO: This should become a chunk list! Its too slow rn
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

// TODO: Maybe we should have json_parser_get_tok(..) instead of this guy
static Json_Token json_tok_get(Json_Tokens tokens, u32 idx) {
  Json_Token_Node *n = tokens.first;
  assert(n);
  for (; (idx>0) && n->next != nullptr; n = n->next, idx--) { }
  return n->tok;
}

static Json_Tokens json_tokenize(Arena *arena, char *json_str) {
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
                token = (Json_Token) {c, count, JSON_TOKEN_KIND_RBRACKET};
                break;
            case '{':
                token = (Json_Token) {c, count, JSON_TOKEN_KIND_LCURLY};
                break;
            case '}':
                token = (Json_Token) {c, count, JSON_TOKEN_KIND_RCURLY};
                break;
            case '-':
            case '+':
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
                // +1 -2 to remove the quotes '\"' FIXME: this is hacky
                token = (Json_Token) {c+1, count-2, JSON_TOKEN_KIND_STRING};
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
            Json_Token_Node *tok = arena_push_array(arena, Json_Token_Node, 1);
            tok->tok = token;
            sll_queue_push(tokens.first, tokens.last, tok);
        }
    }

    return tokens;
}

static void json_tok_print(Json_Tokens tokens) {
  u32 idx = 0; 
  for (Json_Token_Node *n = tokens.first; n != nullptr; n = n->next) {
    // Just for testing
    Json_Token tok = json_tok_get(tokens, idx);
    printf ("%.*s\n", tok.c, tok.v);
    idx+=1;
  }
}

typedef struct {
  char *b;
  u32 count;
} buf;

static b32 buf_eq(buf l, buf r) {
  return (l.b && r.b && l.count == r.count && strncmp(l.b, r.b, l.count) == 0);
}

// TODO: maybe we need a primitive kind here for validation purposes
typedef struct {
  union {
    int i;
    float f;
    buf buf;
    bool b;
  };
} Json_Primitive;

typedef struct Json_Element Json_Element;
struct Json_Element {
  Json_Primitive label;
  Json_Primitive value;

  Json_Element *first;
  Json_Element *next;
};

typedef struct {
  Json_Tokens tokens;
  u64 pos;
  Arena *arena;
}Json_Parser;

static Json_Token json_parser_get_tok(Json_Parser *parser) {
  return json_tok_get(parser->tokens, parser->pos);
}

static Json_Element* json_lookup(Json_Element *root, buf label) {
  Json_Element *iter = root->first;
  while (iter != nullptr && !buf_eq(iter->label.buf, label))iter = iter->next;

  return iter;
}


static Json_Element* json_parse_element(Json_Parser *parser, buf label);
static Json_Primitive json_parse_primitive(Json_Parser *parser) {
  Json_Primitive prim = {};
  Json_Token tok = json_parser_get_tok(parser);
  bool found = true;
  if (tok.kind == JSON_TOKEN_KIND_TRUE) {
    prim = (Json_Primitive) {
      .b = true
    };
  } else if (tok.kind == JSON_TOKEN_KIND_FALSE){
    prim = (Json_Primitive) {
      .b = false
    };
  } else if (tok.kind == JSON_TOKEN_KIND_STRING) {
    prim = (Json_Primitive) { 
      .buf = (buf){tok.v, tok.c} 
    };
  } else if (tok.kind == JSON_TOKEN_KIND_NULL){
    prim = (Json_Primitive) { 
      .i = 0
    };
  } else if (tok.kind == JSON_TOKEN_KIND_NUMBER){ 
    prim = (Json_Primitive) {
      .i = 42
    };
  } else {
    found = false;
  }

  if (found)parser->pos+=1;

  return prim;
}

static Json_Element* json_parse_list(Json_Parser *parser, buf label, Json_Token_Kind end_token, bool has_labels);
static Json_Element* json_parse_element(Json_Parser *parser, buf label) {
  Json_Token tok = json_parser_get_tok(parser);

  // Try to parse a sublist (if its an object or array)
  Json_Element *sublist = nullptr;
  if (tok.kind == JSON_TOKEN_KIND_LCURLY) {
    // TODO: Make a json_parser_eat(parser, TOKEN_KIND) that will error 
    parser->pos += 1; // eat the '{'
    sublist = json_parse_list(parser, label, JSON_TOKEN_KIND_RCURLY, true);
  }
  if (tok.kind == JSON_TOKEN_KIND_LBRACKET) {
    parser->pos += 1; // eat the '['
    sublist = json_parse_list(parser, label, JSON_TOKEN_KIND_RBRACKET, false);
  }

  // Try to parse actual value (if its a primitive) and return the elements
  Json_Element *e = arena_push_array(parser->arena, Json_Element, 1);
  e->first = sublist;
  e->label = (Json_Primitive) { .buf = label };
  e->next = nullptr;
  e->value = json_parse_primitive(parser);

  return e;
}

static Json_Element* json_parse_list(Json_Parser *parser, buf label, Json_Token_Kind end_token, bool has_labels) {
  //Json_Element *first = arena_push_array(parser->arena, Json_Element, 1);
  //Json_Element *last = arena_push_array(parser->arena, Json_Element, 1);
  Json_Element *first = nullptr;
  Json_Element *last = nullptr;

  Json_Primitive labelv = {};
  while (json_parser_get_tok(parser).kind != end_token) {
    if (has_labels) {
      // parse the label
      labelv = json_parse_primitive(parser);

      parser->pos+=1; // eat the colon
    }
    // parse the value
    Json_Element *value = json_parse_element(parser, label);
    value->label = labelv;
    assert(value);
    sll_queue_push(first, last, value);

    if (json_parser_get_tok(parser).kind == JSON_TOKEN_KIND_COMMA) parser->pos+=1; // eat the comma 
  }

  // eat the end_token
  parser->pos+=1;

  return first;
}

static Json_Element *json_parse(Arena * arena, Json_Tokens tokens) {
  Json_Parser p = (Json_Parser) {
    .tokens = tokens,
    .pos = 0,
    .arena = arena
  };
  Json_Element *e = json_parse_element(&p, (buf){});

  return e;
}

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
