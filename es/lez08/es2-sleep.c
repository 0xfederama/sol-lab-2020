#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main (int argc, char *argv[]) {
	
	if (argc!=2) {
		fprintf(stderr, "Usage: %s X[sec of sleep]\n", argv[0]);
		return -1;
	}
	
	int exec, pid;
	pid=fork();
	
	if (pid==0) {
		if ((pid=fork())==0) {
			execl("/bin/sleep", "bin/sleep", argv[1], NULL);
			perror("Error calling sleep");
			return errno;
		}
		if (waitpid(pid, NULL, 0)==-1) {
			perror("Error waiting pid");
			return errno;
		}
		printf("mypid= %d, myparentpid= %d\n", getpid(), getppid());
		return 0;
	}
	
	return 0;
}
