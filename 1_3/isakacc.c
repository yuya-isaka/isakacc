#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "引数の個数が違うよ。");
		return 1;
	}

	printf(".globl main\n\n");
	printf("main:\n");
	printf("	mov $%d, %%rax\n", atoi(argv[1]));
	printf("	ret\n");

	return 0;
}