#include <stdio.h>
#include <ctype.h>

int main(int argc, char **argv)
{
	char *str = "1a";
	if (isdigit(*str))
	{
		printf("OK\n");
	}
	else
	{
		printf("No\n");
	}

	return 0;
}