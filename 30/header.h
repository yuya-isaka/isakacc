#define _POSIX_C_SOURCE 200809L
#include <assert.h>
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
  Type *ty;
  char *str;
};

typedef enum {
  TY_CHAR,
  TY_INT,
  TY_PTR,
  TY_FUNC,
  TY_ARRAY,
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

extern Type *ty_char;
extern Type *ty_int;

typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_EQ,
  ND_NE,
  ND_LT,
  ND_LE,

  ND_NUM,
  ND_NEG,
  ND_VAR,
  ND_ASSIGN,
  ND_ADDR,
  ND_DEREF,
  ND_FUNCALL,

  ND_RETURN,
  ND_EXPR_STMT,
  ND_BLOCK,
  ND_IF,
  ND_FOR,
} NodeKind;

typedef struct Obj Obj;

typedef struct Node Node;
struct Node {
  NodeKind kind;
  Node *next;
  Token *tok;
  Type *ty;

  Node *lhs;
  Node *rhs;

  Node *cond;
  Node *then;
  Node *els;

  Node *init;
  Node *inc;

  Node *body;

  int val;
  Obj *var;

  char *funcname;
  Node *args;
};

struct Obj {
  Obj *next;
  Type *ty;

  bool is_local;
  bool is_func;

  char *init_data;

  Node *body;
  Obj *locals;
  Obj *params;
  char *name;
  int offset;
  int stack_size;
};

void error(char *fmt, ...);
void error_at(char *at, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
Token *tokenize(char *p);
bool equal(Token *tok, char *target);
bool consume(Token **rest, Token *tok, char *target);
Token *skip(Token *tok, char *target);
Obj *parse(Token *tok);
Type *pointer_to(Type *base);
Type *func_type(Type *return_ty);
Type *copy_ty(Type *ty);
Type *array_of(Type *base, int len);
void add_type(Node *node);
bool is_integer(Type *ty);
void codegen(Obj *prog);
char *format(char *fmt, ...);