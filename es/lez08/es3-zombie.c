#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main (int argc, char * argv[]) {
	
	if (argc!=2) {
		fprintf(stderr, "Usage: %s N[number of zombie processes to create]\n", argv[0]);
		return -1;
	}
	
	int num=atoi(argv[1]);
	
	for (int i=0; i<num; ++i) {
		if (fork()==0) exit(0);
	}
	
	printf("Program will continue for 30 sec\n");
	sleep(30);
	
	return 0;
	
}
