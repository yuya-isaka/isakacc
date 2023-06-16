#!/bin/bash

test() {
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
		exit 1
	fi
}

test 0 'return 0;'
test 42 'return 42;'
test 21 'return 5+20-4;'
test 41 'return  12 + 34 - 5 ;'
test 47 'return 5+6*7;'
test 15 'return 5*(9-6);'
test 4 'return (3+5)/2;'
test 10 'return -10+20;'
test 10 'return - -10;'
test 10 'return - - +10;'

test 0 'return 0==1;'
test 1 'return 42==42;'
test 1 'return 0!=1;'
test 0 'return 42!=42;'

test 1 'return 0<1;'
test 0 'return 1<1;'
test 0 'return 2<1;'
test 1 'return 0<=1;'
test 1 'return 1<=1;'
test 0 'return 2<=1;'

test 1 'return 1>0;'
test 0 'return 1>1;'
test 0 'return 1>2;'
test 1 'return 1>=0;'
test 1 'return 1>=1;'
test 0 'return 1>=2;'

test 3 'a=3; return a;'
test 8 'a=3; z=5; return a+z;'

test 3 'a=3; return a;'
test 8 'a=3; z=5; return a+z;'
test 6 'a=b=3; return a+b;'
test 3 'foo=3; return foo;'
test 8 'foo123=3; bar=5; return foo123+bar;'

test 1 'return 1; 2; 3;'
test 2 '1; return 2; 3;'
test 3 '1; 2; return 3;'

echo OK