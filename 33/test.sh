#!/bin/bash

cat <<EOF | gcc -xc -c -o tmp2.o -
int ret3() { return 3; }
int ret5() { return 5; }
int add(int x, int y) { return x+y; }
int sub(int x, int y) { return x-y; }
int add6(int a, int b, int c, int d, int e, int f) {
  return a+b+c+d+e+f;
}
EOF

assert() {
	expected="$1"
	input="$2"

	# ./isakacc "$input"
	./isakacc "$input" > tmp.s || exit
	gcc -o tmp tmp.s tmp2.o
	./tmp
	actual="$?"

	if [ "$expected" = "$actual" ]; then
		echo "$input => $actual"
	else
		echo "$input => $expected expected, but got $actual"
		exit 1
	fi
}

assert 0 'int main() { return 0; }'
assert 42 'int main() { return 42; }'
assert 21 'int main() { return 5+20-4; }'
assert 41 'int main() { return  12 + 34 - 5 ; }'
assert 47 'int main() { return 5+6*7; }'
assert 15 'int main() { return 5*(9-6); }'
assert 4 'int main() { return (3+5)/2; }'
assert 10 'int main() { return -10+20; }'
assert 10 'int main() { return - -10; }'
assert 10 'int main() { return - - +10; }'

assert 0 'int main() { return 0==1; }'
assert 1 'int main() { return 42==42; }'
assert 1 'int main() { return 0!=1; }'
assert 0 'int main() { return 42!=42; }'

assert 1 'int main() { return 0<1; }'
assert 0 'int main() { return 1<1; }'
assert 0 'int main() { return 2<1; }'
assert 1 'int main() { return 0<=1; }'
assert 1 'int main() { return 1<=1; }'
assert 0 'int main() { return 2<=1; }'

assert 1 'int main() { return 1>0; }'
assert 0 'int main() { return 1>1; }'
assert 0 'int main() { return 1>2; }'
assert 1 'int main() { return 1>=0; }'
assert 1 'int main() { return 1>=1; }'
assert 0 'int main() { return 1>=2; }'

assert 3 'int main() { int a; a=3; return a; }'
assert 3 'int main() { int a=3; return a; }'
assert 8 'int main() { int a=3; int z=5; return a+z; }'

assert 1 'int main() { return 1; 2; 3; }'
assert 2 'int main() { 1; return 2; 3; }'
assert 3 'int main() { 1; 2; return 3; }'

assert 3 'int main() { int a=3; return a; }'
assert 8 'int main() { int a=3; int z=5; return a+z; }'
assert 6 'int main() { int a; int b; a=b=3; return a+b; }'
assert 3 'int main() { int foo=3; return foo; }'
assert 8 'int main() { int foo123=3; int bar=5; return foo123+bar; }'

assert 3 'int main() { if (0) return 2; return 3; }'
assert 3 'int main() { if (1-1) return 2; return 3; }'
assert 2 'int main() { if (1) return 2; return 3; }'
assert 2 'int main() { if (2-1) return 2; return 3; }'

assert 55 'int main() { int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
assert 3 'int main() { for (;;) return 3; return 5; }'

assert 10 'int main() { int i=0; while(i<10) i=i+1; return i; }'

assert 3 'int main() { {1; {2;} return 3;} }'
assert 5 'int main() { ;;; return 5; }'

assert 10 'int main() { int i=0; while(i<10) i=i+1; return i; }'
assert 55 'int main() { int i=0; int j=0; while(i<=10) {j=i+j; i=i+1;} return j; }'

assert 3 'int main() { int x=3; return *&x; }'
assert 3 'int main() { int x=3; int *y=&x; int **z=&y; return **z; }'
assert 5 'int main() { int x=3; int y=5; return *(&x+1); }'
assert 3 'int main() { int x=3; int y=5; return *(&y-1); }'
assert 3 'int main() { int x=3; int y=5; return *(-1+&y); }'
assert 5 'int main() { int x=3; int y=5; return *(&x-(-1)); }'
assert 5 'int main() { int x=3; int *y=&x; *y=5; return x; }'
assert 7 'int main() { int x=3; int y=5; *(&x+1)=7; return y; }'
assert 7 'int main() { int x=3; int y=5; *(&y-2+1)=7; return x; }'
assert 5 'int main() { int x=3; return (&x+2)-&x+3; }'
assert 8 'int main() { int x, y; x=3; y=5; return x+y; }'
assert 8 'int main() { int x=3, y=5; return x+y; }'

assert 3 'int main() { return ret3(); }'
assert 5 'int main() { return ret5(); }'
assert 8 'int main() { return add(3, 5); }'
assert 2 'int main() { return sub(5, 3); }'
assert 21 'int main() { return add6(1,2,3,4,5,6); }'
assert 66 'int main() { return add6(1,2,add6(3,4,5,6,7,8),9,10,11); }'
assert 136 'int main() { return add6(1,2,add6(3,add6(4,5,6,7,8,9),10,11,12,13),14,15,16); }'

assert 32 'int main() { return ret32(); } int ret32() { return 32; }'
assert 7 'int main() { return add2(3,4); } int add2(int x, int y) { return x+y; }'
assert 1 'int main() { return sub2(4,3); } int sub2(int x, int y) { return x-y; }'
assert 55 'int main() { return fib(9); } int fib(int x) { if (x<=1) return 1; return fib(x-1) + fib(x-2); }'

# # ポインタや配列を関数に渡すのもできている．
assert 6 'int main() { int x = 3; return addadd(&x,&x); } int addadd(int *x, int *y) { return *x+*y; }'
assert 11 'int main() { int x[2]; *x=5;  x[1]=6; return addadd(&x[0] ,&x[1]); } int addadd(int *x, int *y) { return *x+*y; }'
assert 11 'int main() { int x[2]; *x=5;  x[1]=6; return addadd(x[0] ,x[1]); } int addadd(int x, int y) { return x+y; }'
assert 11 'int main() { int x[2]; *x=5;  x[1]=6; return addadd(x ,x[1]); } int addadd(int *x, int y) { return *x+y; }'
assert 7 'int main() { char x[10]; x[0] = 3; x[1] = 4; return print(x); } int print(char *x) { return x[0] + x[1]; }'
assert 3 'int main() { char x; x = 3; return print(x); } int print(char x) { return x; }'
assert 3 'int main() { int y; int *x = &y; *x = 3; return print(x); } int print(int *x) { return *x; }'
assert 3 'int main() { char y; char *x = &y; *x = 3; return print(x); } int print(char *x) { return *x; }'
# # ポインタを初期化せずに使うとバグる
# # assert 3 'int main() { int *x; *x = 3; return print(x); } int print(int *x) { return *x; }'
# # assert 3 'int main() { char *x; *x = 3; return print(x); } int print(char *x) { return *x; }'
# # 文字列をcharはこういう形で格納できる
assert 97 'int main() { char y = "a"[0]; return y; }'
assert 97 'int main() { char y = *"a"; return y; }'
# # ↓ []はポインタであれば使える，ただデリファレンスしているだけだから
assert 3 'int main() { int x = 3; int *y = &x; return y[0]; }'
# # 文字列をcharに格納はまだできない？
# # これは文字列がvarの配列なので，デリファレンスしないといけない
# # assert 97 'int main() { char y = "a"; return y; }'
# # ↓ こう書くといい
assert 97 'int main() { char *y = "a"; return *y; }'
assert 97 'int main() { char *y = "a"; return y[0]; }'
assert 97 'int main() { char *y = "a";  char *x = y; return *x; }'
# # ↓ できちゃダメだけど，こういう書き方もできる．"a"はアドレスを返すから，intでそれをアドレスとして扱うことで，実現できる（現状intとポインタは同じサイズなので可能）
assert 97 'int main() { int y = "a";  char *x = y; return *x; }'
# # assert 97 'int main() { char y; char *x = &y; *x = "a"; return print(x); } int print(char *x) { return *x; }'
# # ポインタを配列0でアクセスできる
assert 3 'int main() { int x = 3; int *y = &x; return y[0]; }'


assert 3 'int main() { int x[2]; int *y=&x; *y=3; return *x; }'
assert 3 'int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *x; }'
assert 4 'int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *(x+1); }'
assert 5 'int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *(x+2); }'
assert 5 'int main() { int x[3]; *x=3; *(x+1)=4; *(x+2)=5; return *(x+2); }'

assert 3 'int main() { int x[2][3]; int *y=x; *y=3; return **x; }'

# # ↓ 配列への代入を試みようとしているのでだめ
# # assert 3 'int main() { int x[2][3]; *x=3; return x[0][0]; }'
assert 3 'int main() { int x[2][3]; **x=3; return x[0][0]; }'
assert 3 'int main() { int x[2][3]; *x[0]=3; return x[0][0]; }'

# # ↓ これはなぜ配列自体への代入じゃないの？ → 暗黙の型変換？ で xはx[0]のポインタとなるはずがx[0][0]のポインタになってる
assert 3 'int main() { int x[2][3]; int *y=x; *y=3; return x[0][0]; }'
assert 3 'int main() { int x[2][3]; int *y=*x; *y=3; return x[0][0]; }'
assert 3 'int main() { int x[2]; int *y=x; *y=3; return x[0]; }'
# # assert 3 'int main() { int x[2][3]; int *y=x; **y=3; return x[0][0]; }'

assert 1 'int main() { int x[2][3]; int *y=x; *(y+1)=1; return *(*x+1); }'
assert 2 'int main() { int x[2][3]; int *y=x; *(y+2)=2; return *(*x+2); }'
assert 3 'int main() { int x[2][3]; int *y=x; *(y+3)=3; return **(x+1); }'
assert 4 'int main() { int x[2][3]; int *y=x; *(y+4)=4; return *(*(x+1)+1); }'
assert 5 'int main() { int x[2][3]; int *y=x; *(y+5)=5; return *(*(x+1)+2); }'

assert 3 'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *x; }'
assert 4 'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+1); }'
assert 4 'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return x[1]; }'
assert 5 'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+2); }'
assert 5 'int main() { int x[3]; *x=3; x[1]=4; x[2]=5; return *(x+2); }'
assert 5 'int main() { int x[3]; *x=3; x[1]=4; 2[x]=5; return *(x+2); }'
assert 5 'int main() { int x[3]; *x=3; x[1]=4; 2[x]=5; return x[2]; }'
assert 3 'int main() { int x[2][3]; int *y=x; y[0]=3; return x[0][0]; }'
assert 1 'int main() { int x[2][3]; int *y=x; y[1]=1; return x[0][1]; }'
assert 2 'int main() { int x[2][3]; int *y=x; y[2]=2; return x[0][2]; }'
assert 3 'int main() { int x[2][3]; int *y=x; y[3]=3; return x[1][0]; }'
assert 4 'int main() { int x[2][3]; int *y=x; y[4]=4; return x[1][1]; }'
assert 5 'int main() { int x[2][3]; int *y=x; y[5]=5; return x[1][2]; }'
assert 5 'int main() { int x[2][3]; int *y=x; 5[y]=5; return x[1][2]; }'
## not an lvalue assert 5 'int main() { int x[2][3]; *x=5; return x[1][2]; }'
assert 5 'int main() { int x[2][3]; *x[0]=5; return x[0][0]; }'
assert 5 'int main() { int x[2]; int *y = &x; y[0] = 5; return x[0]; }'
assert 5 'int main() { int x[2]; int *y = &x; y[0] = 5; return *x; }'

assert 8 'int main() { int x; return sizeof(x); }'
assert 8 'int main() { int x; return sizeof x; }'
assert 8 'int main() { int *x; return sizeof(x); }'
assert 32 'int main() { int x[4]; return sizeof(x); }'
assert 96 'int main() { int x[3][4]; return sizeof(x); }'
assert 32 'int main() { int x[3][4]; return sizeof(*x); }'
assert 8 'int main() { int x[3][4]; return sizeof(**x); }'
assert 9 'int main() { int x[3][4]; return sizeof(**x) + 1; }'
assert 9 'int main() { int x[3][4]; return sizeof **x + 1; }'
assert 8 'int main() { int x[3][4]; return sizeof(**x + 1); }'
assert 8 'int main() { int x=1; return sizeof(x=2); }'
assert 1 'int main() { int x=1; sizeof(x=2); return x; }'
# assert 0 'int main[3] { return 0; }' 現状行けてまう　いけなくなった．最初の時点でTY_FUNCか確認するから

assert 0 'int x; int main() { return x; }'
assert 3 'int x; int main() { x=3; return x; }'
assert 7 'int x; int y; int main() { x=3; y=4; return x+y; }'
assert 7 'int x, y; int main() { x=3; y=4; return x+y; }'
assert 0 'int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[0]; }'
assert 1 'int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[1]; }'
assert 2 'int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[2]; }'
assert 3 'int x[4]; int main() { x[0]=0; x[1]=1; x[2]=2; x[3]=3; return x[3]; }'

assert 8 'int x; int main() { return sizeof(x); }'
assert 32 'int x[4]; int main() { return sizeof(x); }'

assert 1 'int main() { char x=1; return x; }'
assert 1 'int main() { char x=1; char y=2; return x; }'
assert 2 'int main() { char x=1; char y=2; return y; }'

assert 1 'int main() { char x; return sizeof(x); }'
assert 10 'int main() { char x[10]; return sizeof(x); }'
assert 1 'int main() { return sub_char(7, 3, 3); } int sub_char(char a, char b, char c) { return a-b-c; }'

assert 0 'int main() { return ""[0]; }'
assert 1 'int main() { return sizeof(""); }'

assert 97 'int main() { return "abc"[0]; }'
assert 98 'int main() { return "abc"[1]; }'
assert 99 'int main() { return "abc"[2]; }'
assert 4 'int main() { return sizeof("abc"); }'
assert 0 'int main() { return "abc"[3]; }'

assert 7 'int main() { return "\a"[0]; }'
assert 8 'int main() { return "\b"[0]; }'
assert 9 'int main() { return "\t"[0]; }'
assert 10 'int main() { return "\n"[0]; }'
assert 11 'int main() { return "\v"[0]; }'
assert 12 'int main() { return "\f"[0]; }'
assert 13 'int main() { return "\r"[0]; }'
assert 27 'int main() { return "\e"[0]; }'

assert 106 'int main() { return "\j"[0]; }'
assert 107 'int main() { return "\k"[0]; }'
assert 108 'int main() { return "\l"[0]; }'

assert 7 'int main() { return "\ax\ny"[0]; }'
assert 120 'int main() { return "\ax\ny"[1]; }'
assert 10 'int main() { return "\ax\ny"[2]; }'
assert 121 'int main() { return "\ax\ny"[3]; }'

assert 0 'int main() { return "\0"[0]; }'
assert 16 'int main() { return "\20"[0]; }'
assert 65 'int main() { return "\101"[0]; }'
assert 104 'int main() { return "\1500"[0]; }'

assert 0 'int main() { return "\x00"[0]; }'
assert 119 'int main() { return "\x77"[0]; }'
assert 165 'int main() { return "\xA5"[0]; }'
assert 255 'int main() { return "\x00ff"[0]; }'

assert 0 'int main() { return ({ 0; }); }'
assert 2 'int main() { return ({ 0; 1; 2; }); }'
assert 1 'int main() { ({ 0; return 1; 2; }); return 3; }'
# # ↓ 文式では，最後の値を返す．returnだと何も返さない（codegen だとただjmpして終わるコードになる）のでだめ
# # つまりexpr_stmtしか許さない（これだけ唯一文だけど式としても扱えて，値をraxに入れる）
# assert 2 'int main() { ({ 0; 1; return 2; }); return 3; }'
assert 6 'int main() { return ({ 1; }) + ({ 2; }) + ({ 3; }); }'
assert 3 'int main() { return ({ int x=3; x; }); }'

echo OK