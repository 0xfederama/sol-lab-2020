#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFSIZE 256
#define SYSCALL(sc, ret, string)	\
	if ((ret=sc)==-1) {	\
		perror(string);	\
		return ret;	\
	}

int bc(char*input, char*res) {

	int inp[2];
	int out[2];
	int checkerr;
	SYSCALL(pipe(inp), checkerr, "Error opening first pipe");
	SYSCALL(pipe(out), checkerr, "Error opening second pipe");

	if (fork()==0) {
		//Chiudo i file descriptor non usati nel figlio
		SYSCALL(close(inp[1]), checkerr, "Error closing fd");
		SYSCALL(close(out[0]), checkerr, "Error closing fd");
		//Ridirigo stdin, stdout, stderr sui fd aperti
		SYSCALL(dup2(inp[0], 0), checkerr, "Error redirecting stdin");
		SYSCALL(dup2(out[1], 1), checkerr, "Error redirecting stdout");
		SYSCALL(dup2(out[1], 2), checkerr, "Error redirecting stderr");

		execl("/usr/bin/bc", "bc", "-l", NULL);
		perror("Error in bc");
		return -1;
	}

	//Chiudo i file descriptor non usati nel padre
	SYSCALL(close(inp[0]), checkerr, "Error closing fd");
	SYSCALL(close(out[1]), checkerr, "Error closing fd");	
	//Scrivo in inp[1] l'operazione e leggo il risultato in res
	int rw;
	SYSCALL(write(inp[1], input, strlen(input)), rw, "Error writing in fd");
	SYSCALL(read(out[0], res, BUFSIZE), rw, "Error reading fd");
	//Chiudo i fd e aspetto il figlio
	SYSCALL(close(inp[1]), checkerr, "Error closing fd");
	SYSCALL(wait(NULL), checkerr, "Error waiting for child");

	return rw;
}

int main () {

	char result[BUFSIZE];
	while (1) {
		char input[BUFSIZE];
		memset(input, '\0', BUFSIZE);
		if (fgets(input, BUFSIZE, stdin)==NULL) {
			perror("Error reading from stdin");
			return -1;
		}
		if (strncmp(input, "exit", 4)==0) break;

		int n;
		if ((n=bc(input, result))<0) {
			fprintf(stdout, "Error in bc function\n");
			break;
		}
		result[n]='\0';

		fprintf(stdout, "Operazione: %s", input);
		fprintf(stdout, "Risultato: %s", result);
	}

	return 0;
}