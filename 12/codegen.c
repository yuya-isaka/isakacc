#include "header.h"

static int depth = 0;

static void push()
{
	printf("	push %%rax\n");
	depth++;
	return;
}

static void pop(char *msg)
{
	printf("	pop %s\n", msg);
	depth--;
	return;
}

static void gen_addr(Node *node)
{
	if (node->kind != ND_VAR)
		error("error gen_addr");
	printf("	lea %d(%%rbp), %%rax\n", node->var->offset);
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

	error("error gen_expr");
}

static int align_to(int offset, int align)
{
	return (offset + align - 1) / align * align;
}

static void assign_lvar_offset(Function *prog)
{
	int offset = 0;
	for (Obj *cur = prog->locals; cur; cur = cur->next)
	{
		offset += 8;
		cur->offset = -offset;
	}
	prog->stack_size = align_to(offset, 16);
}

void codegen(Function *prog)
{
	assign_lvar_offset(prog);

	printf(".globl main\n\n");
	printf("main:\n");

	printf("	push %%rbp\n");
	printf("	mov %%rsp, %%rbp\n");
	printf("	sub $%d, %%rsp\n", prog->stack_size);

	for (Node *cur = prog->body; cur; cur = cur->next)
	{
		if (cur->kind != ND_EXPR_STMT)
			error("error codegen");
		gen_expr(cur->lhs);
		assert(depth == 0);
	}

	printf("	mov %%rbp, %%rsp\n");
	printf("	pop %%rbp\n");
	printf("	ret\n");

	return;
}