CC = gcc
CFLAGS = -lpthread

DBGFLAG = -DDEBUG

.PHONY: all test2 clean

all: supermarket

#Compilo la libreria, la linko al file supermarket.c e compilo l'eseguibile
supermarket:
	@echo "Compiling library and source code"
	$(CC) mylib.c -c -o mylib.o
	ar rvs libsm.a mylib.o
	$(CC) supermarket.c $(CFLAGS) -o supermarket -L. -lsm

#Esegue il test2
test2:
	@echo "Running supermarket for 25 seconds"
	(./supermarket & echo $$! > sm.PID) &
	sleep 25
	kill -HUP $$(cat sm.PID)
	@echo "\nSupermarket is closing"
	chmod +x ./analisi.sh
	./analisi.sh
	@echo "\nTest done"

#Rimuove i file compilati, il log e il file in cui viene salvato il PID del supermarket
clean:
	@echo "Deleting useless files"
	-rm -f supermarket sm.PID mylib.o libsm.a supermarket.log