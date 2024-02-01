#!/usr/bin/bash

# Выводит на консоль все переданные аргументы

print() {
	for arg in "$@"; do
		echo "$arg"
	done
}

print "$@"
