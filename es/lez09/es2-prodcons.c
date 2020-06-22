#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUMBERS 20

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond  = PTHREAD_COND_INITIALIZER;
static int buffer;		// Buffer condiviso
static char bufempty=1;	// Se buffer e' vuoto=1

void *producer (void*notused) {

	for (int i=0; i<NUMBERS; ++i) {
		
		pthread_mutex_lock(&mutex);
		while (bufempty==0) {
			pthread_cond_wait(&cond, &mutex);
		}
		buffer=i;
		bufempty=0;
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);		
	}
	
	//Genero un valore <0 per generare "errore" nel consumatore
	pthread_mutex_lock(&mutex);
	while (bufempty==0) {
		pthread_cond_wait(&cond, &mutex);
	}
	buffer=-1;
	bufempty=0;
	printf("Producer done\n");
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
	
	return NULL;
}

void *consumer (void*notused) {
	
	int n=0;
	while (n>=0) {
		pthread_mutex_lock(&mutex);
		while (bufempty==1) {
			pthread_cond_wait(&cond, &mutex);
		}
		n=buffer;
		bufempty=1;
		printf("Consumer has: %d\n", n);
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
	}
	
	printf("Consumer done\n");
	
	return NULL;
}

int main () {
	
	pthread_t thconsumer, thproducer;
	
	if (pthread_create(&thconsumer, NULL, consumer, NULL)!=0) {
		fprintf(stderr, "Error creating consumer thread\n");
		return -1;
	}
	if (pthread_create(&thproducer, NULL, producer, NULL)!=0) {
		fprintf(stderr, "Error creating producer thread\n");
		return -1;
	}
	if (pthread_join(thproducer, NULL) != 0) {
		fprintf(stderr, "Error joining producer thread\n");
		return -1;
    }
    if (pthread_join(thconsumer, NULL) != 0) {
		fprintf(stderr, "Error joining consumer thread\n");
		return -1;
    }
	
	return 0;
}
