#ifndef MYLYB_H
#define MYLIB_H

#include <pthread.h>
#include <errno.h>

#define BUFSIZE 128
#define SYSCALL(r,c,e) if((r=c)==-1) { perror(e); exit(errno); }

/*------------------- CONFIG -------------------*/

typedef struct config {
	int K; 	//Numero massimo di casse
	int C; 	//Numero massimo di clienti
	int E; 	//Numero di clienti che devono uscire prima di farne entrare altri E - 0<E<C
	int T; 	//Tempo massimo per i clienti per fare acquisti - T>10 msec
	int P; 	//Massimo numero di prodotti che un cliente puo' comprare
	int S; 	//Tempo che ci mette un cassiere per passare un prodotto
	int S1; //Massimo numero di casse con 1 o 0 clienti
	int S2; //Massimo numero di clienti per cassa - se c'e una coda lunga almeno S2, viene aperta un'altra cassa
	int startupCheckout; //Quante casse sono aperte all'apertura del supermercato
	int notifyDir; 		 //Ogni quanto tempo i cassieri devono notificare il direttore sul numero di clienti in coda
} config;

int checkconfig(config *c);

config *getconf (char*configfile);

void printconf(config c);


/*----------------- CODA FIFO ------------------*/

typedef struct node {
	void* info;			//Puntatore di tipo void per poter puntare a qualsiasi tipo
	struct node *next;	//Puntatore al nodo successivo nella coda
} node;

typedef struct coda {
	unsigned int size;		//Dimensione della coda
	struct node *first; 	//Puntatore a primo elemento della lista
	struct node *last;		//Puntatore a ultimo elemento della lista
	pthread_mutex_t lock;	//Semaforo di mutex interno alla coda
	pthread_cond_t cond;	//Varcond interna alla coda
} coda;

//Inizializza e ritorna la coda
coda* initCoda ();

/**
 * Inserisce in coda alla coda
 * \retval -1 errore
 * \retval 0 se ok
 */
int push (coda*q, void *data);

/**
 * Rimuove la testa
 * \retval NULL se coda e' vuota
 * \retval info della testa se esiste
 */
void* pop (coda*q);

//Ritorna la lunghezza della coda oppure -1 se ci sono problemi
int getSize (coda *q);

//Free della coda
void deleteCoda (coda*q);


/*---------------- SUPERMARKET -----------------*/

typedef struct casse_sm {
	int id;
	int open;
	coda *codaClienti;	//Non serve nessun lock sulla coda perche' l'implementazione di questa coda prevede gia l'uso di lock
	pthread_mutex_t servitoLock;
	pthread_cond_t servitoCond;
	pthread_mutex_t openLock;
	pthread_cond_t openCond;
} casse_sm;

typedef struct clienteincoda {
	int id;
	int numprod;
	int *servito;
} clienteincoda;

#endif