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
  ND_DEREF,
  ND_ADDR,
  ND_FUNCALL,

  ND_RETURN,
  ND_BLOCK,
  ND_EXPR_STMT,
  ND_STMT_EXPR,
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
  Obj *next;  // 共通
  Type *ty;   // 共通
  char *name; // 共通

  bool is_func;   // 関数
  bool is_global; // 共通

  Node *body;  // 関数
  Obj *locals; // 関数
  Obj *params; // 関数

  char *init_data;

  int offset;     // 変数
  int stack_size; // 関数
};

void error(char *fmt, ...);
void error_at(char *at, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
Token *tokenize(char *p);
Type *array_of(Type *base, int len);
Type *copy_ty(Type *ty);
Type *type_func(Type *base);
void add_type(Node *node);
Type *pointer_to(Type *base);
bool is_integer(Type *ty);
bool equal(Token *tok, char *p);
bool consume(Token **rest, Token *tok, char *target);
Token *skip(Token *tok, char *target);
Obj *parse(Token *tok);
char *format(char *fmt, ...);
void codegen(Obj *prog);