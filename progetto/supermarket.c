#define _POSIX_C_SOURCE  200112L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "mylib.h"

#define LOGFILENAME "./supermarket.log"

static volatile sig_atomic_t s_sighup=0;
static volatile sig_atomic_t s_sigquit=0;

static void handler (int sig) {

	if (sig==SIGQUIT) s_sigquit=1; //Chiusura immediata
	if (sig==SIGHUP) s_sighup=1; //Nessun cliente entra, finisco quelli dentro 

	write(1, (sig==SIGHUP) ? "Received signal SIGHUP\n" : "Received signal SIGQUIT\n", 24); 
	fflush(stdout);
	write(1, "Supermarket is about to close\n", 31);
}

static config *conf; 		//Struct di configurazioni iniziali
static casse_sm *casseCode;	//Array di casse con code di clienti	
static coda *clienti_sm;	//Coda di clienti

static int inizializzato=0; //Varibile per inizializzare l'array delle casse prima che qualche clienti provi ad accedere
static pthread_mutex_t initLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t initCond = PTHREAD_COND_INITIALIZER;

static int totCustomers=0;	//Numero totale di clienti entrati nel supermercato
static pthread_mutex_t totCustLock = PTHREAD_MUTEX_INITIALIZER;

static int inCustomers=0;	//Numero di clienti attualmente dentro il supermercato
static pthread_mutex_t inCustLock = PTHREAD_MUTEX_INITIALIZER;

static int outCustomers=0;	//Numero di clienti usciti
static pthread_mutex_t outCustLock = PTHREAD_MUTEX_INITIALIZER;

static int exitCustomers=0; //Numero di clienti che chiede di uscire
static pthread_mutex_t exitCustLock = PTHREAD_MUTEX_INITIALIZER;

static int exitOK=0;		//Variabile per far uscire i clienti senza acquisti che vogliono uscire
static pthread_mutex_t exitOKLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t exitOKCond = PTHREAD_COND_INITIALIZER;

static int totProd=0;		//Numero totale di prodotti comprati
static pthread_mutex_t totProdLock = PTHREAD_MUTEX_INITIALIZER;

static int notified=0;		//Variabile che specifica che il thread notifier ha mandato la notifica al direttore
static int *sizeCode;		//Array di lunghezze delle code delle casse
static pthread_mutex_t notifLock = PTHREAD_MUTEX_INITIALIZER;

static FILE *logfile;		//Logfile con lock per far scrivere direttamente ai vari thread clienti e cassieri
static pthread_mutex_t logLock = PTHREAD_MUTEX_INITIALIZER;


//---------------------------------------------------------------//
//---------------------- Notifier Thread ------------------------//
//---------------------------------------------------------------//
//Thread che manda notifiche al direttore
void *notify (void *arg) {

	//Prima di iniziare, inizializza l'array di notifica delle casse e dormi per conf->T per cercare di evitare di leggere le code delle casse prima che i clienti si mettano in coda
	if ((sizeCode=malloc(conf->K*sizeof(int))) == NULL) {
		perror("Error creating notifications array");
		pthread_exit(NULL);
	}
	for (int i=0; i<conf->K; ++i) {
		sizeCode[i]=0;
	}
	struct timespec sleeptime = { 0, conf->T*1000000 };
	nanosleep(&sleeptime, NULL);

	while (s_sigquit==0) {
	
		if (s_sighup==1) {
			pthread_mutex_lock(&inCustLock);
			if (inCustomers==0) {
				pthread_mutex_unlock(&inCustLock);
				pthread_exit(NULL);
			}
			pthread_mutex_unlock(&inCustLock);
		}	

		//Dormi per conf->notifyDir secondi
		struct timespec sleeptime = { 0, conf->notifyDir*1000000 };
		nanosleep(&sleeptime, NULL);

		//Segnala al direttore la lunghezza delle code delle casse
		pthread_mutex_lock(&notifLock);
		for (int i=0; i<conf->K; ++i) {
			sizeCode[i] = getSize(casseCode[i].codaClienti);
		}
		notified=1;
		pthread_mutex_unlock(&notifLock);

	}

	pthread_exit(NULL);

}


//---------------------------------------------------------------//
//---------------------- Checkout Thread ------------------------//
//---------------------------------------------------------------//
//Thread che gestisce la singola cassa
void *checkouts (void *arg) {
	
	int id = *(int*)arg;

	//Calcolo tempo random fisso del cassiere
	int seed = id;
	unsigned int tServ = rand_r(&seed) % (60) + 20; //Cosi facendo, sono sicuro che 20<=tServ<=80

	//Inizio a contare il tempo per sapere il tempo totale del supermercato
	struct timespec start={0,0};
	clock_gettime(CLOCK_REALTIME, &start);

	int numprodtot=0, numclienti=0, nclose=0;
	float tmedserv=0, ttotserv=0;

	#ifdef DEBUG
		printf("Cassa %d entra in funzione\n", id);
	#endif

	int openbefore=0;

	while (s_sigquit==0) {

		if (s_sighup==1) {
			pthread_mutex_lock(&inCustLock);
			if (inCustomers==0) {
				pthread_mutex_unlock(&inCustLock);
				break;
			} else pthread_mutex_unlock(&inCustLock);
		}

		int exit=0;	//Variabile per vedere se devo uscire perche chiude il supermercato
		int cont=0;
		pthread_mutex_lock(&casseCode[id].openLock);
		if (casseCode[id].open == 0) {
			cont=1;
			if (openbefore==1) {
				nclose++;
				openbefore=0;
				if (getSize(casseCode[id].codaClienti)>0) {
					deleteCoda(casseCode[id].codaClienti);	//Cosi, se la coda aveva dei clienti ma e' stata chiusa dal direttore, elimino la coda di clienti, che si spostano in altre code
					casseCode[id].codaClienti=initCoda();
				}
			}
		} else openbefore=1;
		pthread_mutex_unlock(&casseCode[id].openLock);
		
		if (s_sigquit==1) {
			if (getSize(casseCode[id].codaClienti)==0) exit=1;
		} else if (s_sighup==1) {
			pthread_mutex_lock(&inCustLock);
			if (inCustomers==0) exit=1;
			pthread_mutex_unlock(&inCustLock);
		}

		if (cont==1) {	//Continuo se la cassa e' chiusa, cosi da non aspettare infinitamente che la cassa aspetti di essere aperta mentre magari il supermercato sta chiudendo
			continue;
		}
		if (exit==1) {
			pthread_cond_broadcast(&casseCode[id].servitoCond);
			break;
		}

		//Se ho clienti in coda
		if (getSize(casseCode[id].codaClienti)>0) {

			#ifdef DEBUG
				printf("Cassa %d sta per fare la pop\n", id);
			#endif

			clienteincoda *cl;
			cl=pop(casseCode[id].codaClienti);
			
			#ifdef DEBUG
				printf("Cassa %d sta servendo cliente %d\n", id, cl->id);
			#endif
			
			numprodtot+=cl->numprod;
			numclienti++;	
			
			//Cassiere dorme per tServ piu numero di oggetti del cliente che sta servendo
			struct timespec servtime = { 0, (tServ+(conf->S*cl->numprod))*1000000 };
			float serv=((double)servtime.tv_sec + 1.0e-9*servtime.tv_nsec); 
			nanosleep(&servtime, NULL);
			ttotserv+=serv;
			tmedserv=(ttotserv)/numclienti;
			
			#ifdef DEBUG
				printf("Cassa %d ha dormito per %.3f s, ora cambia la variabile servito al cliente %d\n", id, serv, cl->id);
				int idcliente=cl->id;
			#endif

			//Il cliente e' stato servito, quindi posso segnalarlo alla coda
			pthread_mutex_lock(&casseCode[id].servitoLock);
			*(cl->servito)=1;
			pthread_cond_broadcast(&casseCode[id].servitoCond);
			pthread_mutex_unlock(&casseCode[id].servitoLock);

			#ifdef DEBUG
				printf("Cassa %d ha servito cliente %d\n", id, idcliente);
			#endif

			//free(cl->servito);
			//free(cl);
		}
		
	}

	//Calcolo tempo finale
	struct timespec closetime={0,0};
	clock_gettime(CLOCK_REALTIME, &closetime);
	float ttot=((double)closetime.tv_sec + 1.0e-9*closetime.tv_nsec)-((double)start.tv_sec + 1.e-9*start.tv_nsec);

	//Scrivi nel logfile
	pthread_mutex_lock(&logLock);
	fprintf(logfile, "CHECKOUT | id:%d | # prod tot:%d | # clienti:%d | t tot aperta:%.3f s | t med serv:%.3f s | # chiusure:%d |\n", id, numprodtot, numclienti, ttot, tmedserv, nclose); fflush(logfile);
	pthread_mutex_unlock(&logLock);

	#ifdef DEBUG
		printf("Cassa %d chiusa\n", id);
	#endif
	
	pthread_exit(NULL);

}

//---------------------------------------------------------------//
//---------------------- Customer Thread ------------------------//
//---------------------------------------------------------------//
//Thread che gestisce il singolo cliente
void *customers (void *arg) {
	
	pthread_mutex_lock(&inCustLock);	//Aumento il numero di clienti all'interno
	inCustomers++;
	pthread_mutex_unlock(&inCustLock);

	int id=*(int*)arg;

	#ifdef DEBUG
		printf("Cliente %d entra nel supermercato\n", id);
	#endif

	pthread_mutex_lock(&totCustLock);	//Aumento il numero di clienti entrati in totale nel supermercato
	totCustomers++;
	pthread_mutex_unlock(&totCustLock);

	//Inizio a contare il tempo per sapere il tempo totale nel supermercato
	struct timespec start={0,0};
	clock_gettime(CLOCK_REALTIME, &start);

	//Numero di prodotti acquistati e tempo che il cliente passa a fare acquisti - random time che dipende da id del cliente
	int seed = id;
	unsigned int numprod = rand_r(&seed) % (conf->P);
	unsigned int tAcq = rand_r(&seed) % (conf->T-10) + conf->T; //Cosi facendo, sono sicuro che 10<=tAcq<=T
	struct timespec buytime = { 0, tAcq*1000000 };
	nanosleep(&buytime, NULL);

	pthread_mutex_lock(&totProdLock);	//Aumento il numero di prodotti comprati
	totProd+=numprod;
	pthread_mutex_unlock(&totProdLock);

	#ifdef DEBUG
		printf("Cliente %d compra %d oggetti\n", id, numprod);
	#endif

	//Se ha 0 prodotti da comprare chiede di uscire al direttore ed esce
	if (numprod == 0) {

		#ifdef DEBUG
			printf("Cliente %d sta chiedendo di uscire senza acquisti\n", id);
		#endif

		//Aumento il numero dei clienti che vogliono uscire
		pthread_mutex_lock(&exitCustLock);
		exitCustomers++;
		pthread_mutex_unlock(&exitCustLock);

		//Chiede il permesso di uscire al direttore ed esce
		pthread_mutex_lock(&exitOKLock);
		while (exitOK==0) {
			pthread_cond_wait(&exitOKCond, &exitOKLock);
		}
		pthread_mutex_unlock(&exitOKLock);

		pthread_mutex_lock(&exitCustLock);
		exitCustomers--;
		pthread_mutex_unlock(&exitCustLock);

		pthread_mutex_lock(&inCustLock);	//Diminuisco il numero di clienti all'interno
		inCustomers--;
		pthread_mutex_unlock(&inCustLock);
		
		pthread_mutex_lock(&outCustLock);	//Aumento il numero di clienti usciti
		outCustomers++;
		pthread_mutex_unlock(&outCustLock);

		#ifdef DEBUG
			printf("Cliente %d e' uscito senza acquisti\n", id);
		#endif

		//Calcolo tempo e scrivo i dati nel logfile
		struct timespec exittime={0,0};
		clock_gettime(CLOCK_REALTIME, &exittime);
		float ttot=((double)exittime.tv_sec + 1.0e-9*exittime.tv_nsec)-((double)start.tv_sec + 1.e-9*start.tv_nsec);
		pthread_mutex_lock(&logLock);
		fprintf(logfile, "CUSTOMER | id:%d | prod acq:0 | t tot supermarket:%.3f s | t tot coda:0 s | #code visit:0 |\n", id, ttot); fflush(logfile);
		pthread_mutex_unlock(&logLock);

		pthread_exit(NULL);
	}

	int numcode=0; //Numero di code visitate
	
	//Inizia a contare il tempo che il cliente sta in coda
	struct timespec startq={0,0};
	clock_gettime(CLOCK_REALTIME, &startq);

	int numcassa=0;

	while (s_sigquit==0) {

		#ifdef DEBUG
			printf("Cliente %d cerca cassa a cui accodarsi\n", id);
		#endif

		//Cerca la cassa finche' quella trovata non e' aperta
		while (1) {
			int seed=id+rand();	//+time(NULL); //FIXME: con time(NULL) (e anche senza) avevo sempre lo stesso numero
			numcassa = rand_r(&seed) % conf->K;
			pthread_mutex_lock(&casseCode[numcassa].openLock);
			if (casseCode[numcassa].open==0) {

				#ifdef DEBUG
					printf("Cliente %d ha trovato cassa %d chiusa, ora ne cerca un'altra\n", id, numcassa);
				#endif

				pthread_mutex_unlock(&casseCode[numcassa].openLock);
				continue;
			}
			pthread_mutex_unlock(&casseCode[numcassa].openLock);
			break;
		}

		#ifdef DEBUG
			printf("Cliente %d deve mettersi in coda alla cassa %d\n", id, numcassa);
		#endif

		//Push del cliente in coda alla cassa
		clienteincoda *cl;
		cl=malloc(sizeof(clienteincoda));
		cl->id=id;
		cl->numprod=numprod;
		cl->servito=(int*)malloc(sizeof(int));
		*(cl->servito)=0; 
		if (push(casseCode[numcassa].codaClienti, cl) == -1) {
			fprintf(stderr, "Error pushing customer #%d in queue\n", id);
			free(cl->servito);
			free(cl);
			pthread_exit(NULL);
		}
		numcode++;

		#ifdef DEBUG
			printf("Cliente %d si e' messo in coda alla cassa %d, che ora ha una coda di lunghezza %d\n", id, numcassa, getSize(casseCode[numcassa].codaClienti));
		#endif

		if (s_sigquit==0) {
			//Cliente e' in coda. Se la cassa chiude nel frattempo, si sposta
			pthread_mutex_lock(&casseCode[numcassa].servitoLock);
			int findnewline=0, segnale=0;
			while (*(cl->servito) == 0) {
				if (s_sigquit==1) {
					segnale=1;
					break;
				}
				pthread_mutex_lock(&casseCode[numcassa].openLock);
				if (casseCode[numcassa].open==0) {
					findnewline=1;
				}
				pthread_mutex_unlock(&casseCode[numcassa].openLock);
				if (findnewline==0) {
					pthread_cond_wait(&casseCode[numcassa].servitoCond, &casseCode[numcassa].servitoLock);
				} else {
					break;
				}
			}
			pthread_mutex_unlock(&casseCode[numcassa].servitoLock);
			free(cl->servito);
			free(cl);
			if (findnewline==0 || segnale==1) {
				break;
			}
		} else {
			free(cl->servito);
			free(cl);
			break;
		}
		
	}

	#ifdef DEBUG 
		printf("Cliente %d e' stato servito dalla cassa %d\n", id, numcassa);
	#endif

	//Cliente esce dal supermercato
	pthread_mutex_lock(&inCustLock);	//Diminuisco il numero di clienti all'interno
	inCustomers--;
	pthread_mutex_unlock(&inCustLock);

	#ifdef DEBUG
		printf("Cliente %d ha diminuito inCustomers\n", id);
	#endif

	pthread_mutex_lock(&outCustLock);	//Aumento il numero di clienti usciti
	outCustomers++;
	pthread_mutex_unlock(&outCustLock);

	#ifdef DEBUG
		printf("Cliente %d sta per misurare tempo di fine\n", id);
	#endif

	//Calcolo tempi di fine
	struct timespec exittime={0,0};
	clock_gettime(CLOCK_REALTIME, &exittime);
	float ttot=((double)exittime.tv_sec + 1.0e-9*exittime.tv_nsec)-((double)start.tv_sec + 1.e-9*start.tv_nsec);
	float tcoda=((double)exittime.tv_sec + 1.0e-9*exittime.tv_nsec)-((double)startq.tv_sec + 1.e-9*startq.tv_nsec);

	//Scrivo i dati nel logfile
	pthread_mutex_lock(&logLock);
	fprintf(logfile, "CUSTOMER | id:%d | prod acq:%d | t tot supermarket:%0.3f s | t tot coda:%0.3f s | #code visit:%d |\n", id, numprod, ttot, tcoda, numcode); fflush(logfile);
	pthread_mutex_unlock(&logLock);

	#ifdef DEBUG
		printf("Cliente %d esce dal supermercato\n", id);
	#endif

	pthread_exit(NULL);

}


//---------------------------------------------------------------//
//-------------------- SM Management Thread ---------------------//
//---------------------------------------------------------------//
//Thread che gestisce casse e clienti
void *supermarketManagement (void *arg) {

	int openCheckout=conf->startupCheckout;

	//Creo un array lungo K per i thread cassa
	pthread_t *checkout;
	if ((checkout=malloc(conf->K*sizeof(pthread_t))) == NULL) {
		perror("Error creating array of checkout threads");
		pthread_exit(NULL);
	}

	#ifdef DEBUG
		printf("Array di thread casse creato\n");
	#endif

	//Inizializzo l'array di casse e code di clienti
	if ((casseCode=malloc(conf->K*sizeof(casse_sm))) == NULL) {
		perror("Error creating array of checkouts with customers queues");
		pthread_exit(NULL);
	}
	for (int i=0; i<conf->K; ++i) {
		casseCode[i].id=i;
		casseCode[i].open=0;
		casseCode[i].codaClienti=initCoda();
		if (pthread_cond_init(&casseCode[i].servitoCond, NULL) != 0) {
			fprintf(stderr, "Error initializing varcond servito #%d in casseCode\n", i); 
			pthread_exit(NULL);
		}
		if (pthread_mutex_init(&casseCode[i].servitoLock, NULL) != 0) {
			fprintf(stderr, "Error initializing lock servito #%d in casseCode\n", i);
			pthread_exit(NULL);
		}
		if (pthread_mutex_init(&casseCode[i].openLock, NULL) != 0) {
			fprintf(stderr, "Error initializing lock open #%d in casseCode\n", i);
			pthread_exit(NULL);
		}
	}

	//Apro le casse che dovranno essere aperte all'apertura del supermercato e lancio i rispettivi thread passandogli l'id della cassa
	int *idcasse;
	idcasse=malloc(conf->K*sizeof(int));
	for (int i=0; i<conf->K; ++i) {
		idcasse[i]=i;
	}
	for (int i=0; i<conf->K; ++i) { 
		if (pthread_create(&checkout[i], NULL, checkouts, &idcasse[i]) != 0) {
				fprintf(stderr, "Error creating thread for checkout #%d\n", i); 
				pthread_exit(NULL);
		}
		if (i<openCheckout) {
			pthread_mutex_lock(&casseCode[i].openLock);
			casseCode[i].open=1;
			pthread_mutex_unlock(&casseCode[i].openLock);
		}
	}

	pthread_mutex_lock(&initLock);
	inizializzato=1;
	pthread_cond_signal(&initCond);
	pthread_mutex_unlock(&initLock);

	#ifdef DEBUG
		printf("Inizializzazione casse finita, lanciati i primi %d thread cassa\n", conf->startupCheckout);
	#endif

	//Gestione casse con S1 e S2
	int opencasse=openCheckout;
	while (s_sighup==0 && s_sigquit==0) {
		pthread_mutex_lock(&notifLock);
		if (notified==1) {	//Se c'e una nuova notifica
			int s1=0, s2=0;	//s1 e' il # di casse che hanno <=1 cliente		//s2=1 se c'e almeno una cassa con #clienti >= conf->S2
			for (int i=0; i<conf->K; ++i) {
				if (sizeCode[i]<=1) {
					s1++;
				} else if (sizeCode[i] >= conf->S2) {
					s2=1;
				}
			}
			if (!(s1 >= conf->S1 && s2==1)) {	//Se per s1 va chiusa una cassa e per s2 ne va aperta un'altra, non faccio niente
				if (s1>=conf->S1) { 
					//Chiudo una cassa se ne ho piu' di una aperta
					if (opencasse>1) {
						for (int i=0; i<conf->K; ++i) {
							pthread_mutex_lock(&casseCode[i].openLock);
							if (casseCode[i].open == 1) {

								#ifdef DEBUG
									printf("Direttore chiude la cassa %d\n", i);
								#endif

								casseCode[i].open=0;
								opencasse--;
								pthread_mutex_unlock(&casseCode[i].openLock);
								break;
							}
							pthread_mutex_unlock(&casseCode[i].openLock);
						}
					}	
				} else if (s2==1) {
					//Apro una cassa se non sono gia' tutte aperte
					if (opencasse < conf->K) {
						for (int i=0; i<conf->K; ++i) {
							pthread_mutex_lock(&casseCode[i].openLock);
							if (casseCode[i].open == 0) {

								#ifdef DEBUG
									printf("Direttore apre la cassa %d\n", i);
								#endif
								
								casseCode[i].open=1;
								opencasse++;
								pthread_mutex_unlock(&casseCode[i].openLock);
								break;
							}
							pthread_mutex_unlock(&casseCode[i].openLock);
						}
					}
				}
			}
			notified=0;
		}
		pthread_mutex_unlock(&notifLock); 
	}

	//Thread join
	for (int i=0; i<conf->K; ++i) {
		if (pthread_join(checkout[i], NULL) != 0) {
			fprintf(stderr, "Error waiting for thread checkout #%d\n", i);
			pthread_exit(NULL);
		}
	}

	#ifdef DEBUG
		printf("Tutte le casse sono chiuse, esco dal thread smManage\n");
	#endif

	//Uscita
	free(checkout);
	free(idcasse);
	pthread_exit(NULL);

}

//---------------------------------------------------------------//
//----------------- Customers Management Thread -----------------//
//---------------------------------------------------------------//
//Thread che gestisce entrata e uscita dei clienti
void *customerManagement (void *arg) {

	int lastTotCustomers = conf->C;

	//Creo un array di thread clienti lungo conf->C
	pthread_t *thClienti; 
	if ((thClienti=(pthread_t*)malloc(conf->C*sizeof(pthread_t))) == NULL) {
		perror("Error creating array of customers");
		pthread_exit(NULL);
	}

	//Aspetto che le casse siano inizializzate prima che i clienti entrino nel supermercato
	pthread_mutex_lock(&initLock);
	while (inizializzato==0) {
		pthread_cond_wait(&initCond, &initLock);
	}
	pthread_mutex_unlock(&initLock);

	#ifdef DEBUG
		printf("Array di thread clienti creato\n");
	#endif

	//Lancio i thread cliente usando un array di id per l'id del cliente corrente
	int *idclienti;
	idclienti=malloc(conf->C*sizeof(int));
	for (int k=0; k<conf->C; ++k) {
		idclienti[k]=k;
	}
	int i=0;
	for (i=0; i<conf->C; i++) {
		if (pthread_create(&thClienti[i], NULL, customers, &idclienti[i]) != 0) {
			perror("Error creating thread customer");
			pthread_exit(NULL);
		}
	}

	//Lancio il thread che aggiorna il direttore
	pthread_t notifier;
	if (pthread_create(&notifier, NULL, notify, NULL) != 0) {
		perror("Error creating thread notifier");
		pthread_exit(NULL);
	}

	#ifdef DEBUG
		printf("Entrati i primi %d clienti\n", conf->K);
	#endif

	while (s_sigquit==0 && s_sighup==0) {

		//Controllo i clienti, se sono C-E ne vanno fatti entrare altri E
		pthread_mutex_lock(&outCustLock);
		if (outCustomers >= conf->E) {

			#ifdef DEBUG
				printf("Entrano altri %d clienti\n", conf->E);
			#endif

			lastTotCustomers+=conf->E;
			thClienti=(pthread_t*)realloc(thClienti, lastTotCustomers*sizeof(pthread_t));
			idclienti=(int*)realloc(idclienti, lastTotCustomers*sizeof(int));
			int ind=i;
			for (int k=0; k<conf->E; ++k) {
				idclienti[ind]=ind;
				#ifdef DEBUG
					printf("Spawna cliente %d\n", idclienti[ind]);
				#endif
				ind++;
			}
			for (int j=0; j<conf->E; ++j) {
				#ifdef DEBUG
					printf("Sta per entrare cliente %d\n", i);
				#endif
				if (pthread_create(&thClienti[i], NULL, customers, &idclienti[i]) != 0) {
					perror("Error creating thread customer");
					pthread_exit(NULL);
				}
				i++;
			}
			outCustomers=0;
		}
		pthread_mutex_unlock(&outCustLock);

		//Se c'e qualche cliente che chiede di uscire (perche' senza acquisti), dai il permesso
		pthread_mutex_lock(&exitCustLock);
		if (exitCustomers > 0) {
			pthread_mutex_lock(&exitOKLock);
			exitOK=1;
			pthread_cond_broadcast(&exitOKCond);
			pthread_mutex_unlock(&exitOKLock);
		}
		pthread_mutex_unlock(&exitCustLock);
	}

	#ifdef DEBUG
		printf("Thread custManage sta per aspettare gli altri thread customer\n");
	#endif

	//Join per i clienti
	for (int i=0; i<lastTotCustomers; ++i) {
		if (pthread_join(thClienti[i], NULL) != 0) {
			perror("Error waiting for thread customer");
			pthread_exit(NULL);
		}
	}

	//Join per il thread di notifica
	if (pthread_join(notifier, NULL) != 0) {
		perror("Error waiting for thread notifier");
		pthread_exit(NULL);
	} 

	#ifdef DEBUG
		printf("Tutti i clienti sono usciti, esco dal thread custManage\n");
	#endif

	free(thClienti);
	free(idclienti);
	pthread_exit(NULL);

}


//---------------------------------------------------------------//
//----------------- Supermarket Manager Thread ------------------//
//---------------------------------------------------------------//
//Thread direttore, gestisce le casse e entrata/uscita clienti
void *direttore (void *arg) {

	#ifdef DEBUG
		printf("Processo direttore lanciato\n");
	#endif

	//Avvio thread che gestisce le casse. Array di thread casse e coda di thread clienti
	pthread_t sm;
	if (pthread_create(&sm, NULL, supermarketManagement, NULL) != 0) {
		perror("Error creating thread supermarketManagement");
		pthread_exit(NULL);
	}

	//Avvio thread che gestisce l'entrata/uscita clienti
	pthread_t cust;
	if (pthread_create(&cust, NULL, customerManagement, NULL) != 0) {
		perror("Error creating thread customerManagement");
		pthread_exit(NULL);
	}

	#ifdef DEBUG
		printf("Il direttore ha lanciato i due thread di gestione\n");
	#endif

	//Thread join
	if (pthread_join(sm, NULL) != 0) {
		perror("Error waiting for thread supermarketManagement");
		pthread_exit(NULL);
	}
	if (pthread_join(cust, NULL) != 0) {
		perror("Error waiting for thread customerManagement");
		pthread_exit(NULL);
	}

	#ifdef DEBUG
		printf("Direttore finito\n");
	#endif

	pthread_exit(NULL);
	
}


//---------------------------------------------------------------//
//---------------------------- MAIN -----------------------------//
//---------------------------------------------------------------//
int main (int argc, char*argv[]) {

	if ((conf=getconf("./config.txt")) == NULL ) {
		fprintf(stderr, "Check the errors and the config file\n");
		exit(EXIT_FAILURE);
	}

#ifdef DEBUG
	printf("CONFIG: ");
	printconf(*conf);
#endif

	int err; //Variabile per controllare gli errori in SYSCALL

	//Gestisco i segnali SIGHUP e SIGQUIT, reindirizzati sull'handler
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler=handler;
	sigset_t handmask;
	SYSCALL(err, sigemptyset(&handmask), "Error emptying handler mask");
	SYSCALL(err, sigaddset(&handmask, SIGHUP), "Error adding SIGHUP to handler mask");
	SYSCALL(err, sigaddset(&handmask, SIGQUIT), "Error adding SIGQUIT to handler mask");
	sa.sa_mask=handmask;
	//Reindirizzo i segnali sull'handler
	SYSCALL(err, sigaction(SIGHUP, &sa, NULL), "Error redirecting SIGHUP to handler");
	SYSCALL(err, sigaction(SIGQUIT, &sa, NULL), "Error redirecting SIGQUIT to handler");

	//Apro il logfile in scrittura, codi che i thread possano scriverci
	if ((logfile=fopen(LOGFILENAME, "w")) == NULL) {
		perror("Error creating/opening log file");
		exit(errno);
	}
	fprintf(logfile, "Apre il supermercato\n\n"); fflush(logfile);

#ifdef DEBUG
	printf("Inizializzazione finita, ora apre il supermercato\n");
#endif

	//Creo il thread direttore
	pthread_t dir;
	if (pthread_create(&dir, NULL, direttore, NULL) != 0) {
		perror("Error creating thread direttore");
		exit(errno);
	}
	if (pthread_join(dir, NULL) != 0) {
		perror("Error waiting for thread direttore");
		exit(errno);
	}

#ifdef DEBUG
	printf("Il supermercato e' chiuso\n");
#endif

	//Stampo il numero di clienti totali entrati
	fprintf(logfile, "\nNum clienti entrati tot = %d\nNum prod comprati = %d\n", totCustomers, totProd); fflush(logfile);

	//Uscita
	fclose(logfile);
	free(conf);
	free(clienti_sm);
	for (int i=0; i<conf->K; ++i) {
		deleteCoda(casseCode[i].codaClienti);
	}
	free(casseCode);
	free(sizeCode);
	return 0;
}