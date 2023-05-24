#!/bin/bash

assert() {
	expected="$1"
	input="$2"

	./isakacc "$input" > tmp.s || exit
	gcc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but got $actual"
	fi
}

assert 10 10
assert 50 50
assert 16 '3+5-2+10'
assert 6 '3 +    5    -2'
assert 16 '( 3   +5)*2'
assert 15 '3  + 4 * 3'
assert 6 '3 - (3  + 4) * 3 / 3 + 10'
assert 10 ' -10+20'
assert 20 ' - - 20'
assert 30 '- - - - + 30'