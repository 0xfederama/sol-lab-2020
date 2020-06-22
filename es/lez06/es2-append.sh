#!/bin/bash

if [ $# -lt 2 ]; then
	echo "usage: $(basename $0) file2 file1 fileout"
	exit -1
fi

args=("$@") 	#Creo array con argomenti in ingresso
numarg=$#
out=${args[$((numarg-1))]}

for ((i=((numarg-2)); i>=0; --i)); do
	cat ${args[$i]} >> $out
	if [ $? -ne 0 ]; then
		echo "Errore in cat file1 >> fileout"
		exit 1
	fi
done
