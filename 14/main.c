#include "header.h"

// static void check(Node *node)
// {
// 	if (node->body)
// 	{
// 		check(node->body);
// 	}
// 	if (node->lhs)
// 	{
// 		printf("lhs val:%d\n", node->lhs->val);
// 		check(node->lhs);
// 	}
// 	if (node->rhs)
// 	{
// 		printf("rhs val:%d\n", node->rhs->val);
// 		check(node->rhs);
// 	}
// }

int main(int argc, char **argv)
{
	if (argc != 2)
		error("%s invalid argument", argv[0]);

	Token *tok = tokenize(argv[1]);

	// for (Token *cur = tok; cur->kind != TK_EOF; cur = cur->next)
	// {
	// 	printf(" name:%s	val%d\n", strndup(cur->loc, cur->len), cur->val);
	// }

	Function *prog = parse(tok);

	// for (Node *cur = prog->body; cur; cur = cur->next)
	// {
	// 	printf(" val:%d\n", cur->val);
	// 	check(cur);
	// }

	codegen(prog);

	return 0;
}