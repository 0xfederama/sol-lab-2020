#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
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

#define SYSCALL(r,c,e) \
	if((r=c)==-1) { perror(e);exit(errno); }

int main (int argc, char*argv[]) {
	if (argc==1) {
		fprintf(stderr, "Usage: %s string [string]\n", argv[0]);
		return -1;
	}

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

    char *buffer=NULL;
	for (int i=1; i<argc; ++i) {
		int n=strlen(argv[i])+1;
		SYSCALL(unused, write(sfd, &n, sizeof(int)), "Client - Error writing strlen in socket");
		SYSCALL(unused, write(sfd, argv[i], n*sizeof(char)), "Client - Error writing argv in socket");

		buffer = realloc(buffer, n*sizeof(char));
		if (!buffer) {
			perror("realloc");
			fprintf(stderr, "Memoria esaurita....\n");
			break;
		}

		SYSCALL(unused, read(sfd, &n, sizeof(int)), "Client - Error reading strlen from socket");
		SYSCALL(unused, read(sfd, buffer, n*sizeof(char)), "Client - Error reading string from socket");
		buffer[n]='\0';

		printf("String: %s\n", buffer);
	}

	close(sfd);
	if (buffer) free(buffer);
	
	return 0;
}