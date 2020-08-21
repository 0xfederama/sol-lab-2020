#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "mylib.h"

/*------------------- CONFIG -------------------*/

int checkconfig(config *c) {
	if (c->K > 0 && c->E > 0 && c->E < c->C && c->P >= 0 && c->T > 10 && c->S1 > 1 && c->S2 > 0 && c->startupCheckout > 0 && c->notifyDir > 0) { //If configs are ok
		return 1;
	}
	return -1;
} 

config *getconf (char*configfile) {

	FILE *fd=NULL;
	config *conf;
	char *buffer;
	//Apri file per leggerlo e allora memoria per conf e buffer
	if ((fd=fopen(configfile, "r")) == NULL) {
		fclose(fd);
		fprintf(stderr, "Error - opening config file\n");
		return NULL;
	}
	if ((conf=malloc(sizeof(config))) == NULL) {
		fclose(fd);
		free(conf);
		fprintf(stderr, "Error - config malloc\n");
		return NULL;
	}
	if ((buffer=malloc(BUFSIZE*sizeof(char))) == NULL) {
		fclose(fd);
		free(conf);
		free(buffer);
		fprintf(stderr, "Error - buffer malloc for config\n");
		return NULL;
	}

	//Leggi ogni riga del file
	int indRow=0;
	char* backup; //Per memorizzare il vecchio valore del buffer
	while (fgets(buffer, BUFSIZE, fd) != NULL) {
		backup=buffer;
		while (*buffer != '=') {
			buffer++;
		}
		buffer++;
		switch (indRow) { //Uso l'indice 'stile' array, anche se la prima riga di config.txt e' 1
			case 0: 
				conf->K=atoi(buffer);
				break;
			case 1: 
				conf->C=atoi(buffer);
				break;
			case 2:
				conf->E=atoi(buffer);
				break;
			case 3:
				conf->T=atoi(buffer);
				break;
			case 4:
				conf->P=atoi(buffer);
				break;
			case 5:
				conf->S=atoi(buffer);
				break;
			case 6:
				conf->S1=atoi(buffer);
				break;
			case 7:
				conf->S2=atoi(buffer);
				break;
			case 8:
				conf->startupCheckout=atoi(buffer);
				break;
			case 9:
				conf->notifyDir=atoi(buffer);
				break;
			default:
				break;
		}
		++indRow;
		buffer=backup;
	}

	if (!(checkconfig(conf))) {
		fprintf(stderr, "Error - config should have K>0, P>=0, 0<E<C, S1>1, S1<S2, T>10, S>0, startupCheckout>0, notifyDir>0\n");
		fclose(fd);
		free(conf);
		free(buffer);
		return NULL;
	} 

	fclose(fd); 
	free(buffer);
	return conf;
}

void printconf(config c) {
	printf("K=%d C=%d E=%d T=%d P=%d S=%d S1=%d S2=%d STARTCHECKOUT=%d NOTIFYDIR=%d\n", c.K, c.C, c.E, c.T, c.P, c.S, c.S1, c.S2, c.startupCheckout, c.notifyDir);
}


/*----------------- CODA FIFO ------------------*/

//Inizializza e ritorna la coda
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
		perror("Error inizializing mutex in queue");
		return NULL;
    }
    if (pthread_cond_init(&q->cond, NULL) != 0) {
		perror("Error initializing varcond in queue");
		if (&q->lock) pthread_mutex_destroy(&q->lock);
		return NULL;
    }
	return q;
}

/**
 * Inserisce in coda alla coda
 * \retval -1 errore
 * \retval 0 se ok
 */
int push (coda*q, void *data) {
	node *ins=malloc(sizeof(node));
	if (!q || !data || !ins) return -1;
	ins->info=data;
	ins->next=NULL;

	pthread_mutex_lock(&q->lock);
	//q->last->next=ins;
	//q->last=ins;
	if (q->size==0) {
		q->first->next=ins;
		q->last=ins;
	} else {
		q->last->next=ins;
		q->last=ins;
	}
	(q->size)++;
	pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->lock);
	return 0;
}

/**
 * Rimuove la testa
 * \retval NULL se coda e' vuota
 * \retval info della testa se esiste
 */
void* pop (coda*q) {
	if (q==NULL) return NULL;
	pthread_mutex_lock(&q->lock);
	while (q->size == 0) {
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

//Ritorna la lunghezza della coda oppure -1 se ci sono problemi
int getSize (coda *q) {
	if (q==NULL) return -1;
	int size=-1;
	pthread_mutex_lock(&q->lock);
	size=q->size;
	pthread_mutex_unlock(&q->lock);
	return size;
}

//Free della coda
void deleteCoda (coda*q) {
	while (q->first != q->last) {
		node *killer=(node*)q->first;
		q->first=q->first->next;
		free((void*)killer);
	}
	if (q->first) free((void*)((void*)q->first));
	if (&q->lock) pthread_mutex_destroy(&q->lock);
    if (&q->cond) pthread_cond_destroy(&q->cond);
    free(q);
}
