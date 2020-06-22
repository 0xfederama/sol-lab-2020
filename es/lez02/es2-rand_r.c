#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define N 1000
#define K 10

int main (int argc, char *argv[]) {
	int occorrenze[K];
	unsigned int seed = time(NULL);
	
	for (int i=0; i<K; i++) occorrenze[i]=0;
	
	for (int i=0; i<N; i++)
		occorrenze[(rand_r(&seed) %K)]++;
		
	printf("Occorrenze di:\n");
	
	for (int i=0; i<K; i++) 
		printf("%d : %5.2f%% \n", i, (occorrenze[i]/(float)N)*100);
		
	printf("\n");
	return 0;
}
