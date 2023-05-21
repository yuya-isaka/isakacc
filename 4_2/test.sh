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
		echo "$input => $expected, expected but got $actual"
	fi
}

assert 1 1
assert 10 10
assert 5 '10+30-35'
assert 10 '10 - 30 + 65 - 35'
# assert 5 ' 10 * 30'
# assert 5 ' 10 -'

echo OK