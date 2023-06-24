#include "header.h"

Type *ty_int = &(Type){TY_INT};

bool is_integer(Type *ty)
{
	return ty->kind == TY_INT;
}

static Type *pointer_to(Type *ty)
{
	Type *type = calloc(1, sizeof(Type));
	type->kind = TY_PTR;
	type->base = ty;
	return type;
}

void add_type(Node *node)
{
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

	switch (node->kind)
	{
	case ND_ADD:
	case ND_SUB:
	case ND_MUL:
	case ND_DIV:
	case ND_ASSIGN:
	case ND_NEG:
		node->ty = node->lhs->ty;
		return;
	case ND_EQ:
	case ND_NE:
	case ND_LT:
	case ND_LE:
	case ND_VAR:
	case ND_NUM:
		node->ty = ty_int;
		return;
	case ND_ADDR:
		node->ty = pointer_to(node->lhs->ty);
		return;
	case ND_DEREF:
		if (node->lhs->ty->kind == TY_PTR)
			node->ty = node->lhs->ty->base;
		else
			node->ty = ty_int;
		return;
	}
}