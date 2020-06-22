#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <coda.h>

//Implementazione coda FIFO con struct a primo e ultimo elemento per rendere piu veloce il processo

coda* initCoda () {
	coda *q=malloc(sizeof(coda));
	if (!q) return NULL;
	q->first=malloc(sizeof(node));
	if (!q->first) return NULL;
	q->first->info = NULL; 
    q->first->next = NULL;
    q->last = q->first; 
	q->size=0;
	if (pthread_mutex_init(&q->lock, NULL) != 0) {
		perror("Error inizializing mutex");
		return NULL;
    }
    if (pthread_cond_init(&q->cond, NULL) != 0) {
		perror("Error initializing varcond");
		if (&q->lock) pthread_mutex_destroy(&q->lock);
		return NULL;
    } 
	return q;
}

int push (coda*q, void *data) {
	node *ins=malloc(sizeof(node));
	if (!q || !data || !ins) return -1;
	ins->info=data;
	ins->next=NULL;

	pthread_mutex_lock(&q->lock);
	q->last->next=ins;
	q->last=ins;
	(q->size)++;
	pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->lock);
	return 0;
}

void* pop (coda*q) {
	if (q==NULL) return NULL;
	pthread_mutex_lock(&q->lock);
	while (q->first == q->last) {
		pthread_cond_wait(&q->cond, &q->lock);
	}
	assert(q->first->next);
	node*n=(node*)q->first;
	void*data=(q->first->next)->info;
	q->first=q->first->next;
	(q->size)--;
	assert(q->size>=0);
	pthread_mutex_unlock(&q->lock);
	free((void*)n);
	return data;
}

void deleteCoda (coda*q) {
	while (q->first != q->last) {
		node *killer=(node*)q->first;
		q->first=q->first->next;
		free((void*)killer);
	}
	if (q->first) free((void*)((void*)q->first));
	if (&q->lock)  pthread_mutex_destroy(&q->lock);
    if (&q->cond)  pthread_cond_destroy(&q->cond);
    free(q);
}

/*void stampaCoda (coda*q) {
	node* printer=q->first;
	printf("Le parole che compaiono una sola volta sono:\n");
	while (printer!=NULL) {
		if (printer->info!=EOS) {
			printf("%s\n", printer->info);
		} else break;
		printer=printer->next;
	}
	free(printer);
}*/