#!/bin/bash

assert() {
	expected="$1"
	input="$2"

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

assert 10 10
assert 150 150
assert 39 '10+30-1'
assert 12 '  10   - 3   +3+2'
assert 65 '   (10  +3) *5'
assert 8 '  10 * ((20 / 5) * (10 + 10)) / 100'
assert 10 '- - - - 10'
assert 20 ' -10 + 20 - - 10'
assert 1 '10 == 10'
assert 0 '10 == 11'
assert 1 '10 != 9'
assert 1 '10 < 11'
assert 0 '10 > 11'
assert 1 '10 >= 10'
assert 1 '10 <= 10'
assert 1 '9 <= 10'
assert 0 '9 >= 10'

assert 0 '0==1'
assert 1 '42==42'
assert 1 '0!=1'
assert 0 '42!=42'
assert 1 '0<1'
assert 0 '1<1'
assert 0 '2<1'
assert 1 '0<=1'
assert 1 '1<=1'
assert 0 '2<=1'
assert 1 '1>0'
assert 0 '1>1'
assert 0 '1>2'
assert 1 '1>=0'
assert 1 '1>=1'
assert 0 '1>=2'