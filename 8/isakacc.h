#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

typedef enum
{
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
	ND_LE
} NodeKind;

typedef struct Node Node;
struct Node
{
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
};

void error(char *fmt, ...);
void verror(char *at, char *fmt, va_list ap);
void error_at(char *at, char *fmt, ...);
void error_tok(Token *at, char *fmt, ...);

Node *parse(Token *tok);
Token *tokenize(char *p);
void codegen(Node *node);