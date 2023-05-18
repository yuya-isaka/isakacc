#include <stdio.h>

int main(int argc, char **argv)
{
	char *a = "isaka";
	char **c = &a;
	char *b = a;
	a++;
	printf("a:%p, %s, %p\n", a, a, &a);
	// printf("b:%p, %s, %p\n", b, b, &b);
	printf("c:%p, %s, %p\n", c, *c, &c);
	*c = "aaa";
	printf("a:%p, %s, %p\n", a, a, &a);
	printf("c:%p, %s, %p\n", c, *c, &c);

	return 0;
}