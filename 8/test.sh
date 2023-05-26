#!/bin/bash

assert() {
	expected="$1"
	input="$2"

	./isakacc "$input" > tmp.s || exit
	gcc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$expected" = "$actual" ]; then
		echo "$input => $expected"
	else
		echo "$input => $expected expected, but got $actual"
	fi
}

assert 10 10
assert 150 150
assert 31 '10+20-3+4'
assert 27 '10  -3     +20'
assert 90 '(10+20)*3'
assert 9 '3+2*2+4/2'
assert 15 '(3  +  2)    *(2+4)    /2'
assert 10 '- - 10'
assert 3 '-10 + 3 - - 3 + + + 10 - - - 3'
assert 1 ' 1 < 10'
assert 0 '1> 10'
assert 1 '10 <= 10'
assert 1 '10 >= 10'
assert 0 '10 <= 9'
assert 0 '9 >= 10'
assert 1 '9 <= 10'

echo OK