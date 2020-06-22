#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PASSWD "/etc/passwd"
#define MAXLENGTH 1024

int main (int argc, char *argv[]) {
	if (argc!=2) {
		fprintf(stderr, "troppi pochi argomenti, usage: %s filename", argv[0]);
		return -1;
	}
	FILE *fd=NULL; //File psswd da aprire e leggere
	FILE *fout=NULL; //File da creare
	char *buffer=NULL; //Stringa in cui scrivere il 
	
	if ((fd=fopen(PASSWD, "r"))==NULL) {
		perror("opening passwd file");
		goto error;
	}
	if ((fout=fopen(argv[1], "w"))==NULL) {
		perror("opening output file");
		goto error;
	}
	if ((buffer=malloc(MAXLENGTH*sizeof(char)))==NULL) {
		perror("malloc buffer");
		goto error;
	}
	
	while (fgets(buffer, MAXLENGTH, fd)!=NULL) {
		char*line;
		if ((line=strchr(buffer, '\n'))==NULL) {
			perror("consider increasing maxlegth");
			goto error;
		}
		*line='\0';
		char*login;
		if ((login=strchr(buffer, ':'))==NULL) {
			perror("wrong file format");
			goto error;
		}
		*login='\0';
		fprintf(fout, "%s\n", buffer);
	}
	
	fclose(fd);
	fclose(fout);
	free(buffer);
	
	return 0;
	
	error: //Chiudo i file, faccio free del buffer e esco con errore
		if (fd) fclose(fd);
		if (fout) fclose(fout);
		if (buffer) free(buffer);
		exit(EXIT_FAILURE);
}
