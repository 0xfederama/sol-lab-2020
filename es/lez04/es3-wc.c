#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>

#define MAX_LEN 2048

int main (int argc, char*argv[]) {
	if (argc==1) {
		fprintf(stderr, "usage: %s [-l -w] file1 [file2 file3 ...]\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	int line=0, word=0;
	char opt;
	int m;
	while ((opt=getopt(argc, argv, "lwm:"))!=-1) {
		switch (opt) {
			case 'l' : {
				line=1;
			} break;
			case 'w' : {
				word=1;
			} break;
			case 'm': {
				m = strtol(optarg, NULL, 10);
			} break;
			default: ;
		}
	}
	
	if (line+word==0) { //Se non sono specificate e' come se fossero abilitate entrambe
		line=1;
		word=1;
	}
	
	
	while (argv[optind]!=NULL) {
		//Apro il file e lo copio nel buffer
		FILE *f;
		char *readfile=argv[optind];
		int numl=0, numw=0;
		char *buffer=(char*)malloc(MAX_LEN*sizeof(char));
		
		if ((f=fopen(readfile, "r"))==NULL) {
			perror("fopen");
			exit(EXIT_FAILURE);
		}
		
		while (fgets(&buffer[strlen(buffer)], MAX_LEN, f)!=NULL) { //Copio dal file nel buffer
			;
		}
		
		int len=strlen(buffer);
		for (int i=0; i<len; ++i) {
			if (buffer[i]=='\n') { numl++; numw++; }
			if (buffer[i]==' ' && !isspace(buffer[i+1])) numw++;
		}
		
		free(buffer);
		fclose(f);
		if (line==1 && word==0) printf("%d %s\n", numl, argv[optind]);
		if (line==0 && word==1) printf("%d %s\n", numw, argv[optind]);
		if (line==1 && word==1) printf("%d %d %s\n", numl, numw, argv[optind]);
		optind++;
	}
	
	return 0;
}
