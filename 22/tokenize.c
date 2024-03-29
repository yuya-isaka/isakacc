#include "header.h"

static char *user_input;

void error(char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(1);
}

static void verror(char *at, char *fmt, va_list ap)
{
	int len = at - user_input;

	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", len, "");
	fprintf(stderr, "^  ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(1);
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

static bool is_ident1(char name)
{
	return ('a' <= name && name <= 'z') || ('A' <= name && name <= 'Z') || name == '_';
}

static bool is_ident2(char name)
{
	return is_ident1(name) || isdigit(name);
}

static bool startwith(char *p, char *target)
{
	return strncmp(p, target, strlen(target)) == 0;
}

static int read_punct(char *p)
{
	if (startwith(p, "==") || startwith(p, "!=") || startwith(p, ">=") || startwith(p, "<="))
		return 2;

	return ispunct(*p) ? 1 : 0;
}

bool equal(Token *tok, char *target)
{
	return memcmp(tok->loc, target, tok->len) == 0 && strlen(target) == tok->len;
}

Token *skip(Token *tok, char *target)
{
	if (!equal(tok, target))
		error_tok(tok, "error skip: %s", strndup(tok->loc, tok->len));
	return tok->next;
}

bool consume(Token **rest, Token *tok, char *target)
{
	if (equal(tok, target))
	{
		*rest = tok->next;
		return true;
	}
	*rest = tok;
	return false;
}

static bool is_keyword(Token *tok)
{
	static char *kw[] = {"return", "if", "else", "for", "while", "int"};

	for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
		if (equal(tok, kw[i]))
			return true;

	return false;
}

static void convert_keywords(Token *tok)
{
	for (Token *cur = tok; cur->kind != TK_EOF; cur = cur->next)
		if (is_keyword(cur))
			cur->kind = TK_KEYWORD;
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

		if (is_ident1(*p))
		{
			char *tmp = p;
			do
			{
				p++;
			} while (is_ident2(*p));
			cur = cur->next = new_token(TK_IDENT, tmp, p);
			continue;
		}

		int punct_len = read_punct(p);
		if (punct_len)
		{
			cur = cur->next = new_token(TK_PUNCT, p, p + punct_len);
			p += punct_len;
			continue;
		}

		error_at(p, "error tokenize");
	}

	cur = cur->next = new_token(TK_EOF, p, p);
	convert_keywords(head.next);
	return head.next;
}