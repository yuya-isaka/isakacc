#include "header.h"

static char *user_input;

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}

static void verror(char *at, char *fmt, va_list ap) {
  int len = at - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", len, "");
  fprintf(stderr, "^   ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}

void error_at(char *at, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror(at, fmt, ap);
}

void error_tok(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror(tok->loc, fmt, ap);
}

static Token *new_token(TokenKind kind, char *start, char *end) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->loc = start;
  tok->len = end - start;
  return tok;
}

static bool is_ident1(char name) {
  return ('a' <= name && name <= 'z') || ('A' <= name && name <= 'Z') ||
         (name == '_');
}

static bool is_ident2(char name) { return is_ident1(name) || isdigit(name); }

static bool start_with(char *p, char *target) {
  return strncmp(p, target, strlen(target)) == 0;
}

static int read_punct(char *p) {
  if (start_with(p, "==") || start_with(p, "!=") || start_with(p, "<=") ||
      start_with(p, ">="))
    return 2;

  return ispunct(*p) ? 1 : 0;
}

bool equal(Token *tok, char *p) {
  return memcmp(tok->loc, p, tok->len) == 0 && strlen(p) == tok->len;
}

bool consume(Token **rest, Token *tok, char *target) {
  if (equal(tok, target)) {
    *rest = tok->next;
    return true;
  }
  *rest = tok;
  return false;
}

Token *skip(Token *tok, char *target) {
  if (equal(tok, target)) {
    return tok->next;
  }
  error_tok(tok, "error skip");
}

static bool is_keyword(Token *tok) {
  static char *kw[] = {"return", "if",  "else",   "for",
                       "while",  "int", "sizeof", "char"};
  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
    if (equal(tok, kw[i]))
      return true;
  return false;
}

static void convert_keyword(Token *tok) {
  for (Token *cur = tok; cur->kind != TK_EOF; cur = cur->next)
    if (is_keyword(cur))
      cur->kind = TK_KEYWORD;
}

static int read_escaped_char(char *p) {
  switch (*p) {
  case 'a':
    return '\a';
  case 'b':
    return '\b';
  case 't':
    return '\t';
  case 'n':
    return '\n';
  case 'v':
    return '\v';
  case 'f':
    return '\f';
  case 'r':
    return '\r';
  case 'e':
    return 27;
  default:
    return *p;
  }
}

static char *string_literal_end(char *p) {
  char *start = p;
  for (; *p != '"'; p++) {
    if (*p == '\n' || *p == '\0')
      error_at(start, "error string_literal_end");
    if (*p == '\\')
      p++;
  }
  return p;
}

static Token *read_string_literal(char *start) {
  char *end = string_literal_end(start + 1);
  char *buf = calloc(1, end - start);
  int len = 0;

  for (char *p = start + 1; p < end;) {
    if (*p == '\\') {
      buf[len++] = read_escaped_char(p + 1);
      p += 2;
    } else {
      buf[len++] = *p++;
    }
  }

  Token *tok = new_token(TK_STR, start, end + 1);
  tok->ty = array_of(ty_char, len + 1);
  tok->str = buf;
  return tok;
}

Token *tokenize(char *p) {
  user_input = p;
  Token head = {};
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (isdigit(*p)) {
      cur = cur->next = new_token(TK_NUM, p, p);
      char *start = p;
      cur->val = strtoul(p, &p, 10);
      cur->len = p - start;
      continue;
    }

    if ('"' == *p) {
      cur = cur->next = read_string_literal(p);
      p += cur->len;
      continue;
    }

    if (is_ident1(*p)) {
      char *start = p;
      do {
        p++;
      } while (is_ident2(*p));
      cur = cur->next = new_token(TK_IDENT, start, p);
      continue;
    }

    int punct_len = read_punct(p);
    if (punct_len) {
      cur = cur->next = new_token(TK_PUNCT, p, p + punct_len);
      p += punct_len;
      continue;
    }

    error_at(p, "error tokenize");
  }

  cur = cur->next = new_token(TK_EOF, p, p);
  convert_keyword(head.next);
  return head.next;
}