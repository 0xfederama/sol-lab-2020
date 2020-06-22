CC = gcc
CFLAGS = -Wall -O3

.PHONY: all exec test cleanout

all: cleanout exec test

#Generazione eseguibili da .c
exec: mycp.c myfcp.c
	$(CC) $(CFLAGS) mycp.c -o mycp && $(CC) $(CFLAGS) myfcp.c -o myfcp

test: 	
	@echo 
	@echo "Running test with syscall"
	./mycp test.txt testout.txt
	@diff test.txt testout.txt
	@echo "Syscall test OK"
	@echo 
	@echo "Running test with stdio call"
	./myfcp test.txt testfout.txt
	@diff test.txt testfout.txt
	@echo "Standard test OK"
	@echo
	@echo "Running test script to check time"
	./test.sh ./mycp
	./test.sh ./myfcp
	#gnuplot plot.gnp

cleanout: 
	rm -f *.dat filein fileout *out.txt

