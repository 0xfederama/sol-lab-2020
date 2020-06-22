#if !defined(CODA_H)
#define CODA_H

#include <pthread.h>

#define EOS (void*)0x1

typedef struct node {
	void* info;
	struct node *next;	//Puntatore al nodo successivo nella coda
} node;

typedef struct coda {
	unsigned int size;		//Dimensione della coda
	struct node *first; 	//Puntatore a primo elemento della lista
	struct node *last;		//Puntatore a ultimo elemento della lista
	pthread_mutex_t lock;	//Semaforo di mutex interno alla coda
	pthread_cond_t cond;	//Varcond interna alla cosa
} coda;

//Inizializzo la coda
coda* initCoda ();

/**
 * Inserisco in coda
 * \retval -1 errore
 * \retval 0 se ok
 */
int push (coda*q, void *data);

/**
 * Rimuovo la testa
 * \retval NULL se coda e' vuota
 * \retval info della testa se esiste
 */
void* pop (coda*q);

void deleteCoda (coda*q);

void stampaCoda (coda*q);

#endif