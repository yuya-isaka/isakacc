#include "header.h"

static Obj *locals;

static Node *new_node(NodeKind kind, Token *tok)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->tok = tok;
	return node;
}

static Node *new_unary(NodeKind kind, Node *lhs, Token *tok)
{
	Node *node = new_node(kind, tok);
	node->lhs = lhs;
	return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok)
{
	Node *node = new_node(kind, tok);
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

static Node *new_num(int val, Token *tok)
{
	Node *node = new_node(ND_NUM, tok);
	node->val = val;
	return node;
}

static Node *new_var(Obj *var, Token *tok)
{
	Node *node = new_node(ND_VAR, tok);
	node->var = var;
	return node;
}

static Obj *find_lvar(Token *tok)
{
	for (Obj *cur = locals; cur; cur = cur->next)
		if (equal(tok, cur->name))
			return cur;

	return NULL;
}

static Obj *get_ident(char *name)
{
	Obj *var = calloc(1, sizeof(Obj));
	var->name = name;
	var->next = locals;
	locals = var;
	return var;
}

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

Node *compound_stmt(Token **rest, Token *tok)
{
	Node head = {};
	Node *cur = &head;

	Node *node = new_node(ND_BLOCK, tok);

	while (!equal(tok, "}"))
	{
		cur = cur->next = stmt(&tok, tok);
		add_type(cur);
	}

	*rest = tok->next;
	node->body = head.next;
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
		Token *tmp = tok;

		if (equal(tok, "=="))
		{
			node = new_binary(ND_EQ, node, relation(&tok, tok->next), tmp);
			continue;
		}

		if (equal(tok, "!="))
		{
			node = new_binary(ND_NE, node, relation(&tok, tok->next), tmp);
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
		Token *tmp = tok;

		if (equal(tok, "<"))
		{
			node = new_binary(ND_LT, node, add(&tok, tok->next), tmp);
			continue;
		}

		if (equal(tok, "<="))
		{
			node = new_binary(ND_LE, node, add(&tok, tok->next), tmp);
			continue;
		}

		if (equal(tok, ">"))
		{
			node = new_binary(ND_LT, add(&tok, tok->next), node, tmp);
			continue;
		}

		if (equal(tok, ">="))
		{
			node = new_binary(ND_LE, add(&tok, tok->next), node, tmp);
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

	if (!rhs->ty->base && is_integer(lhs->ty))
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

	if (lhs->ty->base && is_integer(rhs->ty))
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
		Token *tmp = tok;

		if (equal(tok, "+"))
		{
			node = new_add(node, mul(&tok, tok->next), tmp);
			continue;
		}

		if (equal(tok, "-"))
		{
			node = new_sub(node, mul(&tok, tok->next), tmp);
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
		Token *tmp = tok;

		if (equal(tok, "*"))
		{
			node = new_binary(ND_MUL, node, unary(&tok, tok->next), tmp);
			continue;
		}

		if (equal(tok, "/"))
		{
			node = new_binary(ND_DIV, node, unary(&tok, tok->next), tmp);
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

	if (equal(tok, "*"))
		return new_unary(ND_DEREF, unary(rest, tok->next), tok);

	if (equal(tok, "&"))
		return new_unary(ND_ADDR, unary(rest, tok->next), tok);

	return primary(rest, tok);
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
		Obj *var = find_lvar(tok);
		if (!var)
			var = get_ident(strndup(tok->loc, tok->len));
		*rest = tok->next;
		return new_var(var, tok);
	}

	error_tok(tok, "error primary");
}

Function *parse(Token *tok)
{
	tok = skip(tok, "{");

	Function *prog = calloc(1, sizeof(Function));
	prog->body = compound_stmt(&tok, tok);
	prog->locals = locals;

	return prog;
}