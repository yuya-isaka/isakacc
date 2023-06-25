#include "header.h"

Type *ty_int = &(Type){TY_INT};

bool is_integer(Type *ty)
{
	return ty->kind == TY_INT;
}

Type *pointer_to(Type *base)
{
	Type *ty = calloc(1, sizeof(Type));
	ty->kind = TY_PTR;
	ty->base = base;
	return ty;
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
	for (Node *n = node->body; n; n = n->next)
		add_type(n);

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
	case ND_NUM:
	case ND_EQ:
	case ND_NE:
	case ND_LT:
	case ND_LE:
		node->ty = ty_int;
		return;
	case ND_VAR:
		node->ty = node->var->ty;
		return;
	case ND_ADDR:
		// printf("aaa %s", strndup(node->lhs->tok->loc, node->lhs->tok->len));
		node->ty = pointer_to(node->lhs->ty);
		// printf("aaa %d %d", TY_PTR, node->ty->kind);
		return;
	case ND_DEREF:
		if (node->lhs->ty->kind != TY_PTR)
			error_tok(node->tok, "invalid deref %d %s", node->lhs->ty->kind, strndup(node->lhs->lhs->tok->loc, node->lhs->lhs->tok->len));
		node->