#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

char *user_input;

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

static void verror(char *loc, char *fmt, va_list ap)
{
	int space = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", space, "");
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(EXIT_FAILURE);
}

static void error_at(char *loc, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verror(loc, fmt, ap);
}

static void error_tok(Token *tok, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verror(tok->loc, fmt, ap);
}

static Token *new_token(TokenKind kind, char *start, char *end)
{
	Token *new = calloc(1, sizeof(Token));
	new->kind = kind;
	new->loc = start;
	new->len = end - start;

	return new;
}

static Token *tokenize(void)
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

		if (*p == '-' || *p == '+')
		{
			cur = cur->next = new_token(TK_PUNCT, p, p + 1);
			p++;
			continue;
		}

		error_at(p, "知らん識別子です。\n");
	}

	cur = cur->next = new_token(TK_EOF, p, p);
	return head.next;
}

static int get_number(Token *tok)
{
	if (tok->kind != TK_NUM)
		error_tok(tok, "Not a number.");

	return tok->val;
}

static bool equal(Token *tok, char *target)
{
	return memcmp(tok->loc, target, tok->len) == 0 && target[tok->len] == '\0';
}

static Token *skip(Token *tok, char *target)
{
	if (!equal(tok, target))
		error_tok(tok, "expected %s", target);
	return tok->next;
}

int main(int argc, char **argv)
{
	if (argc != 2)
		error("%s: 引数の個数が違います。", argv[1]);

	user_input = argv[1];

	Token *tok = tokenize();

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