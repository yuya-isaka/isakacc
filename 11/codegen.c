#include "isakacc.h"

static int depth = 0;

static void push(void)
{
	printf("	push %%rax\n");
	depth++;
	return;
}

static void pop(char *target)
{
	printf("	pop %s\n", target);
	depth--;
	return;
}

static void gen_addr(Node *node)
{
	if (node->kind != ND_VAR)
		error("error gen_addr\n");
	int offset = (node->name - 'a' + 1) * -8;
	printf("	lea %d(%%rbp), %%rax\n", offset);
}

static void gen_expr(Node *node)
{
	switch (node->kind)
	{
	case ND_NUM:
		printf("	mov $%d, %%rax\n", node->val);
		return;
	case ND_NEG:
		gen_expr(node->lhs);
		printf("	neg %%rax\n");
		return;
	case ND_ASSIGN:
		gen_addr(node->lhs);
		push();
		gen_expr(node->rhs);
		pop("%rdi");
		printf("	mov %%rax, (%%rdi)\n");
		return;
	case ND_VAR:
		gen_addr(node);
		printf("	mov (%%rax), %%rax\n");
		return;
	}

	gen_expr(node->rhs);
	push();
	gen_expr(node->lhs);
	pop("%rdi");

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

	printf("	push %%rbp\n");
	printf("	mov %%rsp, %%rbp\n");
	printf("	sub $208, %%rsp\n");

	for (Node *cur = node; cur; cur = cur->next)
	{
		if (node->kind != ND_EXPR_STMT)
			error("error codegen\n");
		gen_expr(cur->lhs);
		assert(depth == 0);
	}

	printf("	mov %%rbp, %%rsp\n");
	printf("	pop %%rbp\n");
	printf("	ret\n");

	return;
}