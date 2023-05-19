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
assert 52 52
assert 10 '30-20'
assert 5 ' 10 - 3 + 2 -4'

echo OK