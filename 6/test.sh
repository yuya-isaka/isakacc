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
assert 20 20
assert 0 '10+20-30'
assert 18 '10 -  30 +40 -2'
assert 15 '(2+3)*3'
assert 35 '2*(3-2)*5/2*(2+5)'
assert 1 '-2+3'
assert 2 '- - -3 + 5'
assert 8 '+++++3 + 3 - - 2'