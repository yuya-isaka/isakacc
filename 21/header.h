#define _POSIX_C_SOURCE 200809L
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

void error(char *fmt, ...);

typedef enum
{
	TK_NUM,
	TK_IDENT,
	TK_PUNCT,
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
void error_tok(Token *tok, char *fmt, ...);

typedef enum
{
	TY_PTR,
	TY_INT,
	TY_FUNC,
} TypeKind;

typedef struct Type Type;
struct Type
{
	TypeKind kind;
	Type *base;
	Token *name;
	Type *return_ty;
};

extern Type *ty_int;

typedef struct Obj Obj;
struct Obj
{
	Obj *next;
	char *name;
	int offset;
	Type *ty;
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
	ND_VAR,
	ND_ASSIGN,
	ND_DEREF,
	ND_ADDR,
	ND_IF,
	ND_FOR,
	ND_RETURN,
	ND_BLOCK,
	ND_EXPR_STMT,
	ND_FUNCALL,
} NodeKind;

typedef struct Node Node;
struct Node
{
	NodeKind kind;
	Token *tok;
	Type *ty;
	Node *next;

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

typedef struct Function Function;
struct Function
{
	Function *next;
	char *name;
	Node *body;
	Obj *locals;
	int stack_size;
};

Function *parse(Token *tok);
bool equal(Token *tok, char *target);
Token *skip(Token *tok, char *target);
bool consume(Token **rest, Token *tok, char *target);

void codegen(Function *prog);

bool is_integer(Type *ty);
void add_type(Node *node);
Type *func_type(Type *return_ty);
Type *pointer_to(Type *base);