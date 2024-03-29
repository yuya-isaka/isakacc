#define _POSIX_C_SOURCE 200809L
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

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

typedef enum
{
	TY_INT,
	TY_PTR,
	TY_FUNC,
} TypeKind;

typedef struct Type Type;
struct Type
{
	TypeKind kind;
	Type *base;
	Token *name;
	Type *return_ty;
	Type *params;
	Type *next;
};

extern Type *ty_int;

typedef struct Obj Obj;
struct Obj
{
	Obj *next;
	Type *ty;
	char *name;
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
	ND_VAR,
	ND_ASSIGN,
	ND_DEREF,
	ND_ADDR,
	ND_FUNCALL,
	ND_RETURN,
	ND_BLOCK,
	ND_EXPR_STMT,
	ND_IF,
	ND_FOR,
} NodeKind;

typedef struct Node Node;
struct Node
{
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

typedef struct Function Function;
struct Function
{
	Function *next;
	Node *body;
	Obj *locals;
	char *name;
	int stack_size;
	Obj *params;
};

void error(char *fmt, ...);
void error_at(char *at, char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
Token *tokenize(char *p);
bool equal(Token *tok, char *target);
bool consume(Token **rest, Token *tok, char *target);
Token *skip(Token *tok, char *target);
Function *parse(Token *tok);
Type *pointer_to(Type *base);
Type *type_func(Type *return_ty);
void add_type(Node *node);
bool is_integer(Type *ty);
Type *copy_type(Type *ty);
void codegen(Function *prog);