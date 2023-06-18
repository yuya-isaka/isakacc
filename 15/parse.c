#include "header.h"

static Obj *locals;

static Node *new_node(NodeKind kind)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	return node;
}

static Node *new_unary(NodeKind kind, Node *lhs)
{
	Node *node = new_node(kind);
	node->lhs = lhs;
	return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
	Node *node = new_node(kind);
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

static Node *new_num(int val)
{
	Node *node = new_node(ND_NUM);
	node->val = val;
	return node;
}

static Obj *find_lvar(Token *tok)
{
	for (Obj *cur = locals; cur; cur = cur->next)
		if (strncmp(tok->loc, cur->name, tok->len) == 0 && strlen(cur->name) == tok->len)
			return cur;
	return NULL;
}

static Obj *new_ident(char *name)
{
	Obj *var = calloc(1, sizeof(Obj));
	var->name = name;
	var->next = locals;
	locals = var;
	return var;
}

static Node *new_var(Obj *var)
{
	Node *node = new_node(ND_VAR);
	node->var = var;
	return node;
}

static Node *compound_stmt(Token **rest, Token *tok);
static Node *stmt(Token **rest, Token *tok);
static Node *expr_stmt(Token **rest, Token *tok);
static Node *expr(Token **rest, Token *tok);
static Node *assign(Token **rest, Token *tok);
static Node *equivalent(Token **rest, Token *tok);
static Node *relation(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *unary(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);

static Node *compound_stmt(Token **rest, Token *tok)
{
	Node head = {};
	Node *cur = &head;

	while (!equal(tok, "}"))
	{
		cur = cur->next = stmt(&tok, tok);
	}

	Node *node = new_node(ND_BLOCK);
	node->body = head.next;
	*rest = tok->next;
	return node;
}

static Node *stmt(Token **rest, Token *tok)
{
	if (equal(tok, "return"))
	{
		Node *node = new_unary(ND_RETURN, expr(&tok, tok->next));
		*rest = skip(tok, ";");
		return node;
	}

	if (equal(tok, "{"))
		return compound_stmt(rest, tok->next);

	if (equal(tok, "if"))
	{
		Node *node = new_node(ND_IF);
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
		Node *node = new_node(ND_FOR);
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

	return expr_stmt(rest, tok);
}

static Node *expr_stmt(Token **rest, Token *tok)
{
	if (equal(tok, ";"))
	{
		Node *node = new_node(ND_BLOCK);
		*rest = tok->next;
		return node;
	}

	Node *node = new_unary(ND_EXPR_STMT, expr(&tok, tok));
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
		node = new_binary(ND_ASSIGN, node, assign(&tok, tok->next));
	*rest = tok;
	return node;
}

static Node *equivalent(Token **rest, Token *tok)
{
	Node *node = relation(&tok, tok);

	for (;;)
	{
		if (equal(tok, "=="))
		{
			node = new_binary(ND_EQ, node, relation(&tok, tok->next));
			continue;
		}
		if (equal(tok, "!="))
		{
			node = new_binary(ND_NE, node, relation(&tok, tok->next));
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
		if (equal(tok, "<"))
		{
			node = new_binary(ND_LT, node, add(&tok, tok->next));
			continue;
		}
		if (equal(tok, "<="))
		{
			node = new_binary(ND_LE, node, add(&tok, tok->next));
			continue;
		}
		if (equal(tok, ">"))
		{
			node = new_binary(ND_LT, add(&tok, tok->next), node);
			continue;
		}
		if (equal(tok, ">="))
		{
			node = new_binary(ND_LE, add(&tok, tok->next), node);
			continue;
		}

		*rest = tok;
		return node;
	}
}

static Node *add(Token **rest, Token *tok)
{
	Node *node = mul(&tok, tok);

	for (;;)
	{
		if (equal(tok, "+"))
		{
			node = new_binary(ND_ADD, node, mul(&tok, tok->next));
			continue;
		}
		if (equal(tok, "-"))
		{
			node = new_binary(ND_SUB, node, mul(&tok, tok->next));
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
		if (equal(tok, "*"))
		{
			node = new_binary(ND_MUL, node, unary(&tok, tok->next));
			continue;
		}
		if (equal(tok, "/"))
		{
			node = new_binary(ND_DIV, node, unary(&tok, tok->next));
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
		return new_unary(ND_NEG, unary(rest, tok->next));

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
		Node *node = new_num(tok->val);
		*rest = tok->next;
		return node;
	}

	if (tok->kind == TK_IDENT)
	{
		Obj *var = find_lvar(tok);
		if (!var)
			var = new_ident(strndup(tok->loc, tok->len));
		*rest = tok->next;
		return new_var(var);
	}

	error_tok(tok, "%d error primary: %s", tok->kind, strndup(tok->loc, tok->len));
}

Function *parse(Token *tok)
{
	tok = skip(tok, "{");

	Function *prog = calloc(1, sizeof(Function));
	prog->body = compound_stmt(&tok, tok);
	prog->locals = locals;
	return prog;
}