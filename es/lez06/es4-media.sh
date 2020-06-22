#!/bin/bash

if [ $# -eq 0 ]; then
	echo "No .dat files passed as arguments"
	exit 1
fi

for f ; do

    sum=0   # conterra' la somma degli elementi 
    std=0   # standard deviation
    cnt=0   # contatore 
    values=() # array dei valori
	exec 3<$f
	while IFS=" " read -u 3 line; do
		read -r -a elem <<< $line
		sum=$(echo "scale=2; $sum+${elem[1]}" | bc -q)
		values[$cnt]=${elem[1]}
		(( cnt += 1))
    done
    exec 3<&-  # chiudo il descrittore 3 
    avg=$(echo "scale=2; $sum/$cnt" | bc -q)
    sum=0
    for((i=0;i<$cnt; i++)); do
		sum=$(echo "scale=2; $sum+(${values[$i]}-$avg)^2" | bc -q)
    done
    if [ $cnt -gt 1 ]; then
		std=$(echo "scale=2; sqrt($sum/($cnt-1))" | bc -q)
    fi
    f1=${f%.???}   # tolgo l'estensione
    #f2=${f1##*/}   # tolgo il path se presente
    f2=$(basename $f1)
    # si usa awk solo per avere un output formattato sfruttando il suo printf
    # la stampa con echo va bene ugualmente
    # echo $f2 " " $cnt " " $avg " " $std 
    echo "$f2 $cnt $avg $std"

done
