#!/bin/bash

if [ $# -ne 1 ]; then
    exit -1
fi


OUT=$1
#OUT="/dev/null"

k=0
./client "ciao!" "Hello" "Hi"    >&      $OUT &
PID[k]=$! 
((k++))
./client "prova1" "prova2"       2>&1 >> $OUT & 
PID[k]=$! 
((k++))
./client "altra prova"           2>&1 >> $OUT & 
PID[k]=$! 
((k++))
./client "penultimo MESSAGGIO!"  2>&1 >> $OUT & 
PID[k]=$! 
((k++))
./client "Arrivederci" "goodbye" "au Revoir" "hasta luego"  2>&1 >> $OUT &
PID[k]=$! 
((k++))
./client "altra prova"           2>&1 >> $OUT & 
PID[k]=$! 
((k++))
./client "altra prova"           2>&1 >> $OUT & 
PID[k]=$! 
((k++))
./client "altra prova"           2>&1 >> $OUT & 
PID[k]=$! 
((k++))
./client "Messaggio lungo aaaaaaaaaaa" "bbbb  bbbbbbbbbbbbb     bbbbbbbbbbbbbbbbbbbbbbb bbbbbbbbbbbbbbbbbb" "au Revoir" "hasta luego"  " cccc ccccc " " dddddd d d d d d d ddddddd "  "eeeee eeee eeee eeeeeeeeeee e e e e" 2>&1 >> $OUT
PID[k]=$! 
((k++))
./client "altra prova"           2>&1 >> $OUT & 
PID[k]=$! 
((k++))
./client "altra prova"           2>&1 >> $OUT & 
PID[k]=$! 
((k++))
./client "altra prova"           2>&1 >> $OUT &
PID[k]=$! 
((k++))

for((i=0;i<k;++i)); do
    wait ${PID[i]}
done

