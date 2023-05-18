#!/bin/bash

assert() {
	e="$1"
	i="$2"

	./isakacc "$i" > tmp.s
	cc -o tmp tmp.s
	./tmp
	a="$?"

	if [ "$a" = "$e" ]; then
		echo "$i => $e"
	else
		echo "$i => $e expected, but got $a"
	fi
}

assert 0 0
assert 50 50
assert 10 '3 + 7 - 5 + 5 + 12 - 10 - 2'
assert 5 '10 - 5'

echo OK