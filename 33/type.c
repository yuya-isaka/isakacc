#include "header.h"

Type *ty_int = &(Type){TY_INT, 8};
Type *ty_char = &(Type){TY_CHAR, 1};

bool is_integer(Type *ty) {
  return (ty->kind == TY_INT) || (ty->kind == TY_CHAR);
}

static Type *new_type(TypeKind kind) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = kind;
  return ty;
}

Type *array_of(Type *base, int len) {
  Type *ty = new_type(TY_ARRAY);
  ty->base = base;
  ty->size = base->size * len;
  ty->array_len = len;
  return ty;
}

Type *pointer_to(Type *base) {
  Type *ty = new_type(TY_PTR);
  ty->base = base;
  ty->size = 8;
  return ty;
}

Type *copy_ty(Type *ty) {
  Type *ret = calloc(1, sizeof(Type));
  *ret = *ty;
  return ret;
}

Type *func_type(Type *return_ty) {
  Type *ty = new_type(TY_FUNC);
  ty->return_ty = return_ty;
  return ty;
}

void add_type(Node *node) {
  if (!node || node->ty)
    return;

  add_type(node->lhs);
  add_type(node->rhs);
  add_type(node->cond);
  add_type(node->then);
  add_type(node->els);
  add_type(node->init);
  add_type(node->inc);
  for (Node *cur = node->body; cur; cur = cur->next)
    add_type(cur);
  for (Node *cur = node->args; cur; cur = cur->next)
    add_type(cur);

  switch (node->kind) {
  case ND_ADD:
  case ND_SUB:
  case ND_MUL:
  case ND_DIV:
  case ND_NEG:
    node->ty = node->lhs->ty;
    return;
  case ND_ASSIGN:
    if (node->lhs->ty->kind == TY_ARRAY)
      error_tok(node->tok, "error nd_assign");
    node->ty = node->lhs->ty;
    return;
  case ND_NUM:
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
  case ND_FUNCALL:
    node->ty = ty_int;
    return;
  case ND_VAR:
    node->ty = node->var->ty;
    return;
  case ND_ADDR:
    if (node->lhs->ty->kind == TY_ARRAY)
      node->ty = pointer_to(node->lhs->ty->base);
    else
      node->ty = pointer_to(node->lhs->ty);
    return;
  case ND_DEREF:
    if (!node->lhs->ty->base)
      error_tok(node->tok, "error nd_deref");
    node->ty = node->lhs->ty->base;
    return;
  case ND_STMT_EXPR: {
    if (node->body) {
      Node *stmt = node->body;
      while (stmt->next)
        stmt = stmt->next;

      if (stmt->kind == ND_EXPR_STMT) {
        node->ty = stmt->lhs->ty;
        return;
      }
    }
    error_tok(node->tok, "error nd_stmt_expr");
  }
  }
}