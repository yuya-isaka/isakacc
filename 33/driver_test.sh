#!/bin/bash

tmp=$(mktemp -d /tmp/isakacc-test-XXXXXX)
trap 'rm -rf $tmp' INT TERM HUP EXIT
echo > $tmp/empty.c

check() {
	if [ $? -eq 0 ]; then
		echo "testing $1 ok"
	else
		echo "testing $1 failed"
		exit 1
	fi
}

rm -f $tmp/out
./isakacc -o $tmp/out $tmp/empty.c
[ -f $tmp/out ]
check -o

./isakacc --help 2>&1 | grep -q isakacc
check --help

echo OK