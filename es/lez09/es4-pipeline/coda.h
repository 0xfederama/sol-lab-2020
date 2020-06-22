#if !defined(CODA_H)
#define CODA_H

#define EOS (void*)0x1

typedef struct node {
	char* info;
	int repeat;		 	//Evito le ripetizioni, le gestisco con un contatore per quante volte ho incontrato info
	struct node *next;	//Puntatore al nodo successivo nella coda
} node;

typedef struct coda {
	int size;			//Dimensione della coda
	struct node *first; //Puntatore a primo elemento della lista
	struct node *last;	//Puntatore a ultimo elemento della lista
} coda;

//Inizializzo la coda
coda* initCoda (coda*q);

//Inserisco in coda
void push (coda*q, char *str);

/**
 * Rimuovo la testa
 * \retval NULL se coda e' vuota
 * \retval info della testa se esiste
 */
char* pop (coda*q);

/**
 * Aggiorna il contatore di un nodo se e' gia in lista. Prima lo cerca. Velocizza la ricerca perche' appena lo trova aggiorna.
 * \retval 0 se str non si trova in coda
 * \retval repeat se viene trovato e aggiornato il contatore repeat
 */
int updateRepeat(coda*q, char*str);

void deleteCoda (coda*q);

void stampaCoda (coda*q);

#endif