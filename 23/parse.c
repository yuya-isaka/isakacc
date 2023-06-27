#include "header.h"

static Obj *locals;

static char *get_ident(Token *tok)
{
	if (tok->kind != TK_IDENT)
		error_tok(tok, "error get_ident: %s", strndup(tok->loc, tok->len));
	return strndup(tok->loc, tok->len);
}

static Type *declspec(Token **rest, Token *tok)
{
	if (equal(tok, "int"))
	{
		*rest = tok->next;
		return ty_int;
	}
	error_tok(tok, "error declspec");
}

static Type *type_suffix(Token **rest, Token *tok, Type *ty)
{
	if (equal(tok, "("))
	{
		*rest = skip(tok->next, ")");
		return type_func(ty);
	}
	*rest = tok;
	return ty;
}

static Type *declarator(Token **rest, Token *tok, Type *ty)
{
	while (consume(&tok, tok, "*"))
		ty = pointer_to(ty);

	if (tok->kind != TK_IDENT)
		error_tok(tok, "error declarator");

	ty = type_suffix(rest, tok->next, ty);
	ty->name = tok;
	return ty;
}

static Node *new_node(NodeKind kind, Token *tok)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->tok = tok;
	return node;
}

static Obj *new_lvar(char *name, Type *ty)
{
	Obj *var = calloc(1, sizeof(Obj));
	var->name = name;
	var->ty = ty;
	var->next = locals;
	locals = var;
	return var;
}

static Node *new_var_node(Obj *var, Token *tok)
{
	Node *node = new_node(ND_VAR, tok);
	node->var = var;
	return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok)
{
	Node *node = new_node(kind, tok);
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

static Node *new_unary(NodeKind kind, Node *lhs, Token *tok)
{
	Node *node = new_node(kind, tok);
	node->lhs = lhs;
	return node;
}

static Node *new_num(int val, Token *tok)
{
	Node *node = new_node(ND_NUM, tok);
	node->val = val;
	return node;
}

static Obj *find_lvar(Token *tok)
{
	for (Obj *var = locals; var; var = var->next)
		if (equal(tok, var->name))
			return var;
	return NULL;
}

static Node *declaration(Token **rest, Token *tok);
static Node *stmt(Token **rest, Token *tok);
static Node *compound_stmt(Token **rest, Token *tok);
static Node *expr_stmt(Token **rest, Token *tok);
static Node *expr(Token **rest, Token *tok);
static Node *assign(Token **rest, Token *tok);
static Node *equivalent(Token **rest, Token *tok);
static Node *relation(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *unary(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);

static Node *declaration(Token **rest, Token *tok)
{
	Type *basety = declspec(&tok, tok);

	Node head = {};
	Node *cur = &head;

	int i = 0;

	while (!equal(tok, ";"))
	{
		if (i++ > 0)
			tok = skip(tok, ",");

		Type *ty = declarator(&tok, tok, basety);
		Obj *var = new_lvar(get_ident(ty->name), ty);

		if (!consume(&tok, tok, "="))
			continue;

		Node *lhs = new_var_node(var, tok);
		Node *rhs = assign(&tok, tok);
		Node *node = new_binary(ND_ASSIGN, lhs, rhs, tok);
		cur = cur->next = new_unary(ND_EXPR_STMT, node, tok);
	}

	Node *node = new_node(ND_BLOCK, tok);
	node->body = head.next;
	*rest = tok->next;
	return node;
}

static Node *stmt(Token **rest, Token *tok)
{
	if (equal(tok, "return"))
	{
		Node *node = new_node(ND_RETURN, tok);
		node->lhs = expr(&tok, tok->next);
		*rest = skip(tok, ";");
		return node;
	}

	if (equal(tok, "{"))
		return compound_stmt(rest, tok->next);

	if (equal(tok, "if"))
	{
		Node *node = new_node(ND_IF, tok);
		tok = skip(tok->next, "(");
		node->cond = expr(&tok, tok);
		tok = skip(tok, ")");
		node->then = stmt(&tok, tok);
		if (equal(tok, "else"))
			node->els = stmt(&tok, tok->next);
		*rest = tok;
		return node;
	}

	if (equal(tok, "for"))
	{
		Node *node = new_node(ND_FOR, tok);
		tok = skip(tok->next, "(");
		node->init = expr_stmt(&tok, tok);

		if (!equal(tok, ";"))
			node->cond = expr(&tok, tok);
		tok = skip(tok, ";");

		if (!equal(tok, ")"))
			node->inc = expr(&tok, tok);
		tok = skip(tok, ")");

		node->then = stmt(rest, tok);
		return node;
	}

	if (equal(tok, "while"))
	{
		Node *node = new_node(ND_FOR, tok);

		tok = skip(tok->next, "(");
		node->cond = expr(&tok, tok);
		tok = skip(tok, ")");
		node->then = stmt(rest, tok);
		return node;
	}

	return expr_stmt(rest, tok);
}

static Node *compound_stmt(Token **rest, Token *tok)
{
	Node head = {};
	Node *cur = &head;

	while (!equal(tok, "}"))
	{
		if (equal(tok, "int"))
			cur = cur->next = declaration(&tok, tok);
		else
			cur = cur->next = stmt(&tok, tok);
		add_type(cur);
	}

	Node *node = new_node(ND_BLOCK, tok);
	node->body = head.next;
	*rest = tok->next;
	return node;
}

static Node *expr_stmt(Token **rest, Token *tok)
{
	if (equal(tok, ";"))
	{
		*rest = tok->next;
		return new_node(ND_BLOCK, tok);
	}

	Node *node = new_node(ND_EXPR_STMT, tok);
	node->lhs = expr(&tok, tok);
	*rest = skip(tok, ";");
	return node;
}

static Node *expr(Token **rest, Token *tok)
{
	return assign(rest, tok);
}

static Node *assign(Token **rest, Token *tok)
{
	Node *node = equivalent(&tok, tok);

	if (equal(tok, "="))
		return new_binary(ND_ASSIGN, node, assign(rest, tok->next), tok);

	*rest = tok;
	return node;
}

static Node *equivalent(Token **rest, Token *tok)
{
	Node *node = relation(&tok, tok);

	for (;;)
	{
		Token *start = tok;

		if (equal(tok, "=="))
		{
			node = new_binary(ND_EQ, node, relation(&tok, tok->next), start);
			continue;
		}
		if (equal(tok, "!="))
		{
			node = new_binary(ND_NE, node, relation(&tok, tok->next), start);
			continue;
		}

		*rest = tok;
		return node;
	}
}

static Node *relation(Token **rest, Token *tok)
{
	Node *node = add(&tok, tok);

	for (;;)
	{
		Token *start = tok;

		if (equal(tok, "<"))
		{
			node = new_binary(ND_LT, node, add(&tok, tok->next), start);
			continue;
		}
		if (equal(tok, "<="))
		{
			node = new_binary(ND_LE, node, add(&tok, tok->next), start);
			continue;
		}
		if (equal(tok, ">"))
		{
			node = new_binary(ND_LT, add(&tok, tok->next), node, start);
			continue;
		}
		if (equal(tok, ">="))
		{
			node = new_binary(ND_LE, add(&tok, tok->next), node, start);
			continue;
		}

		*rest = tok;
		return node;
	}
}

static Node *new_add(Node *lhs, Node *rhs, Token *tok)
{
	add_type(lhs);
	add_type(rhs);

	if (is_integer(lhs->ty) && is_integer(rhs->ty))
		return new_binary(ND_ADD, lhs, rhs, tok);

	if (lhs->ty->base && rhs->ty->base)
		error_tok(tok, "error new_add");

	if (is_integer(lhs->ty) && rhs->ty->base)
	{
		Node *tmp = lhs;
		lhs = rhs;
		rhs = tmp;
	}

	rhs = new_binary(ND_MUL, rhs, new_num(8, tok), tok);
	return new_binary(ND_ADD, lhs, rhs, tok);
}

static Node *new_sub(Node *lhs, Node *rhs, Token *tok)
{
	add_type(lhs);
	add_type(rhs);

	if (is_integer(lhs->ty) && is_integer(rhs->ty))
		return new_binary(ND_SUB, lhs, rhs, tok);

	if (is_integer(rhs->ty) && lhs->ty->base)
	{
		rhs = new_binary(ND_MUL, rhs, new_num(8, tok), tok);
		return new_binary(ND_SUB, lhs, rhs, tok);
	}

	if (lhs->ty->base && rhs->ty->base)
	{
		Node *node = new_binary(ND_SUB, lhs, rhs, tok);
		node->ty = ty_int;
		return new_binary(ND_DIV, node, new_num(8, tok), tok);
	}

	error_tok(tok, "error new_sub");
}

static Node *add(Token **rest, Token *tok)
{
	Node *node = mul(&tok, tok);

	for (;;)
	{
		Token *start = tok;

		if (equal(tok, "+"))
		{
			node = new_add(node, mul(&tok, tok->next), start);
			continue;
		}
		if (equal(tok, "-"))
		{
			node = new_sub(node, mul(&tok, tok->next), start);
			continue;
		}

		*rest = tok;
		return node;
	}
}

static Node *mul(Token **rest, Token *tok)
{
	Node *node = unary(&tok, tok);

	for (;;)
	{
		Token *start = tok;

		if (equal(tok, "*"))
		{
			node = new_binary(ND_MUL, node, unary(&tok, tok->next), start);
			continue;
		}
		if (equal(tok, "/"))
		{
			node = new_binary(ND_DIV, node, unary(&tok, tok->next), start);
			continue;
		}

		*rest = tok;
		return node;
	}
}

static Node *unary(Token **rest, Token *tok)
{
	if (equal(tok, "+"))
		return unary(rest, tok->next);
	if (equal(tok, "-"))
		return new_unary(ND_NEG, unary(rest, tok->next), tok);
	if (equal(tok, "&"))
		return new_unary(ND_ADDR, unary(rest, tok->next), tok);
	if (equal(tok, "*"))
		return new_unary(ND_DEREF, unary(rest, tok->next), tok);

	return primary(rest, tok);
}

static Node *funcall(Token **rest, Token *tok)
{
	Node *node = new_node(ND_FUNCALL, tok);
	node->funcname = get_ident(tok);

	tok = tok->next->next;

	Node head = {};
	Node *cur = &head;

	while (!equal(tok, ")"))
	{
		if (cur != &head)
			tok = skip(tok, ",");
		cur = cur->next = assign(&tok, tok);
	}

	*rest = tok->next;
	node->args = head.next;
	return node;
}

static Node *primary(Token **rest, Token *tok)
{
	if (equal(tok, "("))
	{
		Node *node = expr(&tok, tok->next);
		*rest = skip(tok, ")");
		return node;
	}

	if (tok->kind == TK_NUM)
	{
		Node *node = new_num(tok->val, tok);
		*rest = tok->next;
		return node;
	}

	if (tok->kind == TK_IDENT)
	{
		if (equal(tok->next, "("))
		{
			return funcall(rest, tok);
		}

		Obj *var = find_lvar(tok);
		if (!var)
			error_tok(tok, "error find lvar");
		*rest = tok->next;
		return new_var_node(var, tok);
	}

	error_tok(tok, "error primary");
}

static Function *function(Token **rest, Token *tok)
{
	Type *ty = declspec(&tok, tok);
	ty = declarator(&tok, tok, ty);

	locals = NULL;

	Function *prog = calloc(1, sizeof(Function));
	tok = skip(tok, "{");
	prog->body = compound_stmt(rest, tok);
	prog->locals = locals;
	prog->name = get_ident(ty->name);

	return prog;
}

Function *parse(Token *tok)
{
	Function head = {};
	Function *cur = &head;

	while (tok->kind != TK_EOF)
		cur = cur->next = function(&tok, tok);

	return head.next;
}