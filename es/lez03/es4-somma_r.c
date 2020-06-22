#include <stdio.h>
#include <stdlib.h>

#if !defined (INIT_VALUE)
#error("Compile with the flag -DINIT_VALUE=number");
#endif

int somma(int x) {
	static int s = INIT_VALUE;
	s+=x;
	return s;
}

int somma_r(int x, int *s) {
	*s+=x;
	return *s;
}

int main (int argc, char *argv[]) {
	if (argc!=2) {
		fprintf(stderr, "usage: %s N\n", argv[0]);
		return -1;
	}
	int n = strtol(argv[1],NULL, 10);
	if (n<=0 || n>10) {
		fprintf(stderr, "n deve essere compreso tra 1 e 10 inclusi\n");
	}
	int sommaparz=INIT_VALUE;
	for (int i=0; i<n; ++i) {
		int val;
		fprintf(stdout, "Inserisci un numero (%d rimasti): ", n-i);
		if (fscanf(stdin,"%d", &val)!=1) {
			perror("fscanf");
			exit(EXIT_FAILURE);
		}

		fprintf(stdout, "somma: %d\n", somma(val));
		fprintf(stdout, "somma_r: %d\n", somma_r(val, &sommaparz));
	}
	
	return 0;
}
