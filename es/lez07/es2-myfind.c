#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#define BUFSIZE 256

int isdot(const char dir[]) {
  int l = strlen(dir);
  if ( (l>0 && dir[l-1] == '.') ) return 1;
  return 0;
}

int find (const char *nomedir, const char *nomefile) {
	//Entro nella directory
	if (chdir(nomedir)==-1) {
		perror("Error entering directory");
		return 1;
	}
	
	DIR*dir;
	if ((dir=opendir("."))==NULL) { //Apro la directory corrente in cui sono entrato con chdir
		perror("Error opening directory");
		return -1;
	} else {
		struct dirent* file;
		while ((errno=0, file=readdir(dir)) != NULL) { //Leggi ricorsivamente tutti i file in dir
			struct stat statbuf;
			if (stat(file->d_name, &statbuf)==-1) {				
				perror("Error storing file info");
				return -1;
			}
			if (S_ISDIR(statbuf.st_mode)) {
				if (!isdot(file->d_name))
					if (find(file->d_name, nomefile)!=1) chdir("..");
			} else {
				if (strcmp(nomefile, file->d_name)==0) { //Ho trovato il file
					char buf[BUFSIZE];
					if (getcwd(buf, BUFSIZE) == NULL) {
						perror("Getting current working directory");
						return -1;
					}
					printf("%s/%s  %s", buf, file->d_name, ctime(&statbuf.st_mtime));
				}
			}
		}
	}
	if (errno!=0) perror("Reading directory");
	closedir(dir);
	
	return 0;
}

int main (int argc, char*argv[]) {
	
	if (argc!=3) {
		fprintf(stderr, "Usage: %s dir file\n", argv[0]);
		exit(-1);
	}
	
	struct stat statbuf;
	if (stat(argv[1], &statbuf)==-1) {
		perror("Error storing file info");
		return -1;
	}
	
	if(!S_ISDIR(statbuf.st_mode)) {
		fprintf(stderr, "%s is not a directory\n", argv[1]);
		return EXIT_FAILURE;
    } 
	
	int retval = find(argv[1], argv[2]);

	return retval;
}
