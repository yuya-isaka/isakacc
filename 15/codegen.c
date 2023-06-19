#include "header.h"

static int depth;

static void push()
{
	printf("	push %%rax\n");
	depth++;
}

static int pop(char *target)
{
	printf("	pop %s\n", target);
	depth--;
}

static int count()
{
	static int i = 0;
	return i++;
}

static int align_to(int offset, int align)
{
	return (offset + align - 1) / align * align;
}

static void assign_lvar_offsets(Function *prog)
{
	int offset = 0;
	for (Obj *cur = prog->locals; cur; cur = cur->next)
	{
		offset += 8;
		cur->offset -= offset;
	}
	prog->stack_size = align_to(offset, 16);
}

static void gen_addr(Node *node)
{
	if (node->kind != ND_VAR)
		error("invalid addr");
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
	case ND_VAR:
		gen_addr(node);
		printf("	mov (%%rax), %%rax\n");
		return;
	case ND_ASSIGN:
		gen_addr(node->lhs);
		push();
		gen_expr(node->rhs);
		pop("%rdi");
		printf("	mov %%rax, (%%rdi)\n");
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
	case ND_BLOCK:
		for (Node *cur = node->body; cur; cur = cur->next)
			gen_stmt(cur);
		return;
	case ND_EXPR_STMT:
		gen_expr(node->lhs);
		return;
	case ND_IF:
	{
		int c = count();
		gen_expr(node->cond);
		printf("	cmp $0, %%rax\n");
		printf("	je .L.else.%d\n", c);
		gen_stmt(node->then);
		printf("	jmp .L.end.%d\n", c);
		printf(".L.else.%d:\n", c);
		if (node->els)
			gen_stmt(node->els);
		printf(".L.end.%d:\n", c);
		return;
	}
	case ND_FOR:
	{
		int c = count();
		gen_stmt(node->init);
		printf(".L.begin.%d:\n", c);
		if (node->cond)
		{
			gen_stmt(node->cond);
			printf("	cmp $0, %%rax\n");
			printf("	je .L.end.%d\n", c);
		}
		gen_stmt(node->then);
		if (node->inc)
			gen_expr(node->inc);
		printf("	jmp .L.begin.%d\n", c);
		printf(".L.end.%d:\n", c);
		return;
	}
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

	gen_stmt(prog->body);
	assert(depth == 0);

	printf(".L.return:\n");
	printf("	mov %%rbp, %%rsp\n");
	printf("	pop %%rbp\n");
	printf("	ret\n");
}