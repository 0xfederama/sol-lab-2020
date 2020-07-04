#!/bin/bash

PID=$(cat sm.PID)

while [ -e /proc/$PID ]; do
	sleep 0.5
done

if [ -f "supermarket.log" ]; then
	cat supermarket.log
else
	echo "Error: logfile not found"
fi
