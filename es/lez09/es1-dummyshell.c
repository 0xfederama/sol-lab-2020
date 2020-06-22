#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define BUFSIZE 512

int main () {
	
	char*argv[BUFSIZE];
	char input[BUFSIZE];
	int pid;
        int status;
	
	while (1) {
		
		printf("> ");
		fflush(stdout);
		//Riempio input di 0, cosi sovrascrivo ogni volta
		memset(input, 0, BUFSIZE);
		//Leggo da stdin e inserisco in input
		if (read(STDIN_FILENO, input, BUFSIZE)==-1) {
			perror("Error reading from standard input");
			continue;
		}
		input[strlen(input)-1]='\0';
		
		//Se leggo exit esco
		if (strncmp(input, "exit", 4) == 0)  {
			int i=0;
			while (argv[i]!=NULL) {
				free(argv[i]);
				i++;
			}
			break;
		}
		
		//Leggo input e ogni " " divido ogni parola in argv
		if (input[0]!='\0') {
			//Creo dei buffer per il token
			char*tmp, *token=strtok_r(input, " ", &tmp);
			int i=0;
			size_t len=strlen(token)+1;
			if ((argv[i]=(char*)malloc(len*sizeof(char)))==NULL) {
				perror("Error in malloc");
				exit(EXIT_FAILURE);
			}
			strncpy(argv[i], token, len);
			while ((token=strtok_r(NULL, " ", &tmp))!=NULL) {
				len=strlen(token)+1;
				i++;
				if ((argv[i]=(char*)malloc(len*sizeof(char)))==NULL) {
					perror("Error in malloc");
					exit(EXIT_FAILURE);
				}
				strncpy(argv[i], token, len);
			}
			argv[i+1]=NULL; //Adesso in argv ci sono le parole che ho immesso da input
		} else continue;
		
		
		//Forko un processo a cui faccio eseguire il comando
		if ((pid=fork())==-1) {
			perror("Error forking a process");
			continue;
		}
		if (pid==0) {
			execvp(argv[0], &argv[0]);
			perror("Error executing the command");
			exit(errno);
		}
		
		//Aspetto che il processo abbia finito
		if (waitpid(pid, &status, 0)==-1) {
        	perror("Error waiting for child");
        	continue;
        }
	}
	
	printf("Shell done\n");
	return 0;
	
}
