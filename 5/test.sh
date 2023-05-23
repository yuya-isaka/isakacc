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
assert 31 '10-3+4+20'
assert 23 '  10    +20-3    -4'
assert 9 '3+3*2'
assert 12 '(3+3)*2'
assert 3 '3-3*2+6'
assert 6 '(3-3)*2+6'