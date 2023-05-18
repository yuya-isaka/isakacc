#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

static void error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(EXIT_FAILURE);
}

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

static Token *new_token(TokenKind kind, char *start, char *end)
{
	Token *new = calloc(1, sizeof(Token));
	new->kind = kind;
	new->loc = start;
	new->len = end - start;
	return new;
}

static Token *tokenize(char *p)
{
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

		if (*p == '+' || *p == '-')
		{
			cur = cur->next = new_token(TK_PUNCT, p, p + 1);
			p++;
			continue;
		}
	}

	cur = cur->next = new_token(TK_EOF, p, p);
	return head.next;
}

static int get_number(Token *tok)
{
	if (tok->kind != TK_NUM)
		error("not number\n");
	return tok->val;
}

static bool equal(Token *tok, char *op)
{
	return memcmp(tok->loc, op, tok->len) == 0 && op[tok->len] == '\0';
}

static Token *skip(Token *tok, char *op)
{
	if (!equal(tok, op))
		error("expected %s\n", op);

	return tok->next;
}

int main(int argc, char **argv)
{
	if (argc != 2)
		error("%s invalid argument\n", argv[0]);

	Token *tok = tokenize(argv[1]);

	printf(".globl main\n\n");
	printf("main:\n");
	printf("	mov $%d, %%rax\n", get_number(tok));
	tok = tok->next;

	while (tok->kind != TK_EOF)
	{
		if (equal(tok, "+"))
		{
			printf("	add $%d, %%rax\n", get_number(tok->next));
			tok = tok->next->next;
			continue;
		}

		tok = skip(tok, "-");
		printf("	sub $%d, %%rax\n", get_number(tok));
		tok = tok->next;
	}

	printf("	ret\n");
	return 0;
}