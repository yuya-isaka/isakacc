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
assert 5 '10-5'
assert 10 '30+20-10-30'
assert 5 '30 +     55 - 42 - 38'
assert 50 '30 +          10   +10'
assert 25 '10 + 5 * 3'
assert 73 '10 * (3 + 4) + 3'
assert 16 '10 + (4 / 2) * 3'

echo OK