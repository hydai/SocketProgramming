#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "const.h"

int max(int a, int b) {
    return (a>b)?a:b;
}
int str_cli(FILE *fp, int sockfd) {
    char sendline[MAXLINE], recvline[MAXLINE];
    int maxfdp1, stdineof = 0;
    fd_set rset;
    int n;
    FD_ZERO(&rset);
    for(;;) {
        if (stdineof == 0) {
            FD_SET(fileno(fp), &rset);
        }
        FD_SET(sockfd, &rset);
        maxfdp1 = max(fileno(fp), sockfd) + 1;
        select(maxfdp1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(sockfd, &rset)) {
            if (read(sockfd, recvline, MAXLINE) == 0) {
                if (stdineof == 1)
                    while(1) {
                        fgets(sendline, MAXLINE, fp);
                    }
                else
                    printf("str_cli: server terminated prematurely\n");
            }
            fputs(recvline, stdout);
        }
        if (FD_ISSET(fileno(fp), &rset)) {
            if (fgets(sendline, MAXLINE, fp) == NULL) {
                stdineof = 1;
                shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(fp), &rset);
                continue;
            }
            sendline[strlen(sendline)] = '\0';
            write(sockfd, sendline, strlen(sendline)+1);
        }
    }
    /*
    while (~fscanf(fp, "%s", sendline)) {
        printf("Log: get from stdin: %s\n", sendline);
        printf("Log: %s sent to server\n\n", sendline);
        write(sockfd, sendline, strlen(sendline)+1);
        if ((n = read(sockfd, recvline, MAXLINE)) == 0) {
            printf("str_cli: server terminated prematurely\n");
            exit(0);
        } else {
            recvline[n+1] = '\0';
            printf("Recive: %s\n", recvline);
        }
        printf("Log: =======================\n");

        printf("> ");
    }
    */
    return 0;
}

int main (int argc, char **argv) {
    int sockfd, n, i, id;
    char recvline[MAXLINE+1], status;
    struct sockaddr_in servaddr;
    if(argc!=2)
        printf("usage: a.out <IPaddress>\n");
    if((sockfd=socket(AF_INET, SOCK_STREAM,0)) < 0)
        printf("socket error\n");
    bzero(&servaddr,sizeof(servaddr)); /*reset address to zero */
    servaddr.sin_family=AF_INET; /*IPv4*/
    servaddr.sin_port=htons(PORT); /*Port: 13*/
    if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) 
        printf("inet_ption error for %s\n", argv[1]);
    if(connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0) 
        printf("connect error\n");
    status = 1;

    while (status) {
        status = str_cli(stdin, sockfd);
        if (status == 0) {
            while(fgets(recvline, MAXLINE, stdin));
        }
    }
    exit(0);
}
