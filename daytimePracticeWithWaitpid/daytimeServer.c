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
    int listenfd, connfd, status;
    pid_t childpid;
    char line[MAXLINE];
    socklen_t clilen;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
    listenfd=socket(AF_INET, SOCK_STREAM,0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(PORT);
    bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);

    for( ;;) {
        clilen = sizeof(cliaddr);
        connfd=accept(listenfd, (SA *)&cliaddr, &clilen);
        printf("connection from %s, port %d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
        childpid = fork();

        if (childpid == 0) {
            close(listenfd);
            str_echo(connfd);
            exit(0);
        } else {
            signal(SIGCHLD, sig_chld);
        }

        close(connfd);
    }
}
