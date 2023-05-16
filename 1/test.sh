#!/bin/bash

assert() {
	expected="$1"
	input="$2"

	./isakacc "$input" > tmp.s
	cc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$expected" = "$actual" ]; then
		echo "$input => $expected"
	else
		echo "$input => $expected expected, but got $actual"
		exit 1
	fi
}

assert 1 1
assert 42 42

echo OK