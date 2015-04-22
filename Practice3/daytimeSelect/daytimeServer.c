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

int main(int argc, char **argv) {
    int i, maxi, maxfd;
    int nready, client[FD_SETSIZE];
    int listenfd, connfd, sockfd;
    ssize_t n;
    fd_set rset, allset;
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
    maxfd = listenfd; // init
    maxi = -1;
    for (i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;
    }
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    for( ;;) {
        rset = allset;
        nready = select(maxfd+1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(listenfd, &rset)) {
            clilen = sizeof(cliaddr);
            connfd=accept(listenfd, (SA *)&cliaddr, &clilen);
            printf("Log: connection from %s, port %d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            }
            if (i == FD_SETSIZE) {
                printf("Error: too many clients\n");
            }
            FD_SET(connfd, &allset);
            if (connfd > maxfd) {
                maxfd = connfd;
            }
            if (i > maxi) {
                maxi = i;
            }
            if (--nready <= 0) {
                continue;
            }
        }

        for (i = 0; i <= maxi; i++) {    
            if ((sockfd = client[i]) < 0) {
                continue;
            }
            if (FD_ISSET(sockfd, &rset)) {
                if ( (n = read(sockfd, line, MAXLINE)) == 0) {
                    printf("Log: sockfd %d: empty input\n", sockfd);
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else {
                    write(sockfd, line, n+1);
                    printf("Log: from sockfd %d: %s\n", sockfd, line);
                }
                if (--nready <= 0) {
                    break;
                }
            }
        }
    }
}
