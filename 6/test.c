#include <stdio.h>
#include <string.h>

char *str1 = "abc\0def";
char *str2 = "abc";

int main(int argc, char **argv)
{
	if (memcmp(str1, str2, 4))
	{
		printf("違う, %d, %d\n", sizeof(*str1), sizeof(*str2));
		printf("違う, %d, %d\n", strlen(str1), strlen(str2));
	}
	else
	{
		printf("一緒, %d, %d\n", sizeof(*str1), sizeof(*str2));
		printf("一緒, %d, %d\n", strlen(str1), strlen(str2));
	}

	return 0;
}