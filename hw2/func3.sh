#!/usr/bin/bash

# Выполняет линейный поиск переданной строки в хранящемся массиве

linear_search() {
	local target=$1
	array=("pen" "pineapple" "apple" "not_pen")
	for el in "${array[@]}"; do
		if [[ "$el" == "$target" ]]; then
			echo "\"$target\" in the array!"
			return 
		fi
	done
	echo "There is no \"$target\" in the array("
}

linear_search $1
