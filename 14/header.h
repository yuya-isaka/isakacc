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
	TK_EOF,
	TK_KEYWORD,
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

typedef struct Obj Obj;
struct Obj
{
	Obj *next;
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
	ND_ASSIGN,
	ND_VAR,
	ND_BLOCK,
	ND_RETURN,
	ND_IF,
	ND_EXPR_STMT,
} NodeKind;

typedef struct Node Node;
struct Node
{
	NodeKind kind;
	Node *next;

	Node *lhs;
	Node *rhs;

	Node *body;

	Node *cond;
	Node *then;
	Node *els;

	int val;
	Obj *var;
};

typedef struct Function Function;
struct Function
{
	Node *body;
	Obj *locals;
	int stack_size;
};

bool equal(Token *tok, char *target);
Token *skip(Token *tok, char *target);
Function *parse(Token *tok);
void codegen(Function *prog);
void error_tok(Token *tok, char *fmt, ...);