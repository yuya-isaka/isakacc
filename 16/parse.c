#include "header.h"

static Obj *locals;

static Node *new_node(NodeKind kind, Token *tok)
{
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->tok = tok;
	return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, Token *tok)
{
	Node *node = new_node(kind, tok);
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
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
	if (equal(tok, "{"))
		return compound_stmt(rest, tok->next);

	if (equal(tok, "return"))
	{
		Node *node = new_token(ND_RETURN, tok);
		node->lhs = expr(&tok, tok->next);
		*rest = skip(tok, ";");
		return node;
	}

	if (equal(tok, "if"))
	{
		Node *node = new_token(ND_IF, tok);
		tok = skip(tok->next, "(");
		node->cond = expr(&tok, tok);
		tok = skip(tok, ")");
		node->then = stmt(&tok, tok);
		if (equal(tok, "else"))
			node->els = stmt(&tok, tok);

		*rest = tok;
		return node;
	}

	if (equal(tok, "for"))
	{
		Node *node = new_token(ND_FOR, tok);
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
		Node *node = new_token(ND_FOR, tok);
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

	Node *node = new_node(ND_BLOCK, tok);

	while (!equal(tok, "}"))
		cur = cur->next = stmt(&tok, tok);

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

static Node *relation(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *unary(Token **rest, Token *tok);
static Node *primary(Token **rest, Token *tok);

Function *parse(Token *tok)
{
	tok = skip(tok, "{");

	Function *prog = calloc(1, sizeof(Function));
	prog->body = compound_stmt(&tok, tok);
	prog->locals = locals;
	return prog;
}