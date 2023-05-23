#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

static char *user_input;

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

static void error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(EXIT_FAILURE);
}

static void verror(char *at, char *fmt, va_list ap)
{
	int len = at - user_input;

	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", len, "");
	fprintf(stderr, "^");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(EXIT_FAILURE);
}

static void error_tok(Token *tok, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verror(tok->loc, fmt, ap);
}

static void error_at(char *at, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verror(at, fmt, ap);
}

Token *new_token(TokenKind kind, char *start, char *end)
{
	Token *new = calloc(1, sizeof(Token));
	new->kind = kind;
	new->loc = start;
	new->len = end - start;
	return new;
}

static Token *tokenize()
{
	char *p = user_input;
	Token head = {};
	Token *cur = &head;

	while (*p)
	{
		if (isspace(*p))
		{
			p++;
			continue;
		}

		if (isdigit(*p))
		{
			cur = cur->next = new_token(TK_NUM, p, p);
			char *tmp = p;
			cur->val = strtol(p, &p, 10);
			cur->len = p - tmp;
			continue;
		}

		if (ispunct(*p))
		{
			cur = cur->next = new_token(TK_PUNCT, p, p + 1);
			p++;
			continue;
		}

		error_at(p, "invalid token");
	}

	cur = cur->next = new_token(TK_EOF, p, p);
	return head.next;
}

static bool equal(Token *tok, char *target)
{
	return memcmp(tok->loc, target, tok->len) == 0 && target[tok->len] == '\0';
}

static Token *skip(Token *tok, char *target)
{
	if (!equal(tok, target))
		error_tok(tok, "invalid token\n");
	return tok->next;
}

static int get_number(Token *tok)
{
	if (tok->kind != TK_NUM)
		error_tok(tok, "not a number\n");
	return tok->val;
}

typedef enum
{
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
	ND_NUM
} NodeKind;

typedef struct Node Node;
struct Node
{
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
};

static Node *new_node(NodeKind kind)
{
	Node *new = calloc(1, sizeof(Node));
	new->kind = kind;
	return new;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
	Node *node = new_node(kind);
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

static Node *new_num(int val)
{
	Node *node = new_node(ND_NUM);
	node->val = val;
	return node;
}

static Node *expr(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);

static Node *expr(Token **rest, Token *tok)
{
	Node *node = mul(&tok, tok);

	for (;;)
	{
		if (equal(tok, "+"))
		{
			node = new_binary(ND_ADD, node, mul(&tok, tok->next));
			continue;
		}

		if (equal(tok, "-"))
		{
			node = new_binary(ND_SUB, node, mul(&tok, tok->next));
			continue;
		}

		*rest = tok;
		return node;
	}
}

static Node *mul(Token **rest, Token *tok)
{
	Node *node = primary(&tok, tok);

	for (;;)
	{
		if (equal(tok, "*"))
		{
			node = new_binary(ND_MUL, node, primary(&tok, tok->next));
			continue;
		}

		if (equal(tok, "/"))
		{
			node = new_binary(ND_DIV, node, primary(&tok, tok->next));
			continue;
		}

		*rest = tok;
		return node;
	}
}

static Node *primary(Token **rest, Token *tok)
{

	if (equal(tok, "("))
	{
		Node *node = expr(&tok, tok->next);
		tok = skip(tok, ")");
		*rest = tok;
		return node;
	}

	if (tok->kind == TK_NUM)
	{
		Node *node = new_num(tok->val);
		*rest = tok->next;
		return node;
	}

	error_tok(tok, "invalid token\n");
}

static int depth = 0;

static void push(void)
{
	printf("	push %%rax\n");
	depth++;
	return;
}

static void pop(char *argument)
{
	printf("	pop %%%s\n", argument);
	depth--;
	return;
}

static void gen_expr(Node *node)
{
	if (node->kind == ND_NUM)
	{
		printf("	mov $%d, %%rax\n", node->val);
		return;
	}

	gen_expr(node->rhs);
	push();
	gen_expr(node->lhs);
	pop("rdi");

	switch (node->kind)
	{
	case ND_ADD:
		printf("	add %%rdi, %%rax\n");
		return;
	case ND_SUB:
		printf("	sub %%rdi, %%rax\n");
		return;
	case ND_MUL:
		printf("	imul %%rdi, %%rax\n");
		return;
	case ND_DIV:
		printf("	cqo\n");
		printf("	idiv %%rdi, %%rax\n");
		return;
	}

	error("error generate\n");
}

int main(int argc, char **argv)
{
	if (argc != 2)
		error("%s: invalid argument.\n", argv[0]);

	user_input = argv[1];

	Token *tok = tokenize();
	Node *node = expr(&tok, tok);

	printf(".globl main\n\n");
	printf("main:\n");

	gen_expr(node);

	printf("	ret\n");

	assert(depth == 0);
	return 0;
}