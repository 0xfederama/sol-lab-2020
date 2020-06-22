#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/wait.h>

#if !defined(SOCKNAME)
    #define SOCKNAME "./sock"
#endif

#if !defined(BUFSIZE)
    #define BUFSIZE 256
#endif

#define SYSCALL(r,c,e) \
    if((r=c)==-1) { perror(e);exit(errno); }

void cleanup() {
    unlink(SOCKNAME);
}

int cmd(int connfd) {
    int notused;

    if (fork() == 0) {
		SYSCALL(notused, dup2(connfd, 0), "dup2 child (1)");
		SYSCALL(notused, dup2(connfd, 1), "dup2 child (2)");
		SYSCALL(notused, dup2(connfd, 2), "dup2 child (2)");

		execl("/usr/bin/bc", "bc", "-l", NULL);
		return -1;
    }
    SYSCALL(notused, wait(NULL), "Server - Error waiting for child");
    printf("Child exited\n");

    return 0;
}

int main () {

	cleanup();
	atexit(cleanup);

	//Creo il socket
	int sfd;
	SYSCALL(sfd, socket(AF_UNIX, SOCK_STREAM, 0), "Server - Error creating socket");

	//Setto l'indirizzo
	struct sockaddr_un serv_addr;
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sun_family=AF_UNIX;
	strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

	//Bindo l'indirizzo
	int unused;
	SYSCALL(unused, bind(sfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)), "Server - Error binding index");
	
	//Setta socket in ascolto
	SYSCALL(unused, listen(sfd, 1), "Server - Error listening socket");

	//Accetta connessioni e inoltra a cmd per bc
	int afd;
	while (1) {
		SYSCALL(afd, accept(sfd, (struct sockaddr*)NULL, NULL), "Server - Error accepting connections to socket");
		if (cmd(afd)<0) {
			fprintf(stderr, "Server - Error executing bc\n");
            break;
		}
		close(afd);
		printf("Connection done\n");
	}

	close(sfd);
	return 0;
}