#include "header.h"

Type *ty_int = &(Type){TY_INT, 8};
Type *ty_char = &(Type){TY_CHAR, 1};

Type *array_of(Type *base, int len) {
  Type *ty = calloc(1, sizeof(Type));
  ty->kind = TY_ARRAY;
  ty->base = base;
  ty->size = base->size * len;
  ty->array_len = len;
  return ty;
}