#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <coda.h>

#define READSIZE 256

//Inizializzo due mutex per coda e buffer e due varcond per coda e buffer 
static pthread_mutex_t mutex_coda	= PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_buffer = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_coda  	= PTHREAD_COND_INITIALIZER;
static pthread_cond_t cond_buffer 	= PTHREAD_COND_INITIALIZER;

coda *Q1; 		//Dichiarata globale perche' condivisa tra i primi 2 thread
char *BUF=NULL;	//Buffer condiviso tra thread 2 e thread 3

void *stadio1(void*filepath) {
	
	FILE*file=NULL;
	char*line=NULL;

	//Apro il file in lettura
	if ((file=fopen((char*)filepath, "r"))==NULL) {
		perror("Error opening file");
		return NULL;
	}
	//Leggo il file e lo inserisco nel buffer line
	line=malloc(READSIZE*sizeof(char));
	while (line!=NULL && fgets(line, READSIZE, file) !=NULL) {
		if (strlen(line)==0) continue;
		line[strlen(line)-1]='\0';
		//Faccio push di line nella coda senza wait
		pthread_mutex_lock(&mutex_coda);
		push(Q1, line);	
		pthread_cond_signal(&cond_coda);
		pthread_mutex_unlock(&mutex_coda);
		line=malloc(READSIZE*sizeof(char));
	}
	if (line) free(line);

	//Faccio push in coda di EOS
	pthread_mutex_lock(&mutex_coda);
	push(Q1, EOS);
	pthread_cond_signal(&cond_coda);
	pthread_mutex_unlock(&mutex_coda);
	fclose(file);

	return NULL;
}

void *stadio2(void*notused) {

	while (1) {
		char*line=NULL;
		pthread_mutex_lock(&mutex_coda);
		while (Q1->size == 0) {
			pthread_cond_wait(&cond_coda, &mutex_coda);
		}
		line=pop(Q1);		
		pthread_mutex_unlock(&mutex_coda);
		if (line==EOS || line==NULL) break;

		//Divido la linea e ogni parola la inserisco nel buffer BUF, che passo al terzo thread
		char *tmpstr;
        char *token = strtok_r(line, " ", &tmpstr);
        while(token) {
            char *tout = strdup(token);
            //Metto il token appena letto nel buffer
			pthread_mutex_lock(&mutex_buffer);
			while (BUF!=NULL) {
				pthread_cond_wait(&cond_buffer, &mutex_buffer);
			}
			BUF=tout;
			pthread_cond_signal(&cond_buffer);
			pthread_mutex_unlock(&mutex_buffer);
            token = strtok_r(NULL, " ", &tmpstr);
        }
        free(line);
	}

	//Push di EOS e esco
	pthread_mutex_lock(&mutex_buffer);
	while (BUF!=NULL) {
			pthread_cond_wait(&cond_buffer, &mutex_buffer);
		}
	BUF=EOS;
	pthread_cond_signal(&cond_buffer);
	pthread_mutex_unlock(&mutex_buffer);

	return NULL;
}

void *stadio3(void*notused) {
	
	coda*Q2;
	Q2=initCoda(Q2);
	
	while (1) {
		char*token=NULL;

		//Prendo la stringa nel buffer e la 
		pthread_mutex_lock(&mutex_buffer);
		while (BUF==NULL) {
			pthread_cond_wait(&cond_buffer, &mutex_buffer);
		}
		token=BUF;
		BUF=NULL;
		pthread_cond_signal(&cond_buffer);
		pthread_mutex_unlock(&mutex_buffer);
		if (token==EOS) break;
		if (!updateRepeat(Q2, token)) {
			push(Q2, token);
		}
	}

	//Sono uscito, ho finito di inserire le parole nella coda. Stampo tutte le parole con contatore =1
	stampaCoda(Q2);
	deleteCoda(Q2);
	return NULL;
}

int main (int argc, char*argv[]) {

	if (argc!=2) {
		fprintf(stderr, "Usage: %s file\n", argv[0]);
		return -1;
	}

	pthread_t th1, th2, th3;
	
	Q1=initCoda(Q1); //Inizializzo la coda nel main per essere sicuro che venga inizializzata prima di iniziare il secondo thread
	
	//Creo i 3 thread
	if (pthread_create(&th1, NULL, stadio1, argv[1])!=0) {
		fprintf(stderr, "Error creating first thread\n");
		exit(EXIT_FAILURE);
	}
	if (pthread_create(&th2, NULL, stadio2, NULL)!=0) {
		fprintf(stderr, "Error creating second thread\n");
		exit(EXIT_FAILURE);
	}
	if (pthread_create(&th3, NULL, stadio3, NULL)!=0) {
		fprintf(stderr, "Error creathing third thread\n");
		exit(EXIT_FAILURE);
	}

	//Aspetto i 3 thread
	if (pthread_join(th1, NULL)!=0) {
		fprintf(stderr, "Error waiting for first\n");
		exit(EXIT_FAILURE);
	}
	if (pthread_join(th2, NULL)!=0) {
		fprintf(stderr, "Error waiting for second thread\n");
		exit(EXIT_FAILURE);
	}
	if (pthread_join(th3, NULL)!=0) {
		fprintf(stderr, "Error waiting for third thread\n");
		exit(EXIT_FAILURE);
	}

	deleteCoda(Q1);
	return 0;
}