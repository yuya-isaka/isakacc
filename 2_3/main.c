#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		fprintf(stderr, "引数の個数が違う");
		return 1;
	}

	char *p = argv[1];

	printf(".globl main\n\n");
	printf("main:\n");
	printf("	mov $%ld, %%rax\n", strtol(p, &p, 10));

	while (*p)
	{
		if (*p == '+')
		{
			p++;
			printf("	add $%ld, %%rax\n", strtol(p, &p, 10));
			continue;
		}

		if (*p == '-')
		{
			p++;
			printf("	sub $%ld, %%rax\n", strtol(p, &p, 10));
			continue;
		}

		fprintf(stderr, "知らん\n");
		return 1;
	}

	printf("	ret\n");
	return 0;
}