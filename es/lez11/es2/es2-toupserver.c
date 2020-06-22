#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <pthread.h>
#include <unistd.h>

#if !defined(SOCKNAME)
    #define SOCKNAME "./sock"
#endif

#if !defined(BUFSIZE)
    #define BUFSIZE 256
#endif

#if !defined(MAXCONN)
	#define MAXCONN 32
#endif

#define SYSCALL(r,c,e) \
	if((r=c)==-1) { perror(e);exit(errno); }

typedef struct msg {
    int len;
    char *str;
} msg_t;

void cleanup() {
    unlink(SOCKNAME);
}

void toup(char *str) {
    char *p = str;
    while(*p != '\0') { 
        *p = islower(*p) ? toupper(*p) : *p; 
		++p;
    }        
}

void *threadFun(void *arg) {
    assert(arg);
    long connfd = (long)arg;
    do {
		msg_t str;
		int n;
		SYSCALL(n, read(connfd, &str.len, sizeof(int)), "read1");
		if (n==0) break;
		str.str = calloc((str.len), sizeof(char));
		if (!str.str) {
			perror("calloc");
			fprintf(stderr, "Memoria esaurita....\n");
			break;
		}		    
		SYSCALL(n, read(connfd, str.str, str.len * sizeof(char)), "read2");

		toup(str.str);

		SYSCALL(n, write(connfd, &str.len, sizeof(int)), "write");
		SYSCALL(n, write(connfd, str.str, str.len*sizeof(char)), "write");
		free(str.str);
    } while(1);
    close(connfd);	    
    return NULL;
}

void spawn_thread(long afd) {
    pthread_attr_t thattr;
    pthread_t thid;

    if (pthread_attr_init(&thattr) != 0) {
		fprintf(stderr, "pthread_attr_init FALLITA\n");
		close(afd);
		return;
    }
    //Setto il thread in modalità detached
    if (pthread_attr_setdetachstate(&thattr,PTHREAD_CREATE_DETACHED) != 0) {
		fprintf(stderr, "pthread_attr_setdetachstate FALLITA\n");
		pthread_attr_destroy(&thattr);
		close(afd);
		return;
    }
    if (pthread_create(&thid, &thattr, threadFun, (void*)afd) != 0) {
		fprintf(stderr, "pthread_create FALLITA");
		pthread_attr_destroy(&thattr);
		close(afd);
		return;
    }
}

int main () {

	cleanup();
	atexit(cleanup);

	//Creo il socket del server
	int sfd;
	SYSCALL(sfd, socket(AF_UNIX, SOCK_STREAM, 0), "Server - Error creating socket");

	//Setto l'indirizzo
	struct sockaddr_un serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sun_family=AF_UNIX;
	strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

	//Bindo l'indirizzo
	int unused;
	SYSCALL(unused, bind(sfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)), "Server - Error binding address");

	//Setto socket in ascolto
	SYSCALL(unused, listen(sfd, MAXCONN), "Server - Error listening socket");

	//Accetta connessioni e crea thread per ognuna
	while(1) {
		long afd;
		SYSCALL(afd, accept(sfd, (struct sockaddr*)NULL, NULL), "Server - Error accepting connections to socket");
		//Creo thread
		spawn_thread(afd);
	}

	close(sfd);
	return 0;
}