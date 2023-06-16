#include "header.h"

static int align_to(int offset, int align)
{
	return (offset + align - 1) / align * align;
}

static void assign_lvar_offsets(Function *prog)
{
	int offset = 0;
	for (Obj *var = prog->locals; var; var = var->next)
	{
		offset += 8;
		var->offset -= offset;
	}
	prog->stack_size = align_to(offset, 16);
	return;
}

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
	return;
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

	error("invalid expression");
}

static void gen_stmt(Node *node)
{
	switch (node->kind)
	{
	case ND_RETURN:
		gen_expr(node->lhs);
		printf("	jmp .L.return\n");
		return;
	case ND_EXPR_STMT:
		gen_expr(node->lhs);
		return;
	}

	error("invalid statement");
}

void codegen(Function *prog)
{
	assign_lvar_offsets(prog);

	printf(".globl main\n\n");
	printf("main:\n");

	printf("	push %%rbp\n");
	printf("	mov %%rsp, %%rbp\n");
	printf("	sub $%d, %%rsp\n", prog->stack_size);

	for (Node *cur = prog->body; cur; cur = cur->next)
	{
		gen_stmt(cur);
		assert(depth == 0);
	}

	printf("\n.L.return:\n");
	printf("	mov %%rbp, %%rsp\n");
	printf("	pop %%rbp\n");
	printf("	ret\n");

	return;
}