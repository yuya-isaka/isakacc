#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

typedef enum
{
	TK_IDENT,
	TK_PUNCT,
	TK_NUM,
	TK_EOF
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
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_NUM,
	ND_NEG,
	ND_EQ,
	ND_NE,
	ND_LT,
	ND_LE,
	ND_EXPR_STMT,
	ND_IDENT,
	ND_ASSIGN
} NodeKind;

typedef struct Node Node;
struct Node
{
	NodeKind kind;
	Node *next;
	Node *lhs;
	Node *rhs;
	int val;
	char name;
};

void error_tok(Token *tok, char *fmt, ...);
void error(char *fmt, ...);
Token *tokenize(char *p);
Node *parse(Token *tok);
void codegen(Node *node);