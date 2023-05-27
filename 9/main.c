#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
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
	fprintf(stderr, "^ ");
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

static bool starts_with(char *p, char *target)
{
	return strncmp(p, target, strlen(target)) == 0;
}

static int read_punct(char *p)
{
	if (starts_with(p, "==") || starts_with(p, "!=") || starts_with(p, "<=") || starts_with(p, ">="))
		return 2;

	return ispunct(*p) ? 1 : 0;
}

Token *tokenize(char *p)
{
	user_input = p;

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
			cur->val = strtoul(p, &p, 10);
			cur->len = p - tmp;
			continue;
		}

		int punct_len = read_punct(p);
		if (punct_len)
		{
			cur = cur->next = new_token(TK_PUNCT, p, p + punct_len);
			p += punct_len;
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
	ND_NEG,
	ND_NUM,
	ND_EQ,
	ND_NE,
	ND_LT,
	ND_LE,
	ND_EXPR_STMT
} NodeKind;

typedef struct Node Node;
struct Node
{
	NodeKind kind;
	Node *next;
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

static Node *new_unary(NodeKind kind, Node *lhs)
{
	Node *node = new_node(kind);
	node->lhs = lhs;
	return node;
}

static Node *new_num(int val)
{
	Node *node = new_node(ND_NUM);
	node->val = val;
	return node;
}

static Node *expr(Token **rest, Token *tok);
static Node *stmt(Token **rest, Token *tok);
static Node *equivalent(Token **rest, Token *tok);
static Node *relation(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *unary(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);

static Node *expr(Token **rest, Token *tok)
{
	return stmt(rest, tok);
}

static Node *stmt(Token **rest, Token *tok)
{
	Node *node = new_unary(ND_EXPR_STMT, equivalent(&tok, tok));
	*rest = skip(tok, ";");
	return node;
}

static Node *equivalent(Token **rest, Token *tok)
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
		Node *node = equivalent(&tok, tok->next);
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

Node *parse(Token *tok)
{
	Node head = {};
	Node *cur = &head;
	while (tok->kind != TK_EOF)
		cur = cur->next = expr(&tok, tok);
	return head.next;
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
	case ND_EXPR_STMT:
		gen_expr(node->lhs);
		return;
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
		printf("	idiv %%rdi\n");
		return;
	case ND_EQ:
	case ND_NE:
	case ND_LT:
	case ND_LE:
		printf("	cmp %%rdi, %%rax\n");

		if (node->kind == ND_EQ)
		{
			printf("	sete %%al\n");
		}
		else if (node->kind == ND_NE)
		{
			printf("	setne %%al\n");
		}
		else if (node->kind == ND_LT)
		{
			printf("	setl %%al\n");
		}
		else
		{
			printf("	setle %%al\n");
		}

		printf("	movzb %%al, %%rax\n");
		return;
	}
}

void code_gen(Node *node)
{
	printf(".globl main\n\n");
	printf("main:\n");

	for (Node *cur = node; cur; cur = cur->next)
	{
		gen_expr(cur);
		assert(depth == 0);
	}

	printf("	ret\n");
}

int main(int argc, char **argv)
{
	if (argc != 2)
		error("%s: invalid argument\n", argv[0]);

	Token *tok = tokenize(argv[1]);
	Node *node = parse(tok);
	code_gen(node);

	return 0;
}