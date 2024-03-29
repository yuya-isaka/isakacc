#include "header.h"

static Obj *globals;
static Obj *locals;

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

static Type *declarator(Token **rest, Token *tok, Type *ty);

static Type *func_params(Token **rest, Token *tok, Type *base) {
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

  Type *ty = type_func(base);
  ty->params = head.next;
  return ty;
}

static int get_num(Token *tok) {
  if (tok->kind != TK_NUM)
    error_tok(tok, "error get_num");
  return tok->val;
}

static Type *type_suffix(Token **rest, Token *tok, Type *base) {
  if (equal(tok, "(")) {
    return func_params(rest, tok->next, base);
  }

  if (equal(tok, "[")) {
    int len = get_num(tok->next);
    tok = skip(tok->next->next, "]");
    Type *ty = type_suffix(rest, tok, base);
    return array_of(ty, len);

    // ↓
    // 配列の順序が入れ替わる．1次元目に入るのが値で，二次元目に入るのが配列になってしまう
    // Type *ty = array_of(ty, len);
    // return type_suffix(rest, tok, ty);
  }

  *rest = tok;
  return base;
}

static Type *declarator(Token **rest, Token *tok, Type *ty) {
  while (consume(&tok, tok, "*"))
    ty = pointer_to(ty);

  if (tok->kind != TK_IDENT)
    error_tok(tok, "error declaration");

  ty = type_suffix(rest, tok->next, ty);
  ty->name = tok;
  return ty;
}

static bool is_function(Token *tok) {
  if (equal(tok->next, ";"))
    return false;

  Type dummy = {};
  Type *ty = declarator(&tok, tok, &dummy);
  return ty->kind == TY_FUNC;
}

static Obj *new_var(char *name, Type *ty) {
  Obj *var = calloc(1, sizeof(Obj));
  var->name = name;
  var->ty = ty;
  return var;
}

static Obj *new_gvar(char *name, Type *ty) {
  Obj *var = new_var(name, ty);
  var->next = globals;
  globals = var;
  return var;
}

static Obj *new_lvar(char *name, Type *ty) {
  Obj *var = new_var(name, ty);
  var->is_local = true;
  var->next = locals;
  locals = var;
  return var;
}

static char *get_ident(Token *tok) {
  if (tok->kind != TK_IDENT)
    error_tok(tok, "error get_ident");
  return strndup(tok->loc, tok->len);
}

static void create_lvar_params(Type *ty) {
  if (ty) {
    create_lvar_params(ty->next);
    new_lvar(get_ident(ty->name), ty);
  }
}

static bool is_type(Token *tok) {
  return equal(tok, "int") || equal(tok, "char");
}

static Node *new_node(NodeKind kind, Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->tok = tok;
  return node;
}

static Node *new_var_node(Obj *var, Token *tok) {
  Node *node = new_node(ND_VAR, tok);
  node->var = var;
  return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok) {
  Node *node = new_node(kind, tok);
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

static Node *new_unary(NodeKind kind, Node *lhs, Token *tok) {
  Node *node = new_node(kind, tok);
  node->lhs = lhs;
  return node;
}

static Node *new_num(int val, Token *tok) {
  Node *node = new_node(ND_NUM, tok);
  node->val = val;
  return node;
}

static Node *declaration(Token **rest, Token *tok);
static Node *stmt(Token **rest, Token *tok);
static Node *compound_stmt(Token **rest, Token *tok);
static Node *expr_stmt(Token **rest, Token *tok);
static Node *expr(Token **rest, Token *tok);
static Node *assign(Token **rest, Token *tok);
static Node *equivalent(Token **rest, Token *tok);
static Node *relation(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *unary(Token **rest, Token *tok);
static Node *postfix(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);

static Node *declaration(Token **rest, Token *tok) {
  Type *base = declspec(&tok, tok);

  Node head = {};
  Node *cur = &head;

  bool first = true;
  while (!equal(tok, ";")) {
    if (!first)
      tok = skip(tok, ",");
    first = false;

    Type *ty = declarator(&tok, tok, base);
    Obj *var = new_lvar(get_ident(ty->name), ty);

    if (!consume(&tok, tok, "="))
      continue;

    Node *lhs = new_var_node(var, tok);
    Node *rhs = assign(&tok, tok);
    Node *node = new_binary(ND_ASSIGN, lhs, rhs, tok);
    cur = cur->next = new_unary(ND_EXPR_STMT, node, tok);
  }

  *rest = skip(tok, ";");

  Node *node = new_node(ND_BLOCK, tok);
  node->body = head.next;
  return node;
}

static Node *stmt(Token **rest, Token *tok) {
  if (equal(tok, "return")) {
    Node *node = new_node(ND_RETURN, tok);
    node->lhs = expr(&tok, tok->next);
    *rest = skip(tok, ";");
    return node;
  }

  if (equal(tok, "{"))
    return compound_stmt(rest, tok->next);

  if (equal(tok, "if")) {
    Node *node = new_node(ND_IF, tok);
    tok = skip(tok->next, "(");
    node->cond = expr(&tok, tok);
    tok = skip(tok, ")");
    node->then = stmt(&tok, tok);
    if (equal(tok, "else"))
      node->els = stmt(&tok, tok);
    *rest = tok;
    return node;
  }

  if (equal(tok, "for")) {
    Node *node = new_node(ND_FOR, tok);
    tok = skip(tok->next, "(");
    node->init = expr_stmt(&tok, tok);

    if (!equal(tok, ";"))
      node->cond = expr(&tok, tok);
    tok = skip(tok, ";");

    if (!equal(tok, ")"))
      node->inc = expr(&tok, tok);
    tok = skip(tok, ")");

    node->then = stmt(rest, tok);
    return node;
  }

  if (equal(tok, "while")) {
    Node *node = new_node(ND_FOR, tok);
    tok = skip(tok->next, "(");
    node->cond = expr(&tok, tok);
    tok = skip(tok, ")");
    node->then = stmt(rest, tok);
    return node;
  }

  return expr_stmt(rest, tok);
}

static Node *compound_stmt(Token **rest, Token *tok) {
  Node head = {};
  Node *cur = &head;

  while (!equal(tok, "}")) {
    if (is_type(tok))
      cur = cur->next = declaration(&tok, tok);
    else
      cur = cur->next = stmt(&tok, tok);
    add_type(cur);
  }

  *rest = skip(tok, "}");

  Node *node = new_node(ND_BLOCK, tok);
  node->body = head.next;
  return node;
}

static Node *expr_stmt(Token **rest, Token *tok) {
  if (equal(tok, ";")) {
    *rest = tok->next;
    return new_node(ND_BLOCK, tok);
  }

  Node *node = new_node(ND_EXPR_STMT, tok);
  node->lhs = expr(&tok, tok);
  *rest = skip(tok, ";");
  return node;
}
static Node *expr(Token **rest, Token *tok) { return assign(rest, tok); }

static Node *assign(Token **rest, Token *tok) {
  Node *node = equivalent(&tok, tok);

  if (equal(tok, "="))
    return new_binary(ND_ASSIGN, node, assign(rest, tok->next), tok);

  *rest = tok;
  return node;
}

static Node *equivalent(Token **rest, Token *tok) {
  Node *node = relation(&tok, tok);

  for (;;) {
    Token *start = tok;

    if (equal(tok, "==")) {
      node = new_binary(ND_EQ, node, relation(&tok, tok->next), start);
      continue;
    }
    if (equal(tok, "!=")) {
      node = new_binary(ND_NE, node, relation(&tok, tok->next), start);
      continue;
    }

    *rest = tok;
    return node;
  }
}

static Node *relation(Token **rest, Token *tok) {
  Node *node = add(&tok, tok);

  for (;;) {
    Token *start = tok;

    if (equal(tok, "<")) {
      node = new_binary(ND_LT, node, add(&tok, tok->next), start);
      continue;
    }
    if (equal(tok, "<=")) {
      node = new_binary(ND_LE, node, add(&tok, tok->next), start);
      continue;
    }
    if (equal(tok, ">")) {
      node = new_binary(ND_LT, add(&tok, tok->next), node, start);
      continue;
    }
    if (equal(tok, ">=")) {
      node = new_binary(ND_LE, add(&tok, tok->next), node, start);
      continue;
    }

    *rest = tok;
    return node;
  }
}

static Node *new_add(Node *lhs, Node *rhs, Token *tok) {
  add_type(lhs);
  add_type(rhs);

  if (is_integer(lhs->ty) && is_integer(rhs->ty))
    return new_binary(ND_ADD, lhs, rhs, tok);

  if (lhs->ty->base && rhs->ty->base)
    error_tok(tok, "error new_add");

  if (is_integer(lhs->ty) && rhs->ty->base) {
    Node *tmp = lhs;
    lhs = rhs;
    rhs = tmp;
  }

  // sizeを間違えそうになってた
  rhs = new_binary(ND_MUL, rhs, new_num(lhs->ty->base->size, tok), tok);
  return new_binary(ND_ADD, lhs, rhs, tok);
}

static Node *new_sub(Node *lhs, Node *rhs, Token *tok) {
  add_type(lhs);
  add_type(rhs);

  if (is_integer(lhs->ty) && is_integer(rhs->ty))
    return new_binary(ND_SUB, lhs, rhs, tok);

  if (lhs->ty->base && rhs->ty->base) {
    Node *node = new_binary(ND_SUB, lhs, rhs, tok);
    node->ty = ty_int;
    return new_binary(ND_DIV, node, new_num(lhs->ty->base->size, tok), tok);
  }

  if (is_integer(rhs->ty) && lhs->ty->base) {
    rhs = new_binary(ND_MUL, rhs, new_num(lhs->ty->size, tok), tok);
    return new_binary(ND_SUB, lhs, rhs, tok);
  }

  error_tok(tok, "error new_sub");
}

static Node *add(Token **rest, Token *tok) {
  Node *node = mul(&tok, tok);

  for (;;) {
    Token *start = tok;

    if (equal(tok, "+")) {
      node = new_add(node, mul(&tok, tok->next), start);
      continue;
    }
    if (equal(tok, "-")) {
      node = new_sub(node, mul(&tok, tok->next), start);
      continue;
    }

    *rest = tok;
    return node;
  }
}

static Node *mul(Token **rest, Token *tok) {
  Node *node = unary(&tok, tok);

  for (;;) {
    Token *start = tok;

    if (equal(tok, "*")) {
      node = new_binary(ND_MUL, node, unary(&tok, tok->next), start);
      continue;
    }
    if (equal(tok, "/")) {
      node = new_binary(ND_DIV, node, unary(&tok, tok->next), start);
      continue;
    }

    *rest = tok;
    return node;
  }
}

static Node *unary(Token **rest, Token *tok) {
  if (equal(tok, "+"))
    return unary(rest, tok->next);
  if (equal(tok, "-"))
    return new_unary(ND_NEG, unary(rest, tok->next), tok);
  if (equal(tok, "&"))
    return new_unary(ND_ADDR, unary(rest, tok->next), tok);
  if (equal(tok, "*"))
    return new_unary(ND_DEREF, unary(rest, tok->next), tok);

  postfix(rest, tok);
}

static Node *postfix(Token **rest, Token *tok) {
  Node *node = primary(&tok, tok);

  while (equal(tok, "[")) {
    Token *start = tok;
    Node *rhs = expr(&tok, tok->next);
    tok = skip(tok, "]");
    node = new_unary(ND_DEREF, new_add(node, rhs, start), start);
  }

  *rest = tok;
  return node;
}

static char *temp_name(void) {
  static int i = 0;
  return format(".L..%d", i++);
  // ちょっとわかってなかった（けどできた，emit_data)
}

static Obj *new_anon_gvar(Type *ty) { return new_gvar(temp_name(), ty); }

static Obj *new_string_literal(char *literal, Type *ty) {
  Obj *var = new_anon_gvar(ty);
  var->init_data = literal;
  return var;
}

static Node *run_func(Token **rest, Token *tok) {
  Node *node = new_node(ND_FUNCALL, tok);
  node->funcname = get_ident(tok);

  tok = tok->next->next;

  Node head = {};
  Node *cur = &head;

  while (!equal(tok, ")")) {
    if (cur != &head)
      tok = skip(tok, ",");
    cur = cur->next = assign(&tok, tok);
  }

  *rest = skip(tok, ")");

  node->args = head.next;
  return node;
}

static Obj *find_lvar(Token *tok) {
  for (Obj *cur = locals; cur; cur = cur->next)
    if (equal(tok, cur->name))
      return cur;

  for (Obj *cur = globals; cur; cur = cur->next)
    if (equal(tok, cur->name))
      return cur;

  error_tok(tok, "error find_lvar");
}

static Node *primary(Token **rest, Token *tok) {
  if (equal(tok, "(")) {
    Node *node = expr(&tok, tok->next);
    *rest = skip(tok, ")");
    return node;
  }

  if (tok->kind == TK_NUM) {
    Node *node = new_num(tok->val, tok);
    *rest = tok->next;
    return node;
  }

  if (tok->kind == TK_STR) {
    Obj *var = new_string_literal(tok->str, tok->ty);
    *rest = tok->next;
    return new_var_node(var, tok); // わかってなかた(けどできた)

    // ここは変数のvarで返さないといけない
    // なぜかというと，

    // これだとerror nd_derefでエラー
    // つまり，「Derefの先はポインタじゃないとだめ（つまり生ポインタか配列じゃないとダメ）」
    // でもここでのnew_var_nodeは「varの配列」では？
    // だけどこ，varの配列のbaseのタイプを持ったderefが返される
    // なので，"aaa"[0]のpost_fixをやるときに，new_addのところでは，ただのbaseになってる
    // つまり配列として扱われない
    // これをやるべきなのは文字列リテラルではなく，文字に対してはこの処理をする
    // 文字列リテラルは配列でアクセスできる必要がある．
    // []でアクセスできるのは，その左側のタイプが配列の時のみ（正確にはポインタの時のみ）
    // return new_unary(ND_DEREF, new_var_node(var, tok), tok);
  }

  if (tok->kind == TK_IDENT) {
    if (equal(tok->next, "("))
      return run_func(rest, tok);

    Obj *var = find_lvar(tok);
    if (!var)
      error_tok(tok, "error find_lvar");
    *rest = tok->next;
    return new_var_node(var, tok);
  }

  // 忘れがち
  if (equal(tok, "sizeof")) {
    Node *node = unary(rest, tok->next);
    add_type(node);
    return new_num(node->ty->size, tok);
  }

  error_tok(tok, "error primary: %s", strndup(tok->loc, tok->len));
}

static Token *function(Token *tok, Type *ty) {
  ty = declarator(&tok, tok, ty);

  Obj *fn = new_gvar(get_ident(ty->name), ty);
  fn->is_func = true;

  locals = NULL;

  create_lvar_params(ty->params);
  fn->params = locals;

  tok = skip(tok, "{");
  fn->body = compound_stmt(&tok, tok);
  fn->locals = locals;

  return tok;
}

static Token *global_variable(Token *tok, Type *ty) {
  bool first = true;
  while (!consume(&tok, tok, ";")) {
    if (!first)
      tok = skip(tok, ",");
    first = false;

    ty = declarator(&tok, tok, ty);
    new_gvar(get_ident(ty->name), ty);
  }

  return tok;
}

Obj *parse(Token *tok) {
  globals = NULL;

  while (tok->kind != TK_EOF) {
    Type *ty = declspec(&tok, tok);

    if (is_function(tok)) {
      tok = function(tok, ty);
      continue;
    }

    tok = global_variable(tok, ty);
  }

  return globals;
}