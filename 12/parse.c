#include "header.h"

static Obj *locals;

static bool equal(Token *tok, char *target)
{
	return memcmp(tok->loc, target, tok->len) == 0 && target[tok->len] == '\0';
}

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

static Node *new_var(Obj *var)
{
	Node *node = new_node(ND_VAR);
	node->var = var;
	return node;
}

static Obj *new_lvar(char *name)
{
	Obj *var = calloc(1, sizeof(Obj));
	var->name = name;
	var->next = locals;
	locals = var;
	return var;
}

static Obj *find_var(Token *tok)
{
	for (Obj *cur = locals; cur; cur = cur->next)
	{
		if (strncmp(cur->name, tok->loc, tok->len) == 0 && cur->name[tok->len] == '\0')
		{
			return cur;
		}
	}
	return NULL;
}

static Node *stmt(Token **rest, Token *tok);
static Node *expr(Token **rest, Token *tok);
static Node *assign(Token **rest, Token *tok);
static Node *equality(Token **rest, Token *tok);
static Node *relation(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *unary(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);

static Node *stmt(Token **rest, Token *tok)
{
	Node *node = new_unary(ND_EXPR_STMT, expr(&tok, tok));
	if (!equal(tok, ";"))
		error_tok(tok, "error stmt");
	*rest = tok->next;
	return node;
}

static Node *expr(Token **rest, Token *tok)
{
	return assign(rest, tok);
}

static Node *assign(Token **rest, Token *tok)
{
	Node *node = equality(&tok, tok);
	if (equal(tok, "="))
	{
		node = new_binary(ND_ASSIGN, node, assign(&tok, tok->next));
	}
	*rest = tok;
	return node;
}

static Node *equality(Token **rest, Token *tok)
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
		if (!equal(tok, ")"))
			error_tok(tok, "error primary");
		*rest = tok->next;
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
		Obj *var = find_var(tok);
		if (!var)
			var = new_lvar(strndup(tok->loc, tok->len));
		*rest = tok->next;
		return new_var(var);
	}

	error_tok(tok, "error primary last");
}

Function *parse(Token *tok)
{
	Node head = {};
	Node *cur = &head;

	while (tok->kind != TK_EOF)
	{
		cur = cur->next = stmt(&tok, tok);
	}

	Function *prog = calloc(1, sizeof(Function));
	prog->body = head.next;
	prog->locals = locals;

	return prog;
}