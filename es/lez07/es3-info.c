#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <time.h>
#include <grp.h>


int info (const char *file) {
	
	struct stat info;
	if (stat(file, &info)==-1) {
		perror("Error storing file info");
		return errno;
	}
	
	char perm[11];
	perm[10]='\0';
	
	for (int i=0; i<10; ++i) {
		perm[i]='-';
	}
	
	if (S_ISREG(info.st_mode)) perm[0]='-';
	if (S_ISDIR(info.st_mode)) perm[0]='d';
	if (S_ISCHR(info.st_mode)) perm[0]='c';
	if (S_ISBLK(info.st_mode)) perm[0]='b';
	if (S_ISLNK(info.st_mode)) perm[0]='l';
	if (S_ISFIFO(info.st_mode)) perm[0]='p';
	if (S_ISSOCK(info.st_mode)) perm[0]='s';

	if (S_IRUSR & info.st_mode) perm[1]='r';
	if (S_IWUSR & info.st_mode) perm[2]='w';
	if (S_IXUSR & info.st_mode) perm[3]='x';

	if (S_IRGRP & info.st_mode) perm[4]='r';
	if (S_IWGRP & info.st_mode) perm[5]='w';
	if (S_IXGRP & info.st_mode) perm[6]='x';

	if (S_IROTH & info.st_mode) perm[7]='r';
	if (S_IWOTH & info.st_mode) perm[8]='w';
	if (S_IXOTH & info.st_mode) perm[9]='x';

	struct passwd *pw=getpwuid(info.st_uid);
	struct group *gr =getgrgid(info.st_gid);
	
	printf("%-16s (%-7ld):  %-12s %s,%s  %-8ld %s", file, info.st_ino, perm, pw->pw_name, gr->gr_name, info.st_size, ctime(&info.st_mtime));
	
	return 0;
}

int main (int argc, char * argv[]) {
	
	if (argc<2) {
		fprintf(stderr, "Usage: %s dir/file [dir/file]\n", argv[0]);
		return -1;
	}
	
	for (int i=1; i<argc; ++i) {
		if (info(argv[i])!=0) return -1;
	}
	
	return 0;
	
}
