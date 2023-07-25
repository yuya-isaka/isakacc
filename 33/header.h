#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
  TK_NUM,
  TK_IDNET,
  TK_PUNCT,
  TK_STR,
  TK_KEYWORD,
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
  int array_len;
  Token *name;
};

extern Type *ty_int;
extern Type *ty_char;

void error(char *fmt, ...);
void error_at(char *at, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
Token *tokenize_file(char *input_path);
Type *array_of(Type *base, int len);
bool equal(Token *tok, char *target);