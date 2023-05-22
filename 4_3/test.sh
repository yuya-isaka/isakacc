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

assert 10 10
assert 150 150
assert 10 '30-5+20-35'
assert 20 '  50 + 30 - 20 - 40'

echo OK