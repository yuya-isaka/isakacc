#include "isakacc.h"

static int depth = 0;

static void push(void)
{
	printf("	push %%rax\n");
	depth++;
	return;
}

static void pop(char *arg)
{
	printf("	pop %%%s\n", arg);
	depth--;
	return;
}

static void gen_expr(Node *node)
{
	switch (node->kind)
	{
	case ND_NEG:
		gen_expr(node->lhs);
		printf("	neg %%rax\n");
		return;
	case ND_NUM:
		printf("	mov $%d, %%rax\n", node->val);
		return;
	}

	gen_expr(node->rhs);
	push();
	gen_expr(node->lhs);
	pop("rdi");

	switch (node->kind)
	{
	case ND_ADD:
		printf("	add %%rdi, %%rax\n");
		return;
	case ND_SUB:
		printf("	sub %%rdi, %%rax\n");
		return;
	case ND_MUL:
		printf("	imul %%rdi, %%rax\n");
		return;
	case ND_DIV:
		printf("	cqo\n");
		printf("	idiv %%rdi\n");
		return;
	case ND_EQ:
	case ND_NE:
	case ND_LT:
	case ND_LE:
		printf("	cmp %%rdi, %%rax\n");

		if (node->kind == ND_EQ)
		{
			printf("	sete %%al\n");
		}
		else if (node->kind == ND_NE)
		{
			printf("	setne %%al\n");
		}
		else if (node->kind == ND_LT)
		{
			printf("	setl %%al\n");
		}
		else
		{
			printf("	setle %%al\n");
		}

		printf("	movzb %%al, %%rax\n");
		return;
	}

	error("error gen_expr\n");
}

void codegen(Node *node)
{
	printf(".globl main\n\n");
	printf("main:\n");

	gen_expr(node);

	printf("	ret\n");

	assert(depth == 0);
	return;
}