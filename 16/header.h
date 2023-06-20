#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

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

void error(char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
Token *tokenize(char *p);
bool equal(Token *tok, char *target);

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
	ND_IF,
	ND_FOR,
	ND_RETURN,
	ND_BLOCK,
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
	Token *tok;

	Node *cond;
	Node *then;
	Node *els;

	Node *init;
	Node *inc;

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

Function *parse(Token *tok);