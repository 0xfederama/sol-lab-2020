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
#include <sys/select.h>
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

int cmd(long connfd) {
    msg_t str;
    if (read(connfd, &str.len, sizeof(int))<=0) return -1;
    str.str = calloc((str.len), sizeof(char));
    if (!str.str) {
        perror("calloc");
        fprintf(stderr, "Memoria esaurita....\n");
        return -1;
    }		        
    if (read(connfd, str.str, str.len*sizeof(char))<=0) return -1;
    toup(str.str);
    if (write(connfd, &str.len, sizeof(int))<=0) { free(str.str); return -1;}
    if (write(connfd, str.str, str.len*sizeof(char))<=0) { free(str.str); return -1;}
    free(str.str);
    return 0;
}

// ritorno l'indice massimo tra i descrittori attivi
int updatemax(fd_set set, int fdmax) {
    for(int i=(fdmax-1);i>=0;--i)
	    if (FD_ISSET(i, &set)) return i;
    assert(1==0);
    return -1;
}

int main(int argc, char *argv[]) {
    cleanup();    
    atexit(cleanup);  
    
    int listenfd;
    SYSCALL(listenfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket");
    
    struct sockaddr_un serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;    
    strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);
    int notused;
    SYSCALL(notused, bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr)), "bind");
    SYSCALL(notused, listen(listenfd, MAXCONN), "listen");
    
    fd_set set, tmpset;
    // azzero sia il master set che il set temporaneo usato per la select
    FD_ZERO(&set);
    FD_ZERO(&tmpset);

    // aggiungo il listener fd al master set
    FD_SET(listenfd, &set);

    // tengo traccia del file descriptor con id piu' grande
    int fdmax = listenfd; 

    for(;;) {      
        // copio il set nella variabile temporanea per la select
        tmpset = set;
        if (select(fdmax+1, &tmpset, NULL, NULL, NULL) == -1) { // attenzione al +1
            perror("select");
            return -1;
        }
        // cerchiamo di capire da quale fd abbiamo ricevuto una richiesta
        for(int i=0; i <= fdmax; i++) {
            if (FD_ISSET(i, &tmpset)) {
                long connfd;
                if (i == listenfd) { // e' una nuova richiesta di connessione 
                    SYSCALL(connfd, accept(listenfd, (struct sockaddr*)NULL ,NULL), "accept");
                    FD_SET(connfd, &set);  // aggiungo il descrittore al master set
                    if(connfd > fdmax) fdmax = connfd;  // ricalcolo il massimo
                    continue;
                } 
                connfd = i;  // e' una nuova richiesta da un client già connesso

                // eseguo il comando e se c'e' un errore lo tolgo dal master set
                if (cmd(connfd) < 0) { 
                    close(connfd); 
                    FD_CLR(connfd, &set); 
                    // controllo se deve aggiornare il massimo
                    if (connfd == fdmax) fdmax = updatemax(set, fdmax);
                }
            }
        }
    }

    close(listenfd);
    return 0;
}
