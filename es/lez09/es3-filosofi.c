#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

typedef struct threadArgs {
    int          thid; //Thread id
    int          N;
    pthread_mutex_t *forks; //Due 
} threadArgs;

void pensamangia(unsigned int *seed) {
	long r=rand_r(seed)%100000;
	struct timespec t={0, r};
	nanosleep(&t, NULL);
}

void *filosofo(void*arg) {

	int myid=((threadArgs*)arg)->thid;
    int N=((threadArgs*)arg)->N;
	int left=myid%N;
	int right=myid-1;
	pthread_mutex_t destra = ((threadArgs*)arg)->forks[right];
    pthread_mutex_t sinistra = ((threadArgs*)arg)->forks[left];

	int numthink=0;
	int numeat=0;

	//Pensa, prende forchette e poi mangia
	unsigned int seed=myid;

	for (int i=0; i<100; i++) {

		//Pensa
		pensamangia(&seed);
		numthink++;

		//Prendi le forchette (semafori) e mangia (in modo ordinato, prima forchette di indice minore)
		//Si puo' ordinare altrimenti dividendo tra filosofi di id pari e dispari, cosi' da decidere un ordine a
		//	filosofi "alterni"
		if (left<right) { //TODO: chiedere al prof perche left<right funziona (perche primo va contro gli altri?)
			pthread_mutex_lock(&destra);
			pthread_mutex_lock(&sinistra);
			numeat++;
			pensamangia(&seed);
			pthread_mutex_unlock(&sinistra);
			pthread_mutex_unlock(&destra);
		} else {
			pthread_mutex_lock(&sinistra);
			pthread_mutex_lock(&destra);
			numeat++;
			pensamangia(&seed);
			pthread_mutex_unlock(&destra);
			pthread_mutex_unlock(&sinistra);
		}

	}
	
	fprintf(stdout, "Filosofo %3d: ho mangiato %d volte e pensato %d volte\n", myid, numeat, numthink);
    fflush(stdout);
    pthread_exit(NULL);

}

int main (int argc, char*argv[]) {

	int n=5;
	if (argc>1) {
		n=atoi(argv[1]);
		if (n>100) n=100;
		else if (n<5) n=5;
	}

	pthread_t *filosofi;
	pthread_mutex_t *forks;
	threadArgs *tharg;
	filosofi = malloc(n*sizeof(pthread_t));
    tharg = malloc(n*sizeof(threadArgs));    
    forks  = malloc(n*sizeof(pthread_mutex_t));
    if (!filosofi || !tharg || !forks) {
		fprintf(stderr, "Error in malloc\n");
		exit(EXIT_FAILURE);
    }

	//Creo n filosofi e inizializzo i semafori
	for (int i=0; i<n; ++i) {
		tharg[i].thid=(i+1);
		tharg[i].N=n;
		tharg[i].forks=forks;
		if (pthread_mutex_init(&forks[i], NULL)!=0) {
			fprintf(stderr, "Error initializing mutex\n");
			exit(EXIT_FAILURE);
		}
	}

	//Creo n thread
	for (int i=0; i<n; ++i) {
		if (pthread_create(&filosofi[i], NULL, filosofo, &tharg[i])!=0) {
			fprintf(stderr, "Error creating thread %d\n", i);
			exit(EXIT_FAILURE);
		}
	}

	//Aspetto n thread
	for (int i=0; i<n; ++i) {
		if (pthread_join(filosofi[i], NULL)==-1) {
			fprintf(stderr, "Error waiting thread %d\n", i); //Non esco perche' comunque aspetto gli altri thread
		}
	}

	free(filosofi);
	free(tharg);
	free(forks);
	return 0;
}