#!/bin/bash

test() {
	input="$1"
	expected="$2"

	./isakacc "$input" > tmp.s || exit
	gcc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $expected"
	else
		echo "$input => $expected expected, but got $actual"
	fi
}

test '3+3;' 6
test '3 + 4 - 10 +     20;' 17
test '10; 20;' 20
test '10+20-3; 10;' 10
test '30-27;' 3
test '10;' 10
test '10+20-3;' 27
test '10 + 30 - 3 - 3;' 34
test '(10 + 20) + 3;' 33
test '(10 + 20) * 3;' 90
test '10 + 10 * 2;' 30
test '10 / 5 + 2 * (2 - 1);' 4
test '- - - -10;' 10
test '- 20 + 30 / 3 - - 10;' 0
test '1<10;' 1
test '10 < 10;' 0
test '9 < -10;' 0
test '1>10;' 0
test '10 > 10;' 0
test '9 > -10;' 1
test '9 > 1;' 1
test '10 <= 10;' 1
test '10 >= 10;' 1
test '10 <= 9;' 0
test '9 >= 10;' 0
test '10 <= -11;' 0
test '-11 >= 10;' 0
test 'a=3; b=4; a+b;' 7
test 'a=- - -3; b = 3 + -2 + 10 * 2 / (10/2); b-a;' 8
test 'a=10; z = 3; y = a*z; y;' 30
test 'a=10; (c=3); a+c;' 13
test 'abc=10; abc;' 10
test 'bar333444=3; _foo_898=5; bar333444+_foo_898;' 8

echo OK