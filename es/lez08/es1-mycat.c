#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFSIZE 256

int main (int argc, char *argv[]) {
	
	if (argc==1) {
		fprintf(stderr, "Usage: %s file1 [fileN]\n", argv[0]);
		return -1;
	}
	
	for (int i=1; i<argc; ++i) {
		
		char buffer[BUFSIZE];
		
		//Apro il file
		int file;
		if ((file=open(argv[i], O_RDONLY))==-1) {
			perror("Error opening file");
			continue;
		}
		
		//Leggo il file e lo scrivo subito in output
		int len=0;
		do {
			if ((len=read(file, buffer, BUFSIZE))==-1) {
				perror("Error reading file");
				break;
			}
			int lenwrite=0;
			if ((lenwrite=write(1, buffer, len))==-1) {
				perror("Error writing in stdout");
				break;
			} else if (lenwrite<len) {
				fprintf(stderr, "Error writing in stdout, syscall 'write' didn't wrote all bytes\n");
				break;
			}			
		} while (len>0);
		
		//Chiudo il file
		if (close(file)==-1) {
			perror("Error closing file");
			continue;
		}
	}
	
	return 0;
}
