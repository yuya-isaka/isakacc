#include "header.h"

Type *ty_int = &(Type){TY_INT, 8};

bool is_integer(Type *ty) { return ty->kind == TY_INT; }

Type *copy_ty(Type *ty) {
  Type *ret = calloc(1, sizeof(Type));
  *ret = *ty;
  return ret;
}

Type *pointer_to(Type *base) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_PTR;
  ty->base = base;
  ty->size = 8;
  return ty;
}

Type *type_func(Type *return_ty) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_FUNC;
  ty->return_ty = return_ty;
  return ty;
}

Type *array_of(Type *base, int size) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_ARRAY;
  ty->size = base->size * size;
  ty->base = base;
  ty->array_len = size;
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
      error_tok(node->lhs->tok, "not an lvalue");
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
      node->ty =
          pointer_to(node->lhs->ty->base); // 先頭へのポインタになるから？
    else
      node->ty = pointer_to(node->lhs->ty);
    return;
  case ND_DEREF:
    if (!node->lhs->ty->base)
      error_tok(node->tok, "error deref");
    node->ty =
        node->lhs->ty
            ->base; // DEREFな時点で，そのbaseのtypeが設定される．なので，ASSIGNで呼び出す段階ではtypeはintになってるんか
    return;
  }
}