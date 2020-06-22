#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

void figli (int n) {
	int ppid=getpid();
	if (n>0) {
		for (int i=0; i<n; ++i) fprintf(stdout, "-");
		fprintf(stdout, " %d: creo un processo figlio\n", ppid);
		fflush(stdout);
		int pid = fork();
		if (pid==-1) {
			perror("Error doing fork");
			exit(EXIT_FAILURE);
		}
		if (pid==0) {
			figli(n-1);
		} else {
			int status;
			if (waitpid(pid, &status, 0)==-1) {
				perror("Error waiting for child");
				return;
			}
			if (!WIFEXITED(status)) {
				fprintf(stdout, " %d: figlio %d terminato con fallimento\n", ppid, pid);
				fflush(stdout);
			}
			for (int i=0; i<(n-1); ++i) fprintf(stdout, "-");
			if (n>1) fprintf(stdout, " ");
			fprintf(stdout, "%d: terminato con successo\n", ppid);
			fflush(stdout);
		}
	} else {
		fprintf(stdout, "%d: sono l'ultimo discendente\n", ppid);
		fflush(stdout);
	}
}

int main (int argc, char *argv[]) {
	
	if (argc!=2) {
		fprintf(stderr, "Usage: %s N[>1]\n", argv[0]);
		return -1;
	}
	
	int n=atoi(argv[1]);
	int pid=getpid();
	figli(n);
	
	if (pid==getpid()) {
		for (int i=0; i<n; ++i) fprintf(stdout, "-");
		fprintf(stdout, " %d: terminato con successo\n", pid);
		fflush(stdout);
	}
	
	return 0;
}
