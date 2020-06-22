#include <stdio.h>
#include <stdlib.h>

#define dimN 16
#define dimM  8

#define CHECK_PTR(x, str) \
	if ((x)==NULL) { \
		perror(#str); \
		exit(EXIT_FAILURE); \
	}

#define ELEM(M,i,j)	(M[(i*dimM)+j])

#define PRINTMAT(mat) \
	do { \
		printf("Stampo la matrice %s:\n", #mat); \
		for (size_t i=0; i<dimN; ++i) { \
		for (size_t j=0; j<dimM; ++j)  \
			printf ("%4ld ", ELEM(mat,i,j)); \
		printf("\n"); \
		} \
	} while (0)

int main() {
    long *M = malloc(dimN*dimM*sizeof(long));
    CHECK_PTR(M, "malloc"); 
    for(size_t i=0;i<dimN;++i)
	for(size_t j=0;j<dimM;++j)			
	    ELEM(M,i,j) = i+j;    
    
    PRINTMAT(M);
    free(M);
    return 0;
}
