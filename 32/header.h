#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TK_NUM,
  TK_IDENT,
  TK_PUNCT,
  TK_KEYWORD,
  TK_STR,
  TK_EOF,
} TokenKind;

typedef struct Type Type;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  int val;
  char *loc;
  int len;
  char *str;
  Type *ty;
};

typedef enum {
  TY_INT,
  TY_CHAR,
  TY_PTR,
  TY_ARRAY,
  TY_FUNC,
} TypeKind;

struct Type {
  TypeKind kind;
  int size;
  Type *next;
  Type *base;
  Type *return_ty;
  Type *params;
  Token *name;
  int array_len;
};

extern Type *ty_int;
extern Type *ty_char;

void error(char *fmt, ...);
void error_at(char *at, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
Token *tokenize(char *p);
Type *array_of(Type *base, int len);