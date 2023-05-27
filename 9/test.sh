#!/bin/bash

assert() {
	expected="$1"
	input="$2"

	./isakacc "$input" > tmp.s || exit
	gcc -o tmp tmp.s
	./tmp

	actual="$?"

	if [ "$expected" = "$actual" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but got $actual"
	fi
}

assert 13 '10; 13;'
assert 35 '150; 10; 13; 15; 16; 35;'
assert 27 '10; 20; 10+20-3;'
assert 22 '10  - 3   + 3; 11 + 11;'
assert 39 '(10+3)*3;'
assert 17 '10+3*3-2;'
assert 3 '(11 + 4) * 5 / (5 * (3 + 2));'
assert 10 '30; - - - -10;'
assert 8 '-10 + 5 - - 3 - - - - 10;'
assert 1 '11 < 10; 3 < 10;'
assert 0 '11 < 10;'
assert 1 '10 > 1;'
assert 0 '10 > 11;'
assert 1 '10 >= 10;'
assert 1 '10 >= 9;'
assert 0 '8 >= 9;'
assert 1 '9 <= 9;'
assert 0 '9 <= 8;'
assert 1 '7 <= 8;'
assert 1 '1 == 1;'
assert 0 '1 == 0;'
assert 1 '1 != 0;'
assert 0 '1 != 1;'

echo OK