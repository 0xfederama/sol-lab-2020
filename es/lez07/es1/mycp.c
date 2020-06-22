#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define BUFSIZE 256

int main (int argc, char *argv[]) {
	
	if (argc<3 || argc>4) {
		fprintf(stderr, "Usage: %s filein fileout [buffersize]\n", argv[0]);
		exit(-1);
	}
	size_t bufsize=BUFSIZE;
	if (argc==4) bufsize=strtol(argv[3], NULL, 10);
	char*filein=argv[1];
	char*fileout=argv[2];
	
	//Apro il file in input
	int fin;
	if ((fin=open(filein, O_RDONLY))==-1) {
		perror("Error opening input file");
		exit(-1);
	}
	
	//Apro o creo il file in output
	int fout;
	if ((fout=open(fileout, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP))==-1) {
		perror("Error opening output file");
		exit(-1);
	}
	
	char *buf=(char*)malloc(bufsize*sizeof(char));
	if (!buf) {
		perror("Error in malloc buf");
		return errno;
	}
	size_t len=0;
	
	//Leggo da un file e copio subito nell'altro
	do {
		if ((len=read(fin, buf, bufsize))==-1) {
			perror("Error reading input file");
			exit(-1);
		}
		int lenwrite=0;
		if ((lenwrite=write(fout, buf, len))==-1) {
			perror("Error writing in output file");
			exit(-1);
		} else if (lenwrite<len) {
			fprintf(stderr, "Error writing in output file, syscall 'write' didn't wrote all bytes");
			exit(-1);
		}
	} while (len>0); //Se len=0 allora sono arrivato in fondo al file
	
	//Chiudo i file e flusho il file output
	if (fsync(fout)==-1) {
		perror("Error flushing output file");
		exit(-1);
	}
	if (close(fin)==-1) {
		perror("Error closing input file");
		exit(-1);
	}
	if (close(fout)==-1) {
		perror("Error closing output file");
	}
	free(buf);
	
	return 0;
	
}
