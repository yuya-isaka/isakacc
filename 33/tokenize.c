#include "header.h"

static char *cu_file_name;
static char *cu_user_input;

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}

static void verror_at(char *at, char *fmt, va_list ap) {
  char *line = at;
  while (cu_user_input < line && line[-1] != '\n')
    line--;

  char *end = at;
  while (*end != '\n')
    end++;

  int line_no = 1;
  for (char *now = cu_user_input; now < line; now++)
    if (*now == '\n')
      line_no++;

  int indent = fprintf(stderr, "%s: %d: ", cu_file_name, line_no);
  fprintf(stderr, "%.*s\n", (int)(end - line), line);

  int pos = at - line + indent;

  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^  ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}

void error_at(char *at, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(at, fmt, ap);
}

void error_tok(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  verror_at(tok->loc, fmt, ap);
}

static bool start_with(char *p, char *target) {
  return strncmp(p, target, strlen(target)) == 0;
}

static Token *new_token(TokenKind kind, char *start, char *end) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->loc = start;
  tok->len = end - start;
  return tok;
}

static char *string_literal_end(char *p) {
  char *start = p;
  for (; *p != '"'; p++) {
    if (*p == '\n' || *p == '\0')
      error_at(start, "error string_literal end");
    if (*p == '\\')
      p++;
  }
  return p;
}

static int from_hex(char *p) {
  if ('0' <= *p && *p <= '9')
    return *p - '0';
  if ('a' <= *p && *p <= 'f')
    return *p - 'a' + 10;
  return *p - 'A' + 10;
}

static int read_escaped_char(char **rest, char *p) {
  if ('0' <= *p && *p <= '7') {
    int c = *p++ - '0';
    if ('0' <= *p && *p <= '7') {
      c = (c << 3) + (*p++ - '0');
      if ('0' <= *p && *p <= '7')
        c = (c << 3) + (*p++ - '0');
    }
    *rest = p;
    return c;
  }

  if (!strcmp(p, "x")) {
    p++;
    if (!isxdigit(*p))
      error("invalid x digit");

    int c = 0;
    while (isxdigit(*p))
      c = (c << 4) + from_hex(p++);

    *rest = p;
    return c;
  }

  *rest = p + 1;

  switch (*p) {
  case 'a':
    return '\a';
  case 'b':
    return '\b';
  case 't':
    return '\t';
  case 'n':
    return '\n';
  case 'f':
    return '\f';
  case 'r':
    return '\r';
  case 'v':
    return '\v';
  case 'e':
    return 27;
  default:
    return *p;
  }
}

static Token *read_string_literal(char *start) {
  char *end = string_literal_end(start + 1);
  char *buf = calloc(1, end - start);
  int len = 0;

  for (char *p = start + 1; p < end;) {
    if (*p == '\\')
      buf[len++] = read_escaped_char(&p, p + 1);
    else
      buf[len++] = *p++;
  }

  Token *tok = new_token(TK_STR, start, end + 1);
  tok->ty = array_of(ty_char, len + 1);
  tok->str = buf;
  return tok;
}

static bool is_ident1(char name) {
  return ('a' <= name && name <= 'z') || ('A' <= name && name <= 'Z') ||
         (name == '_');
}

static bool is_ident2(char name) { return is_ident1(name) || isdigit(name); }

static int read_punct(char *p) {
  if (start_with(p, "==") || start_with(p, "!=") || start_with(p, "<=") ||
      start_with(p, ">="))
    return 2;

  return ispunct(*p) ? 1 : 0;
}

bool equal(Token *tok, char *target) {
  return memcmp(tok->loc, target, tok->len) == 0 && strlen(target) == tok->len;
}

static bool is_keyword(Token *tok) {
  static char *kw[] = {"return", "if",  "else", "while",
                       "for",    "int", "char", "sizeof"};
  for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++)
    if (equal(tok, kw[i]))
      return true;
  return false;
}

static void convert_keywords(Token *tok) {
  for (Token *cur = tok; cur; cur = cur->next)
    if (is_keyword(cur))
      cur->kind = TK_KEYWORD;
}

static Token *tokenize(char *file_name, char *p) {
  cu_file_name = file_name;
  cu_user_input = p;

  Token head = {};
  Token *cur = &head;

  while (*p) {
    if (start_with(p, "//")) {
      p += 2;
      while (*p != '\n')
        p++;
      continue;
    }

    if (start_with(p, "/*")) {
      char *q = strstr(p + 2, "*/");
      if (!q)
        error_at(p, "error nothing */");
      p = q + 2;
      continue;
    }

    if (isspace(*p)) {
      p++;
      continue;
    }

    if (isdigit(*p)) {
      char *start = p;
      cur = cur->next = new_token(TK_NUM, p, p);
      cur->val = strtoul(p, &p, 10);
      cur->len = p - start;
      continue;
    }

    if (*p == '"') {
      cur = cur->next = read_string_literal(p);
      p += cur->len;
      continue;
    }

    if (is_ident1(*p)) {
      char *start = p;
      do {
        p++;
      } while (is_ident2(*p));
      cur = cur->next = new_token(TK_IDNET, start, p);
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
  convert_keywords(head.next);
  return head.next;
}

static char *read_file(char *path) {
  FILE *fp;

  if (!strcmp(path, "-")) {
    fp = stdin;
  } else {
    fp = fopen(path, "r");
    if (!fp)
      error("cannot open file %s: %s", path, strerror(errno));
  }

  char *buf;
  size_t buflen;
  FILE *out = open_memstream(&buf, &buflen);

  for (;;) {
    char buf2[4096];
    int n = fread(buf2, 1, sizeof(buf2), fp);
    if (n == 0)
      break;
    fwrite(buf2, 1, n, out);
  }

  if (fp != stdin)
    fclose(fp);

  fflush(out);
  if (buflen == 0 || buf[buflen - 1] != '\n')
    fputc('\n', out);
  fputc('\0', out);
  fclose(out);

  return buf;
}

Token *tokenize_file(char *input_path) {
  return tokenize(input_path, read_file(input_path));
}