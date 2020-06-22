#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define BUFSIZE 256

int main (int argc, char *argv[]) {
	
	if (argc<3 || argc>4) {
		fprintf(stderr, "Usage: %s filein fileout [buffersize]\n", argv[0]);
		exit(-1);
	}
	size_t bufsize=BUFSIZE;
	if (argc==4) bufsize=strtol(argv[3], NULL, 10);
	char*fin=argv[1];
	char*fout=argv[2];
	
	//Apro il file in input
	FILE *filein;
	if ((filein=fopen(fin, "r")) == NULL) {
		perror("Error opening input file");
		exit(-1);
	}
	
	//Apro o creo il file in output
	FILE *fileout;
	if ((fileout=fopen(fout, "w+")) == NULL) {
		perror("Error opening output file");
		exit(-1);
	}

	char *buf=(char*)malloc(bufsize*sizeof(char));
	if (!buf) {
		perror("Error in malloc buf");
		return errno;
	}
	size_t len=0;
	
	//Leggo da un file e copio nell'altro	
	while ((len=fread(buf, 1, bufsize, filein))>0) {
		if (fwrite(buf, 1, len, fileout) != len) {
			perror("Error writing in output file");
			exit(-1);
		}
	}
	
	fclose(filein);
	fclose(fileout);
	free(buf);
	
	return 0;
	
}
