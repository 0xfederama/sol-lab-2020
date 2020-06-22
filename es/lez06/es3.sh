#!/bin/bash

if [ $# -ne 2 ]; then
	echo "usage: $(basename $0) file1 file2"
	exit 1
fi

f1=$1
f2=$2

echo "Choose a number from the options available:
1- Remove $f1 and $f2
2- Archive $f1 and $f2 in a single archive
3- Append $f1 to $f2
4- Exit"

read num

case $num in
	1) 	
		echo "Are you sure you want to remove $f1 and $f2? [y/n]"
		read yn
		if [ $yn == "y" ]; then
			rm -f $f1 $f2
		fi
		exit 0
		;;
	2)	
		b1=$(basename $f1)
		b2=$(basename $f2)
		name=${b1%.*}
		name+=${b2%.*}
		echo "Compressing files"
		tar cf - $f1 $f2 | gzip > $name.tar.gz 2> /dev/null
		if [ $? -ne 0 ]; then
			echo "Error compressing files"
			exit 1
		fi
		exit 0
		;;
	3)
		echo "Appending $f1 to $f2"
		cat $f1 >> $f2
		if [ $? -ne 0 ]; then
			echo "Error concatenating files"
			exit 1
		fi
		exit 0
		;;
	4)
		exit 0
		;;
	*) 
		echo "You have to choose a number between 1 and 4"
		exit 0
		;;
esac
