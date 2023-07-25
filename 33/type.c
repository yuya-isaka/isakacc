#include "header.h"

Type *ty_int = &(Type){TY_INT, 8};
Type *ty_char = &(Type){TY_CHAR, 1};

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