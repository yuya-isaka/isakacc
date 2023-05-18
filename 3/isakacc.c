#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

void error(const char *fmt, ...)
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
	TK_RESERVED,
	TK_NUM,
	TK_EOF
} TokenKind;

typedef struct Token Token;

struct Token
{
	TokenKind kind;
	Token *next;
	long int val;
	char *str;
};

Token *new_token(TokenKind kind, Token *cur, char *str)
{
	Token *new = calloc(1, sizeof(Token));
	new->kind = kind;
	new->str = str;
	cur->next = new;
	return new;
}

Token *tok;

Token *tokenize(char *p)
{
	Token head;
	head.next = NULL;
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
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}

		if (isdigit(*p))
		{
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}

		error("%s: 知らない識別子です。", *p);
	}

	new_token(TK_EOF, cur, p);
	return head.next;
}

bool at_eof(Token *cur)
{
	return cur->kind == TK_EOF;
}

bool consume(char str)
{
	if (tok->kind != TK_RESERVED || tok->str[0] != str)
		return false;
	tok = tok->next;
	return true;
}

long int expect_number()
{
	if (tok->kind != TK_NUM)
		error("expected a number");
	long int val = tok->val;
	tok = tok->next;
	return val;
}

void expect(char str)
{
	if (tok->kind != TK_RESERVED || tok->str[0] != str)
		error("expected %c", str);
	tok = tok->next;
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		error("引数の個数が違います. %s", argv[0]);
		return 1;
	}

	tok = tokenize(argv[1]);

	printf(".intel_syntax noprefix\n");
	printf(".globl main\n\n");
	printf("main:\n");
	printf("	mov rax, %ld\n", expect_number());

	while (!at_eof(tok))
	{
		if (consume('+'))
		{
			printf("	add rax, %ld\n", expect_number());
			continue;
		}

		expect('-');
		printf("	sub rax, %ld\n", expect_number());
	}

	printf("	ret\n");
	return 0;
}