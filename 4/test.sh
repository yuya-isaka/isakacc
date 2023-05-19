#!/bin/bash

assert() {
	expected="$1"
	input="$2"

	./isakacc "$input" > tmp.s
	gcc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but got $actual"
	fi
}

assert 0 0
assert 5 5
assert 120 120
assert 10 '10-10+10'
assert 5 '20+3-4-14'
assert 20 ' 30 -3 - 4 + 2 - 5'
assert 30 '30*'

echo OK