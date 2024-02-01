#!/usr/bin/bash

# Считает сумму от 1 до n, где n - введенное с клавиатуры число

start=0
end=$1
sum=0

if [ $end -le 0 ]; then
	echo "К сожалению, вы ввели некорректное число"
else
	while [ $start -ne $end ]
	do
		(( start++ ))
		sum=$[$sum + $start]
	done
fi

echo $sum
