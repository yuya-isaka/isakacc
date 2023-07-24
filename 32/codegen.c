#include "header.h"

static FILE *output_file;
static char *current_fn_name;
static char *argreg64[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
static char *argreg8[] = {"%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
static int depth;

static void println(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(output_file, fmt, ap);
  va_end(ap);
  fprintf(output_file, "\n");
}

static void push(void) {
  println("	push %%rax");
  depth++;
}

static void pop(char *target) {
  println("	pop %s", target);
  depth--;
}

static int count(void) {
  static int i = 0;
  return i++;
}

static int align_to(int offset, int align) {
  return (offset + align - 1) / align * align;
}

static void assign_lvar_offset(Obj *prog) {
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

static void emit_data(Obj *prog) {
  for (Obj *var = prog; var; var = var->next) {
    if (var->is_func)
      continue;

    println("	.data");
    println("	.globl %s", var->name);
    println("%s:", var->name);

    if (var->init_data) {
      for (int i = 0; i < var->ty->size; i++)
        println("	.byte %d", var->init_data[i]);
    } else {
      println("	.zero %d", var->ty->size);
    }
  }
}

static void gen_expr(Node *node);
static void gen_stmt(Node *node);

static void gen_addr(Node *node) {
  switch (node->kind) {
  case ND_VAR:
    if (node->var->is_global) {
      println("	lea %s(%%rip), %%rax", node->var->name);
    } else {
      println("	lea %d(%%rbp), %%rax", node->var->offset);
    }
    return;
  case ND_DEREF:
    gen_expr(node->lhs);
    return;
  }

  error_tok(node->tok, "error gen_addr");
}

static void load(Type *ty) {
  if (ty->kind == TY_ARRAY)
    return;

  if (ty->size == 1) {
    println("	movsbq (%%rax), %%rax");
  } else {
    println("	mov (%%rax), %%rax");
  }
}

static void store(Type *ty) {
  pop("%rdi");

  if (ty->size == 1) {
    println("	mov %%al, (%%rdi)");
  } else {
    println("	mov %%rax, (%%rdi)");
  }
}

static void gen_expr(Node *node) {
  switch (node->kind) {
  case ND_NUM:
    println("	mov $%d, %%rax", node->val);
    return;
  case ND_NEG:
    gen_expr(node->lhs);
    println("	neg %%rax");
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
  case ND_STMT_EXPR:
    for (Node *cur = node->body; cur; cur = cur->next)
      gen_stmt(cur);
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

    println("	mov $0, %%rax");
    println("	call %s", node->funcname);

    return;
  }
  }

  gen_expr(node->rhs);
  push();
  gen_expr(node->lhs);
  pop("%rdi");

  switch (node->kind) {
  case ND_ADD:
    println("	add %%rdi, %%rax");
    return;
  case ND_SUB:
    println("	sub %%rdi, %%rax");
    return;
  case ND_MUL:
    println("	imul %%rdi, %%rax");
    return;
  case ND_DIV:
    println("	cqo");
    println("	idiv %%rdi");
    return;
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
    println("	cmp %%rdi, %%rax");

    if (node->kind == ND_EQ) {
      println("	sete %%al");
    } else if (node->kind == ND_NE) {
      println("	setne %%al");
    } else if (node->kind == ND_LT) {
      println("	setl %%al");
    } else {
      println("	setle %%al");
    }

    println("	movzb %%al, %%rax");
    return;
  }

  error_tok(node->tok, "error expression");
}

static void gen_stmt(Node *node) {
  switch (node->kind) {
  case ND_RETURN:
    gen_expr(node->lhs);
    println("	jmp .L.return.%s", current_fn_name);
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
    println("	cmp $0, %%rax");
    println("	je .L.else.%d", c);
    gen_stmt(node->then);
    println("	jmp .L.end.%d", c);
    println(".L.else.%d:", c);
    if (node->els)
      gen_stmt(node->els);
    println(".L.end.%d:", c);
    return;
  }
  case ND_FOR: {
    int c = count();
    if (node->init)
      gen_stmt(node->init);
    println(".L.begin.%d:", c);
    if (node->cond)
      gen_expr(node->cond);
    println("	cmp $0, %%rax");
    println("	je .L.end.%d", c);
    gen_stmt(node->then);
    if (node->inc)
      gen_expr(node->inc);
    println("	jmp .L.begin.%d", c);
    println(".L.end.%d:", c);
    return;
  }
  }

  error_tok(node->tok, "error statement");
}

static void emit_text(Obj *prog) {
  for (Obj *fn = prog; fn; fn = fn->next) {
    if (!fn->is_func)
      continue;

    println("	.text");
    println("	.globl %s", fn->name);
    println("%s:", fn->name);
    current_fn_name = fn->name;

    println("	push %%rbp");
    println("	mov %%rsp, %%rbp");
    println("	sub $%d, %%rsp", fn->stack_size);

    int i = 0;
    for (Obj *var = fn->params; var; var = var->next) {
      if (var->ty->size == 1) {
        println("	mov %s, %d(%%rbp)", argreg8[i++], var->offset);
      } else {
        println("	mov %s, %d(%%rbp)", argreg64[i++], var->offset);
      }
    }

    gen_stmt(fn->body);
    assert(depth == 0);

    println(".L.return.%s:", fn->name);
    println("	mov %%rbp, %%rsp");
    println("	pop %%rbp");
    println("	ret");
  }
}

void codegen(Obj *prog, FILE *out) {
  output_file = out;

  assign_lvar_offset(prog);
  emit_text(prog);
  emit_data(prog);
}