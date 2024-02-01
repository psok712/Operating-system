#!/usr/bin/bash

# Иллюстрация вызова функции из функции (каждый вызов функции увеличивает значение переданной переменной)

func1() {
	echo "Function 1"
	echo $1
	func2 $[$1+1]
}

func2() {
	echo "Function 2"
	echo $1
	func3 $[$1+1]
}

func3() {
	echo "Function 3"
	echo $1
}

func1 $1
