#include "header.h"

static int depth;
static char *current_fn_name;
static char *argreg[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

static void push()
{
	printf("	push %%rax\n");
	depth++;
}

static void pop(char *target)
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

static void assign_lvar_offset(Function *prog)
{
	for (Function *func = prog; func; func = func->next)
	{
		int offset = 0;
		for (Obj *cur = func->locals; cur; cur = cur->next)
		{
			offset += 8;
			cur->offset = -offset;
		}
		func->stack_size = align_to(offset, 16);
	}
}

static void gen_expr(Node *node);

static void gen_addr(Node *node)
{
	if (node->kind == ND_VAR)
	{
		printf("	lea %d(%%rbp), %%rax\n", node->var->offset);
		return;
	}

	if (node->kind == ND_DEREF)
	{
		gen_expr(node->lhs);
		return;
	}

	error_tok(node->tok, "error gen_addr");
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
	case ND_ADDR:
		gen_addr(node->lhs);
		return;
	case ND_DEREF:
		gen_expr(node->lhs);
		printf("	mov (%%rax), %%rax\n");
		return;
	case ND_FUNCALL:
	{
		int nargs = 0;
		for (Node *arg = node->args; arg; arg = arg->next)
		{
			gen_expr(arg);
			push();
			nargs++;
		}

		for (int i = nargs - 1; i >= 0; i--)
			pop(argreg[i]);

		printf("	mov $0, %%rax\n");
		printf("	call %s\n", node->funcname);
		return;
	}
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

	error_tok(node->tok, "error expression");
}

static void gen_stmt(Node *node)
{
	switch (node->kind)
	{
	case ND_RETURN:
		gen_expr(node->lhs);
		printf("	jmp .L.return.%s\n", current_fn_name);
		return;
	case ND_BLOCK:
		for (Node *n = node->body; n; n = n->next)
			gen_stmt(n);
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
		if (node->init)
			gen_stmt(node->init);
		printf(".L.begin.%d:\n", c);
		if (node->cond)
			gen_expr(node->cond);
		printf("	cmp $0, %%rax\n");
		printf("	je .L.end.%d\n", c);
		gen_stmt(node->then);
		if (node->inc)
			gen_expr(node->inc);
		printf("	jmp .L.begin.%d\n", c);
		printf(".L.end.%d:\n", c);
		return;
	}
	}

	error_tok(node->tok, "error statement");
}

void codegen(Function *prog)
{
	assign_lvar_offset(prog);

	for (Function *func = prog; func; func = func->next)
	{
		current_fn_name = func->name;

		printf("	.globl %s\n", func->name);
		printf("%s:\n", func->name);

		printf("	push %%rbp\n");
		printf("	mov %%rsp, %%rbp\n");
		printf("	sub $%d, %%rsp\n", func->stack_size);

		int i = 0;
		for (Obj *var = func->params; var; var = var->next)
			printf("	mov %s, %d(%%rbp)\n", argreg[i++], var->offset);

		gen_stmt(func->body);
		assert(depth == 0);

		printf(".L.return.%s:\n", func->name);
		printf("	mov %%rbp, %%rsp\n");
		printf("	pop %%rbp\n");
		printf("	ret\n");
	}
}