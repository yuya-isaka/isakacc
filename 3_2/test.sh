#!/bin/bash

assert () {
	expected="$1"
	input="$2"

	./isakacc "$input" > tmp.s || exit
	gcc -static -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$expected" = "$actual" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but got $actual"
	fi
}

assert 0 0
assert 55 55
assert 3 '12+3-12'
assert 10 '30  +2 - 3 + 4 - 23'

echo OK