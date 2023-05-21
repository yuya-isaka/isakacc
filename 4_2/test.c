#include <stdio.h>

static void plus(int *a, int *c)
{
	c = a;
	printf("a: %p\n", a);
	printf("c: %p\n", c);
}

int main(int argc, char **argv)
{
	int b = 1;
	int *a = &b;
	int tmp = 5;
	int *c = &tmp;

	printf("1. a:%p\n", a);
	printf("1. c:%p\n", c);
	plus(a, c);
	printf("2. a:%p\n", a);
	printf("2. c:%p\n", c);

	// まとめ
	// ポインタの値を変えたかったら、ポインタをそのまま渡すだけじゃダメ
	// → その場合ポインタの値渡しになっているので、変更されない
	// → ポインタのポインタを渡すと、参照渡しとなる

	// というかまとめると、デリファレンスして変更すると、関数の呼び出し元でも値が変更されるみたいやな〜

	return 0;
}