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

assert 10 '10;'
assert 150 '150;'
assert 37 '10 + 30 -3;'
assert 24 '10+20-3-3;'
assert 150 '(10 + 20) * 5;'
assert 19 '10 +3 *3;'
assert 20 '30 - 10 /5 * (3+2);'
assert 20 '- - - - 20;'
assert 31 '- + - 30 + - 3 - - 4;'
assert 7 '- 30 + 20 - - 10 + -3 -- 10;'
assert 1 '1 < 10;'
assert 0 '10 < 10;'
assert 0 '10 < -11;'
assert 0 '10 > 10;'
assert 1 '10 > 9;'
assert 0 '-10 > 9;'
assert 1 '10 >= 9;'
assert 1 '10 >= 10;'
assert 0 '10 >= 11;'
assert 1 '10 <= 11;'
assert 1 '11 <= 11;'
assert 0 '12 <= 11;'
assert 7 'a=3; b=4; a+b;'
assert 18 'a=1; b=3+4; c=4-1; d = 10-3+2-2*1/2; z = c*d; u = z - b; y = a + u; y;'
assert 21 'a=b=c=4+3; a+b+c;'