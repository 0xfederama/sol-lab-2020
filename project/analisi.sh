#!/bin/bash

PID=$(cat sm.PID)
LOG="supermarket.log"

while [ -e /proc/$PID ]; do
	sleep 0.5
done

if [ -f $LOG ]; then
	while read line; do 
		echo $line
	done < $LOG
else
	echo "Error: logfile not found"
fi
