#include "header.h"

static Obj *globals;

static Type *declspec(Token **rest, Token *tok) {
  if (equal(tok, "int")) {
    *rest = tok->next;
    return ty_int;
  }
  if (equal(tok, "char")) {
    *rest = tok->next;
    return ty_char;
  }

  error_tok(tok, "error declspec");
}

static char *get_ident(Token *tok) {
  if (tok->kind != TK_IDENT)
    error_tok(tok, "error get_ident");
  return strndup(tok->loc, tok->len);
}

static Type *declarator(Token **rest, Token *tok, Type *basety);

static Type *func_params(Token **rest, Token *tok, Type *basety) {
  Type head = {};
  Type *cur = &head;

  while (!equal(tok, ")")) {
    if (cur != &head)
      tok = skip(tok, ",");
    Type *ty = declspec(&tok, tok);
    ty = declarator(&tok, tok, ty);
    cur = cur->next = copy_ty(ty);
  }

  *rest = skip(tok, ")");

  Type *ty = func_type(basety);
  ty->params = head.next;
  return ty;
}

static int get_num(Token *tok) {
  if (tok->kind != TK_NUM)
    error_tok(tok, "error get_num");
  return tok->val;
}

static Type *type_suffix(Token **rest, Token *tok, Type *basety) {
  if (equal(tok, "(")) {
    return func_params(rest, tok->next, basety);
  }

  if (equal(tok, "[")) {
    int len = get_num(tok->next);
    tok = skip(tok->next->next, "]");
    Type *ty = type_suffix(&tok, tok, basety);
    return array_of(ty, len);
  }

  *rest = tok;
  return basety;
}

static Type *declarator(Token **rest, Token *tok, Type *basety) {
  while (consume(&tok, tok, "*"))
    basety = pointer_to(basety);

  if (tok->kind != TK_IDENT)
    error_tok(tok, "error declarator");

  Type *ty = type_suffix(rest, tok->next, basety);
  ty->name = tok;
  return ty;
}

static bool is_function(Token *tok) {
  if (equal(tok->next, ";"))
    return false;

  Type dummy = {};
  Type *ty = declarator(&tok, tok, &dummy);
  return tok->kind == TY_FUNC;
}

static Obj *new_var(char *name, Type *ty) {
  Obj *var = calloc(1, sizeof(Obj));
  var->name = name;
  var->ty = ty;
  return var;
}

static Obj *new_gvar(char *name, Type *ty) { Obj *var = new_var(name, ty); }

static Token *function(Token *tok, Type *basety) { return tok; }

static Token *global_variable(Token *tok, Type *basety) {
  bool first = true;
  while (!equal(tok, ";")) {
    if (!first)
      tok = skip(tok, ",");
    first = false;

    Type *ty = declarator(&tok, tok, basety);
    new_gvar(get_ident(ty->name), ty);
    // ここまで
  }
}

Obj *parse(Token *tok) {
  globals = NULL;

  while (tok->kind != TK_EOF) {
    Type *basety = declspec(&tok, tok);

    if (is_function(tok)) {
      tok = function(tok, basety);
      continue;
    }

    tok = global_variable(tok, basety);
  }

  return globals;
}