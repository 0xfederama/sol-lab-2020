#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DIM 512

#define CHECKNULL(x, val, str) \
	if ((x)==(val)) { \
		perror(#str); \
		exit(EXIT_FAILURE); \
	}

typedef int (*F_t)(const void*, const void*, size_t);

int compare(F_t cmp, const void *s1, const void *s2, size_t n) {
    return cmp(s1,s2,n);
}

int main (int argc, char*argv[]) {
	if (argc!=3) {
		fprintf(stderr, "usage: %s <dim> crea/controlla\n", argv[0]);
		return -1;
	}
	int dim=strtol(argv[1], NULL, 10);
	if (dim>DIM) {
		fprintf(stderr, "reduce matrix dimension, it should be <=%d\n", DIM);
		return -1;
	}
	
	if (strncmp("crea", argv[2], strlen("crea"))==0) {
	
		float *mat=NULL;
		CHECKNULL(mat=(float*)malloc(dim*dim*sizeof(float)), NULL, "malloc");
		
		//Inizializzo la matrice
		for (int i=0; i<dim; ++i)
			for (int j=0; j<dim; ++j)
				mat[i*dim+j]=(i+j)/2.0;
		
		
		//Creo i file di output
		FILE *fdec=NULL;
		FILE *fbin=NULL;
		CHECKNULL(fdec=fopen("./mat_dump.txt", "w"), NULL, "creating .txt file");
		CHECKNULL(fbin=fopen("./mat_dump.dat", "w"), NULL, "creating .dat file");
		
		//Scrivo la matrice nei file di output
		for (int i=0; i<dim; ++i)
			for (int j=0; j<dim; ++j)
				fprintf(fdec, "%f\n", mat[i*dim+j]);
				
		for (int i=0; i<dim; ++i)
			for (int j=0; j<dim; ++j)
				fwrite(&mat[i*dim+j], sizeof(float), 1, fbin);
		
		fclose(fdec);
		fclose(fbin);
		free(mat);
		
	} else {
		
		//Apro i file con le matrici memorizzate
		FILE *fdec;
		CHECKNULL(fdec=fopen("./mat_dump.txt", "r"), NULL, "opening .txt file");
		FILE *fbin;
		CHECKNULL(fbin=fopen("./mat_dump.dat", "r"), NULL, "opening .dat file");
		
		//Creo matrici in cui copiare quelle dei file
		float *matdec=NULL;
		CHECKNULL(matdec=(float*)malloc(dim*dim*sizeof(float)), NULL, "malloc");
		float *matbin=NULL;
		CHECKNULL(matbin=(float*)malloc(dim*dim*sizeof(float)), NULL, "malloc");
		
		//Copio la matrice decimale
		char buf[128];
		for (int i=0; i<dim; ++i)
			for (int j=0; j<dim; ++j) {
				if (fgets(buf, 128, fdec)==NULL) {
					perror("fgets");
					return -1;
				}
				buf[strlen(buf)-1]='\0';
				matdec[i*dim+j]=strtof(buf, NULL);
			}
		fclose(fdec);
		
		//Copio la matrice binaria
		for(int i=0; i<dim; ++i)
			for (int j=0; j<dim; ++j) {
				if (fread(&matbin[i*dim+j], sizeof(float), 1, fbin)!=1) {
					perror("fread");
					return -1;
				}
			}
		fclose(fbin);
		
		//Confronto le due matrici matbin e matdec
		if (compare(memcmp, matdec, matbin, dim*dim*sizeof(float))==0) 
			printf("le matrici corrispondono\n");
		else 
			printf("le matrici non corrispondono\n");
		
		free(matdec);
		free(matbin);
		
	}
	return 0;
}
















