#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>

#if !defined(SOCKNAME)
    #define SOCKNAME "./sock"
#endif

#if !defined(BUFSIZE)
    #define BUFSIZE 256
#endif

#define SYSCALL(r,c,e) \
    if((r=c)==-1) { perror(e);exit(errno); }

int main () {
	
    //Creo il socket
	int sfd;
	SYSCALL(sfd, socket(AF_UNIX, SOCK_STREAM, 0), "Client - Error creating socket");

    //Setto l'indirizzo
	struct sockaddr_un serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sun_family=AF_UNIX;
	strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

    //Connetto il socket e l'indirizzo
    int unused;
    SYSCALL(unused, connect(sfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)), "Client - Error connecting socket");

    char buf[BUFSIZE];
    while (1) {
        if (fgets(buf, BUFSIZE-1, stdin)==NULL) break;
        if (strncmp(buf, "quit", 4)==0) break;

        int err, n;
        SYSCALL(err, write(sfd, buf, strlen(buf)), "Client - Error writing in socket");
        SYSCALL(n, read(sfd, buf, BUFSIZE), "Client - Error reading from socket");

        buf[n]='\0';
        printf("Result: %s", buf);
    }

    close(sfd);
    return 0;
}