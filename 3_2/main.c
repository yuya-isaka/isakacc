#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

static void error(const char *fmt, ...)
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
	long val;
	char *loc;
	int len;
};

Token *new_token(TokenKind kind, char *start, char *end)
{
	Token *new = calloc(1, sizeof(Token));
	new->kind = kind;
	new->loc = start;
	new->len = end - start;
	return new;
}

Token *tokenize(char *p)
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

		if (*p == '+' || *p == '-')
		{
			cur = cur->next = new_token(TK_PUNCT, p, p + 1);
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

		error("ivalid argument.");
	}
	cur = cur->next = new_token(TK_EOF, p, p);
	return head.next;
}

// OK
static int get_number(Token *tok)
{
	if (tok->kind != TK_NUM)
		error("expected a number\n");
	return tok->val;
}

static bool equal(Token *tok, char *str)
{
	return memcmp(tok->loc, str, tok->len) == 0 && str[tok->len] == '\0';
}

static Token *skip(Token *tok, char *str)
{
	if (!equal(tok, str))
		error("expected %s\n", str);
	return tok->next;
}

int main(int argc, char **argv)
{
	if (argc != 2)
		error("%s: invalid argument num.\n", argv[0]);

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