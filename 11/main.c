#include "isakacc.h"

int main(int argc, char **argv)
{
	if (argc != 2)
		error("%s: invalid argument\n", argv[0]);

	Token *tok = tokenize(argv[1]);
	Node *node = parse(tok);
	codegen(node);

	return 0;
}