#include "header.h"

static char *current_fn_name;
static char *argreg64[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
static char *argreg8[] = {"%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
static int depth;

void push(void) {
  printf("	push %%rax\n");
  depth++;
}

void pop(char *target) {
  printf("	pop %s\n", target);
  depth--;
}

static int count(void) {
  static int i = 0;
  return i++;
}

int align_to(int offset, int align) {
  return (offset + align - 1) / align * align;
}

void assign_lvar_offset(Obj *prog) {
  for (Obj *fn = prog; fn; fn = fn->next) {
    if (!fn->is_func)
      continue;

    int offset = 0;
    for (Obj *var = fn->locals; var; var = var->next) {
      offset += var->ty->size;
      var->offset -= offset;
    }
    fn->stack_size = align_to(offset, 16);
  }
}

void emit_data(Obj *prog) {
  for (Obj *var = prog; var; var = var->next) {
    if (var->is_func)
      continue;

    printf("	.data\n");
    printf("	.globl %s\n", var->name);
    printf("%s:\n", var->name);

    if (var->init_data) {
      for (int i = 0; i < var->ty->size; i++)
        printf("	.byte %d\n", var->init_data[i]);
    } else {
      printf("	.zero %d\n", var->ty->size);
    }
  }
}

void gen_expr(Node *node);

void gen_addr(Node *node) {
  switch (node->kind) {
  case ND_VAR:
    if (node->var->is_global) {
      printf("	lea %s(%%rip), %%rax\n", node->var->name);
    } else {
      printf("	lea %d(%%rbp), %%rax\n", node->var->offset);
    }
    return;
  case ND_DEREF:
    gen_expr(node->lhs);
    return;
  }

  error_tok(node->tok, "error gen_addr");
}

void load(Type *ty) {
  if (ty->kind == TY_ARRAY)
    return;

  if (ty->size == 1) {
    printf("	movsbq (%%rax), %%rax\n");
  } else {
    printf("	mov (%%rax), %%rax\n");
  }
}

void store(Type *ty) {
  pop("%rdi\n");

  if (ty->size == 1) {
    printf("	mov %%al, (%%rdi)\n");
  } else {
    printf("	mov %%rax, (%%rdi)\n");
  }
}

void gen_expr(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    printf("	mov $%d, %%rax\n", node->val);
    return;
  case ND_NEG:
    gen_expr(node->lhs);
    printf("	neg %%rax\n");
    return;
  case ND_VAR:
    gen_addr(node);
    load(node->ty);
    return;
  case ND_ASSIGN:
    gen_addr(node->lhs);
    push();
    gen_expr(node->rhs);
    store(node->ty);
    return;
  case ND_ADDR:
    gen_addr(node->lhs);
    return;
  case ND_DEREF:
    gen_expr(node->lhs);
    load(node->ty);
    return;
  case ND_FUNCALL: {
    int nargs = 0;
    for (Node *cur = node->args; cur; cur = cur->next) {
      gen_expr(cur);
      push();
      nargs++;
    }

    for (int i = nargs - 1; i >= 0; i--)
      pop(argreg64[i]);

    printf("	mov $0, %%rax\n");
    printf("	call %s\n", node->funcname);

    return;
  }
  }

  gen_expr(node->rhs);
  push();
  gen_expr(node->lhs);
  pop("%rdi");

  switch (node->kind) {
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

    if (node->kind == ND_EQ) {
      printf("	sete %%al\n");
    } else if (node->kind == ND_NE) {
      printf("	setne %%al\n");
    } else if (node->kind == ND_LT) {
      printf("	setl %%al\n");
    } else {
      printf("	setle %%al\n");
    }

    printf("	movzb %%al, %%rax\n");
    return;
  }

  error_tok(node->tok, "error expression");
}

void gen_stmt(Node *node) {
  switch (node->kind) {
  case ND_RETURN:
    gen_expr(node->lhs);
    printf("	jmp .L.return.%s\n", current_fn_name);
    return;
  case ND_EXPR_STMT:
    gen_expr(node->lhs);
    return;
  case ND_BLOCK:
    for (Node *n = node->body; n; n = n->next)
      gen_stmt(n);
    return;
  case ND_IF: {
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
  case ND_FOR: {
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

void emit_text(Obj *prog) {
  for (Obj *fn = prog; fn; fn = fn->next) {
    if (!fn->is_func)
      continue;

    printf("	.text\n");
    printf("	.globl %s\n", fn->name);
    printf("%s:\n", fn->name);
    current_fn_name = fn->name;

    printf("	push %%rbp\n");
    printf("	mov %%rsp, %%rbp\n");
    printf("	sub $%d, %%rsp\n", fn->stack_size);

    int i = 0;
    for (Obj *var = fn->params; var; var = var->next) {
      if (var->ty->size == 1) {
        printf("	mov %s, %d(%%rbp)\n", argreg8[i++], var->offset);
      } else {
        printf("	mov %s, %d(%%rbp)\n", argreg64[i++], var->offset);
      }
    }

    gen_stmt(fn->body);
    assert(depth == 0);

    printf(".L.return.%s:\n", fn->name);
    printf("	mov %%rbp, %%rsp\n");
    printf("	pop %%rbp\n");
    printf("	ret\n");
  }
}

void codegen(Obj *prog) {
  assign_lvar_offset(prog);
  emit_text(prog);
  emit_data(prog);
}