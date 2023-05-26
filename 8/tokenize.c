#include "isakacc.h"

static char *user_input;

void error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(EXIT_FAILURE);
}

void verror(char *at, char *fmt, va_list ap)
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

void error_tok(Token *tok, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	verror(tok->loc, fmt, ap);
}

void error_at(char *at, char *fmt, ...)
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

static bool startswith(char *a, char *b)
{
	return strncmp(a, b, strlen(b)) == 0;
}

static int read_punct(char *p)
{
	if (startswith(p, "==") || startswith(p, "<=") || startswith(p, "!=") || startswith(p, ">="))
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