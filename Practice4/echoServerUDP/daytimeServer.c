#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <signal.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <poll.h>
#include <errno.h>
#include "const.h"

void dg_echo(int sockfd, struct sockaddr *pcliaddr, socklen_t clilen) {
    int n;
    socklen_t len;
    char mesg[MAXLINE];
    struct sockaddr_in *cliaddr;
    for (;;) {
        len = clilen;
        n = recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);
        cliaddr = (struct sockaddr_in *)pcliaddr;
        printf("Data from %s, port %d: %s", inet_ntoa(cliaddr->sin_addr), ntohs(cliaddr->sin_port), mesg);
        sendto(sockfd, mesg, n, 0, pcliaddr, len);
    }
}

int main(int argc, char **argv) {
    int sockfd, connfd, status;
    pid_t childpid;
    char line[MAXLINE];
    socklen_t clilen;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    sockfd=socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(PORT);
    bind(sockfd, (SA *)&servaddr, sizeof(servaddr));
    dg_echo(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr));
    /*
    for( ;;) {
        clilen = sizeof(cliaddr);
        connfd=accept(listenfd, (SA *)&cliaddr, &clilen);
        printf("connection from %s, port %d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
        childpid = fork();

        if (childpid == 0) {
            close(listenfd);
            dg_echo(connfd);
            exit(0);
        } else {
            signal(SIGCHLD, sig_chld);
        }
        close(connfd);
    }
    */
}
