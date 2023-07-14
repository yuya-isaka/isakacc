#include "header.h"

static char *current_fn_name;
static int depth;
static char *argreg8[] = {"%dil", "%sil", "%dl", "%cl", "%r8b", "%r9b"};
static char *argreg64[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

static void push(void) {
  printf("	push %%rax\n");
  depth++;
}

static void pop(char *target) {
  printf("	pop %s\n", target);
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

    printf("	.data\n");
    printf("	.globl %s\n", var->name);
    printf("%s:\n", var->name);

    if (var->init_data) {
      // やはりサイズ
      for (int i = 0; i < var->ty->size; i++)
        printf("	.byte %d\n", var->init_data[i]);
    } else {
      printf("	.zero %d\n", var->ty->size);
    }
  }
}

static void load(Type *ty) {
  if (ty->kind == TY_ARRAY)
    return;

  // やっぱり１
  // 配列はそもそも弾かれてるから問題ない
  if (ty->size == 1)
    printf("	movsbq (%%rax), %%rax\n");
  else
    printf("	mov (%%rax), %%rax\n");
}

static void store(Type *ty) {
  pop("%rdi\n");

  // やっぱり１
  // Assignの左辺に配列が来ないことは保証されてて，左辺の型を受け継いでるから，配列ではないので問題ない
  if (ty->size == 1)
    printf("	mov %%al, (%%rdi)\n");
  else
    printf("	mov %%rax, (%%rdi)\n");
}

static void gen_expr(Node *node);

static void gen_addr(Node *node) {
  switch (node->kind) {
  case ND_VAR:
    if (node->var->is_local)
      printf("	lea %d(%%rbp), %%rax\n", node->var->offset);
    else
      printf("	lea %s(%%rip), %%rax\n", node->var->name);
    return;
  case ND_DEREF:
    gen_expr(node->lhs);
    return;
  }

  error_tok(node->tok, "error gen_addr");
}

static void gen_expr(Node *node) {
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
    // printf("\n\n");
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
    // ここで欲しいのはアドレスが欲しい．つまり中の値はまだ出してほしくない
    // ここで渡されるのは左辺で，左辺にもし配列が来ていた場合，例えば"aaa"[0]という場合は，"aaa"という配列varと0をnew_addしてからそれをデリファレンスしている．つまりnew_addで生成されるものはarrayとなっている．それはnd-addなのでadd_typeで左辺のものが受け継がれるから，derefのところの左辺には配列が入っていると
    // でこれを進めると中にはvarで配列のものやからND_VARのところがよばれる
    // で，その後，それは配列やからそこでアドレスを求めた後に，帰ってくるはず
    // でも"aaa"でやる場合は，これもvarの配列やねんけど，これはderef
    // で呼ばれないから，そのままvarがよばれる，なので，それは配列とみなされて
    // アドレスだけが残る
    // なのでその中身自体を参照することができていないということか
    // 文字列だけの時はそれはvarとして扱われているから，
    // varで配列の時，こうやって書きながらの方が割と整理できている気がする
    // 「varで配列」これはキーワード
    // varで配列のやつはそのままアクセスしても，アドレスが得られるだけ．つまりデリファレンスしないといけない
    gen_expr(node->lhs);
    // printf("\n\n\n");
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

    // bですと，多分 byteでbなんかな
    printf("	movzb %%al, %%rax\n");
    return;
  }
}

static void gen_stmt(Node *node) {
  switch (node->kind) {
  case ND_RETURN:
    gen_expr(node->lhs);
    printf("	jmp .L.return.%s\n", current_fn_name);
    return;
  case ND_EXPR_STMT:
    gen_expr(node->lhs);
    return;
  case ND_BLOCK:
    for (Node *cur = node->body; cur; cur = cur->next)
      gen_stmt(cur);
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

static void emit_text(Obj *prog) {
  for (Obj *fn = prog; fn; fn = fn->next) {
    if (!fn->is_func)
      continue;

    current_fn_name = fn->name;
    printf("	.text\n");
    printf("	.globl %s\n", current_fn_name);
    printf("%s:\n", current_fn_name);

    printf("	push %%rbp\n");
    printf("	mov %%rsp, %%rbp\n");
    printf("	sub $%d, %%rsp\n", fn->stack_size);

    int i = 0;
    for (Obj *var = fn->params; var; var = var->next) {
      // やっぱり１かー
      if (var->ty->size == 1)
        printf("	mov %s, %d(%%rbp)\n", argreg8[i++], var->offset);
      else
        printf("	mov %s, %d(%%rbp)\n", argreg64[i++], var->offset);
    }

    gen_stmt(fn->body);
    assert(depth == 0);

    printf(".L.return.%s:\n", current_fn_name);
    printf("	mov %%rbp, %%rsp\n");
    printf("	pop %%rbp\n");
    printf("	ret\n");
  }
}

void codegen(Obj *prog) {
  assign_lvar_offset(prog);
  emit_data(prog);
  emit_text(prog);
}