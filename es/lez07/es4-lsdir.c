#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFSIZE 1024

int isdot (const char*dir) {
	int len=strlen(dir);
	if (len>0 && dir[len-1]=='.') return 1;	
	return 0;
}

void ls (const char *nomedir) {
	
	struct stat statbuf;
	stat(nomedir, &statbuf);
	if (S_ISDIR(statbuf.st_mode)) {	
		DIR *dir;
		fprintf(stdout, "------------------\n");
		fprintf(stdout, "Directory: %s\n", nomedir);
		
		if ((dir=opendir(nomedir))==NULL) {
			perror("Error opening directory");
			return;
		} else {	
			struct dirent *file;
			while ((errno=0, file=readdir(dir))!=NULL) { //Leggi tutti i file all'interno
				
				//Salvo il nome del percorso fino al file per fare ls nel caso in cui sia una dir
				char filename[BUFSIZE];
				int lencwd=strlen(nomedir);
				int lenfile=strlen(file->d_name);
				if (lencwd+lenfile+2>BUFSIZE) {
					fprintf(stderr, "Error storing file name, increase buffer size\n");
					exit(EXIT_FAILURE);
				}
				strcpy(filename, nomedir);
				strcat(filename, "/");
				strcat(filename, file->d_name);
				
				//printf("DEBUG:%s\n\n", filename);
				
				struct stat statbuf;
				if (stat(filename, &statbuf)==-1) {
					perror("Error storing file info");
					return;
				}
				if (S_ISDIR(statbuf.st_mode)) {
					if (!isdot(filename)) ls(filename);
				} else {
					char perm[10] = {'-','-','-','-','-','-','-','-','-','\0'};
					if (S_IRUSR & statbuf.st_mode) perm[0]='r';
					if (S_IWUSR & statbuf.st_mode) perm[1]='w';
					if (S_IXUSR & statbuf.st_mode) perm[2]='x';

					if (S_IRGRP & statbuf.st_mode) perm[3]='r';
					if (S_IWGRP & statbuf.st_mode) perm[4]='w';
					if (S_IXGRP & statbuf.st_mode) perm[5]='x';

					if (S_IROTH & statbuf.st_mode) perm[6]='r';
					if (S_IWOTH & statbuf.st_mode) perm[7]='w';
					if (S_IXOTH & statbuf.st_mode) perm[8]='x';
					
					fprintf(stdout, "%25s: %10ld  %s\n", file->d_name, statbuf.st_size, perm);	
				}
				
			}
			
			if (errno != 0) perror("Error reading directory");
			closedir(dir);
				
		}
	}		
}

int main (int argc, char * argv[]) {
	
	if (argc!=2) {
		fprintf(stderr, "Usage: %s dir\n", argv[0]);
		return -1;
	}
	
	struct stat buf;
	stat(argv[1], &buf);
	if (!S_ISDIR(buf.st_mode)) {
		fprintf(stderr, "%s is not a directory\n", argv[1]);
		return -1;
	}
	
	ls(argv[1]);
	
	fprintf(stdout, "------------------\n");
	
	return 0;
}
