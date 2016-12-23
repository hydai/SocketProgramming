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

int max(int a, int b) {
    return (a > b)?a:b;
}
void sig_chld(int signo) {
    pid_t pid;
    int stat;
    while((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        printf("child %d terminated\n", pid);
    }
    return;
}

void str_echo(int sockfd) {
    ssize_t n;
    char buf[MAXLINE];
again:
    while ( (n = read(sockfd, buf, MAXLINE)) > 0) {
        write(sockfd, buf, n);
        printf("From %d: %s\n", sockfd, buf);
    }
    if (n < 0 && errno == EINTR)
        goto again;
    else if(n<0)
      printf("str_echo: read error");
}

int main(int argc, char **argv) {
    int i, maxi, maxfd;
    int nready, client[FD_SETSIZE];
    int listenfd, connfd, sockfd, udpfd, maxfdp1;
    ssize_t n;
    fd_set rset, allset;
    char mesg[MAXLINE];
    socklen_t clilen;
    pid_t childpid;
    const int on = 1;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    listenfd=socket(AF_INET, SOCK_STREAM,0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(PORT);

    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);

    // UDP
    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(PORT);
    bind(udpfd, (SA *)&servaddr, sizeof(servaddr));

    signal(SIGCHLD, sig_chld);
    FD_ZERO(&rset);
    maxfdp1 = max(listenfd, udpfd)+1;
    for( ;;) {
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);
        nready = select(maxfdp1, &rset, NULL, NULL, NULL);
        if (nready < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                printf("Error: select error\n");
            }
        }
        if (FD_ISSET(listenfd, &rset)) {
            clilen = sizeof(cliaddr);
            connfd=accept(listenfd, (SA *)&cliaddr, &clilen);
            if ((childpid = fork()) == 0) {
                close(listenfd);
                str_echo(connfd);
                exit(0);
            }
            printf("Log: connection from %s, port %d, childid = %d by TCP\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), childpid);
            close(connfd);
        }
        if (FD_ISSET(udpfd, &rset)) {
            clilen = sizeof(cliaddr);
            printf("Log: connection from %s, port %d by UDP\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
            n = recvfrom(udpfd, mesg, MAXLINE, 0, (struct sockaddr*)&cliaddr, &clilen);
            sendto(udpfd, mesg, n, 0, (struct sockaddr*)&cliaddr, clilen);
        }
    }
}
