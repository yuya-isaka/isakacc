#define _POSIX_C_SOURCE 200809L
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

typedef enum
{
	TK_NUM,
	TK_PUNCT,
	TK_IDENT,
	TK_KEYWORD,
	TK_EOF,
} TokenKind;

typedef struct Token Token;
struct Token
{
	TokenKind kind;
	Token *next;
	int val;
	char *loc;
	int len;
};

Token *tokenize(char *p);
void error(char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
void error_at(char *at, char *fmt, ...);

typedef enum
{
	TY_INT,
	TY_PTR,
} TypeKind;

typedef struct Type Type;
struct Type
{
	TypeKind kind;
	Type *base;
	Token *name;
};

extern Type *ty_int;

typedef struct Obj Obj;
struct Obj
{
	Obj *next;
	char *name;
	Type *ty;
	int offset;
};

typedef enum
{
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
	ND_ASSIGN,
	ND_VAR,
	ND_EXPR_STMT,
	ND_IF,
	ND_FOR,
	ND_RETURN,
	ND_BLOCK,
	ND_DEREF,
	ND_ADDR,
} NodeKind;

typedef struct Node Node;
struct Node
{
	NodeKind kind;
	Node *next;
	Token *tok;
	Type *ty;
	Obj *var;
	int val;

	Node *lhs;
	Node *rhs;
	Node *cond;
	Node *then;
	Node *els;
	Node *init;
	Node *inc;
	Node *body;
};

typedef struct Function Function;
struct Function
{
	Node *body;
	Obj *locals;
	int stack_size;
};

Function *parse(Token *tok);

Token *skip(Token *tok, char *target);
bool equal(Token *tok, char *target);
bool consume(Token **rest, Token *tok, char *target);

bool is_integer(Type *ty);
Type *pointer_to(Type *base);
void add_type(Node *node);

void codegen(Function *prog);