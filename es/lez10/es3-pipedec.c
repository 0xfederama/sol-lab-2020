#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define SYSCALL(sc, ret, string) \
	if ((ret=sc)==-1) {	perror(string);	return ret;	}

int main (int argc, char*argv[]) {
	
	if (argc!=2) {
		fprintf(stderr, "Usage: %s <num>\n", argv[0]);
		return -1;
	}

	//Creo le pipe per i figli
	int dec1[2];
	int dec2[2];
	int checkerr;
	SYSCALL(pipe(dec1), checkerr, "Error opening first pipe");
	SYSCALL(pipe(dec2), checkerr, "Error opening second pipe");

	//Primo figlio, prende il parametro in input
	if (fork()==0) {
		//Chiudo dec1[0] e dec2[1] che non mi servono
		SYSCALL(close(dec2[1]), checkerr, "Error closing dec1[1]");
		SYSCALL(close(dec1[0]), checkerr, "Error closing dec2[0]");
		//Redirigo input su dec1[0] e output su dec2[1]
		SYSCALL(dup2(dec2[0], 0), checkerr, "Error redirecting stdin");
		SYSCALL(dup2(dec1[1], 1), checkerr, "Error redirecting stdout");

		//Eseguo execl
		execl("./dec", "dec", argv[1], NULL);
		perror("Error in execl");
		return -1;
	}

	//Secondo figlio, non prende input
	if (fork()==0) {
		//Chiudo dec1[1] e dec2[0] che non mi servono
		SYSCALL(close(dec1[1]), checkerr, "Error closing dec1[0]");
		SYSCALL(close(dec2[0]), checkerr, "Error closing dec2[1]");
		//Redirigo input su dec1[0] e output su dec2[1]
		SYSCALL(dup2(dec1[0], 0), checkerr, "Error redirecting stdin");
		SYSCALL(dup2(dec2[1], 1), checkerr, "Error redirecting stdout");

		//Eseguo execl
		execl("./dec", "dec", NULL);
		perror("Error in execl");
		return -1;
	}

	//Aspetto figli, chiudo tutti i fd e esco
	SYSCALL(close(dec1[0]), checkerr, "Error closing fd");
	SYSCALL(close(dec1[1]), checkerr, "Error closing fd");
	SYSCALL(close(dec2[0]), checkerr, "Error closing fd");
	SYSCALL(close(dec2[1]), checkerr, "Error closing fd");

	SYSCALL(wait(NULL), checkerr, "Error waiting for child");
	SYSCALL(wait(NULL), checkerr, "Error waiting for child");

	printf("Program finished\n");

	return 0;
}