#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <assert.h>
#include <coda.h>

typedef struct threadArgs {
	int thid;	//Thread id
	int start;	//Variabili per producer, per inserire in coda i processi con valore da start a stop, 
	int stop;	//	ovvero i messaggi che non sono ancora stati prodotti
	coda *q;
} threadArgs;

void stampaCoda(coda*q) {	//Stampa per DEBUG
	node*print=q->first->next;
	while (print!=NULL) {
		printf("%d\n", *((int*)print->info)); //Cast esplicito al valore memorizzato nella coda
		print=print->next;
	}
	free(print);
}

void *producer(void *arg) {
	coda*q=((threadArgs*)arg)->q;
	int id=((threadArgs*)arg)->thid;
	int start=((threadArgs*)arg)->start;
	int stop=((threadArgs*)arg)->stop;
	for (int i=start; i<stop; ++i) {
		int *mess=malloc(sizeof(int));
		if (mess==NULL) {
			perror("Error in malloc");
            pthread_exit(NULL);
		}
		*mess=i;
		if (push(q, mess)==-1) {
			fprintf(stderr, "Error pushing in queue\n");
			pthread_exit(NULL);
		}
		printf("Producer %d sent %d\n", id, *mess);
	}
	printf("Producer %d. Done\n", id);
	return NULL;
}

void *consumer(void *arg) {
	coda*q=((threadArgs*)arg)->q;
	int id=((threadArgs*)arg)->thid;
	int consumed=0;
	while (1) {
		int *mess;
		mess=pop(q);
		assert(mess);
		if (*mess==-1) {
			free(mess);
			break;
		}
		consumed++;
		printf("Consumer %d received %d\n", id, *mess);
	}
	printf("Consumer %d received %d messages. Done\n", id, consumed);
	return NULL;
}

int main (int argc, char*argv[]) {
	
	//Parsing
	extern char*optarg;
	int p=0, c=0, k=0;
	int opt;
	while ((opt=getopt(argc, argv, "p:c:k:"))!=-1) {
		switch(opt) {
			case 'p':
				p=atoi(optarg);
				break;
			case 'c':
				c=atoi(optarg);
				break;
			case 'k':
				k=atoi(optarg);
				break;
			default:
				fprintf(stderr, "Usage: %s -p <#producer> -c <#consumer> -k <#messages>\n", argv[0]);
				exit(EXIT_FAILURE);
				break;
		}
	}
	if (p==0 || c==0 || k==0) {
		fprintf(stderr, "Usage: %s -p <#producer> -c <#consumer> -k <#messages>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	pthread_t *th;
	threadArgs *tharg;
	th = malloc((p+c)*sizeof(pthread_t));
    tharg = malloc((p+c)*sizeof(threadArgs));
    if (!th || !tharg) {
        fprintf(stderr, "Error in malloc\n");
        exit(EXIT_FAILURE);
    }
    coda *q = initCoda();	//Coda condivisa tra producer e consumer, la passo poi a entrambi i thread
    if (!q) {
        fprintf(stderr, "Error initializing queue\n");
        exit(EXIT_FAILURE);
    }

	int chunk = k/p;
	int r=k%p;
    int start = 0;
    for(int i=0;i<p; ++i) {		//Inizializzo la struct per il thread producer
        tharg[i].thid = i;
        tharg[i].q    = q;
        tharg[i].start= start;
        tharg[i].stop = start+chunk + ((i<r)?1:0);
        start = tharg[i].stop;	
    }
    for(int i=p;i<(p+c); ++i) {	//Inizializzo la struct per il thread consumer
        tharg[i].thid = i-p;	//i-p perche' cosi vanno da 0 a c senza dover creare nuovo array (mantengo i)
        tharg[i].q    = q;		
        tharg[i].start= 0;
        tharg[i].stop = 0;
    }

    for(int i=0;i<p; ++i) {
		if (pthread_create(&th[i], NULL, producer, &tharg[i]) != 0) {
            fprintf(stderr, "pthread_create failed (Producer)\n");
            exit(EXIT_FAILURE);
        }
	}
	for(int i=0;i<c; ++i) {
		if (pthread_create(&th[p+i], NULL, consumer, &tharg[p+i]) != 0) {
            fprintf(stderr, "pthread_create failed (Consumer)\n");
            exit(EXIT_FAILURE);
        }
	}

	//Aspetto i producer
	for(int i=0;i<p; ++i) {
		pthread_join(th[i], NULL);
	}
	    
    //Inserisco c valori di terminazione in coda in modo da far fermare i consumer
    for(int i=0;i<c; ++i) {
        int *eos = malloc(sizeof(int));
        *eos = -1;
        push(q, eos);
    }
    //Aspetto i consumer
    for(int i=0;i<c; ++i) {
		pthread_join(th[p+i], NULL);
	}
	        
	deleteCoda(q);
    free(th);
    free(tharg);

	return 0;
}