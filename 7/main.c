#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
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

static Token *new_token(TokenKind kind, char *start, char *end)
{
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->loc = start;
	tok->len = end - start;
	return tok;
}

static bool startswith(char *p, char *q)
{
	return strncmp(p, q, strlen(q)) == 0;
}

static int read_punct(char *p)
{
	if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">="))
		return 2;

	return ispunct(*p) ? 1 : 0;
}

static Token *tokenize(void)
{
	char *p = user_input;

	Token head;
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
			cur->val = strtoul(p, &p, 10);
			cur->len = p - tmp;
			continue;
		}

		int punctlen = read_punct(p);
		if (punctlen)
		{
			cur = cur->next = new_token(TK_PUNCT, p, p + punctlen);
			p += punctlen;
			continue;
		}

		error_at(p, "error tokenize\n");
	}

	cur = cur->next = new_token(TK_EOF, p, p);
	return head.next;
}

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

static bool equal(Token *tok, char *target)
{
	return memcmp(tok->loc, target, tok->len) == 0 && target[tok->len] == '\0';
}

static Token *skip(Token *tok, char *target)
{
	if (!equal(tok, target))
		error_tok(tok, "error skip\n");
	return tok->next;
}

static Node *new_node(NodeKind kind)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	return node;
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

static Node *new_unary(NodeKind kind, Node *lhs)
{
	Node *node = new_node(kind);
	node->lhs = lhs;
	return node;
}

static Node *expr(Token **rest, Token *tok);
static Node *equality(Token **rest, Token *tok);
static Node *relation(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *unary(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);

static Node *expr(Token **rest, Token *tok)
{
	return equality(rest, tok);
}

static Node *equality(Token **rest, Token *tok)
{
	Node *node = relation(&tok, tok);

	for (;;)
	{
		if (equal(tok, "=="))
		{
			node = new_binary(ND_EQ, node, relation(&tok, tok->next));
			continue;
		}

		if (equal(tok, "!="))
		{
			node = new_binary(ND_NE, node, relation(&tok, tok->next));
			continue;
		}

		*rest = tok;
		return node;
	}
}
static Node *relation(Token **rest, Token *tok)
{
	Node *node = add(&tok, tok);

	for (;;)
	{
		if (equal(tok, "<"))
		{
			node = new_binary(ND_LT, node, add(&tok, tok->next));
			continue;
		}
		if (equal(tok, "<="))
		{
			node = new_binary(ND_LE, node, add(&tok, tok->next));
			continue;
		}
		if (equal(tok, ">"))
		{
			node = new_binary(ND_LT, add(&tok, tok->next), node);
			continue;
		}
		if (equal(tok, ">="))
		{
			node = new_binary(ND_LE, add(&tok, tok->next), node);
			continue;
		}

		*rest = tok;
		return node;
	}
}
static Node *add(Token **rest, Token *tok)
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
	Node *node = unary(&tok, tok);

	for (;;)
	{
		if (equal(tok, "*"))
		{
			node = new_binary(ND_MUL, node, unary(&tok, tok->next));
			continue;
		}
		if (equal(tok, "/"))
		{
			node = new_binary(ND_DIV, node, unary(&tok, tok->next));
			continue;
		}

		*rest = tok;
		return node;
	}
}
static Node *unary(Token **rest, Token *tok)
{
	if (equal(tok, "+"))
		return unary(rest, tok->next);

	if (equal(tok, "-"))
		return new_unary(ND_NEG, unary(rest, tok->next));

	return primary(rest, tok);
}

static Node *primary(Token **rest, Token *tok)
{
	if (equal(tok, "("))
	{
		Node *node = expr(&tok, tok->next);
		*rest = skip(tok, ")");
		return node;
	}

	if (tok->kind == TK_NUM)
	{
		Node *node = new_num(tok->val);
		*rest = tok->next;
		return node;
	}

	error_tok(tok, "error primary\n");
}

static int depth = 0;

static void push(void)
{
	printf("	push %%rax\n");
	depth++;
	return;
}

static void pop(char *target)
{
	printf("	pop %%%s\n", target);
	depth--;
	return;
}

static void gen_expr(Node *node)
{
	switch (node->kind)
	{
	case ND_NEG:
		gen_expr(node->lhs);
		printf("	neg %%rax\n");
		return;
	case ND_NUM:
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
	case ND_EQ:
	case ND_NE:
	case ND_LT:
	case ND_LE:
		printf("	cmp %%rdi, %%rax\n");

		if (node->kind == ND_EQ)
			printf("	sete %%al\n");
		else if (node->kind == ND_NE)
			printf("	setne %%al\n");
		else if (node->kind == ND_LT)
			printf("	setl %%al\n");
		else
			printf("	setle %%al\n");

		printf("	movzb %%al, %%rax\n");
		return;
	}

	error("error gen_expr\n");
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

	if (tok->kind != TK_EOF)
		error_tok(tok, "invalid EOF\n");

	printf("	ret\n");
	assert(depth == 0);
	return 0;
}