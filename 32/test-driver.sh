#!/bin/bash

tmp=$(mktemp -d /tmp/isakacc-test-XXXXXX)
trap 'rm -rf $tmp' INT TERM HUP EXIT
echo > $tmp/empty.c

check() {
	if [ $? -eq 0 ]; then
		echo "testing $1 ... passed"
	else
		echo "testing $1 ... failed"
		exit 1
	fi
}

# -o
rm -f $tmp/out
./isakacc -o $tmp/out $tmp/empty.c
[ -f $tmp/out ]
check -o

# --help
./isakacc --help 2>&1 | grep -q isakacc
check --help

echo OK

# バッククウォートと$()はコマンド置換
# trapの'は例外で，シグナルが発生したとき文字列をコマンドとして解釈して実行する

# 一時ディレクトリを作成 XXXXXXはランダムな文字列となり，結果のパスがtmpに保存

# trapコマンドでこのスクリプトが終了したとき（特定のシグナルを受け取ったとき）一時ディレクトリを削除
# ↓ シグナル
# INT 中断
# TERM 終了
# HUP ハングアップ
# EXIT 終了

# echoコマンドは何も引数を指定しないと空行を出力
# リダイレクトを使って出力を一時ディレクトリ内のファイルに保存

# -eq  = は 数値比較か文字列比較か

# outが存在し，それがファイルであることをテスト
# 終了ステータスに結果を保存

# --help
# 標準エラー出力も標準出力にリダイレクトする
# grep -qは検索結果があればただすぐに終了することを示す