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
void dg_cli(FILE *fp, int sockfd, const SA *pservaddr, socklen_t servlen) {
    char sendline[MAXLINE], recvline[MAXLINE+1];
    int n;
    socklen_t len;
    struct sockaddr *preply_addr;
    preply_addr = malloc(servlen);
    connect(sockfd, (struct sockaddr *) pservaddr, servlen);
    while (fgets(sendline, MAXLINE, fp) != NULL) {
        /*
        sendto(sockfd, sendline, strlen(sendline), 0, pservaddr, servlen);
        len = servlen;
        n = recvfrom(sockfd, recvline, MAXLINE, 0, preply_addr, &len);
        if (len != servlen || memcmp(pservaddr, preply_addr, len) != 0) {
            printf("reply from %s (ignored)\n", sock_ntop(preply_addr, len));
            continue;
        }
        recvline[n] = 0;
        fputs(recvline, stdout);
        */
        write(sockfd, sendline, strlen(sendline));
        n = read(sockfd, recvline, MAXLINE);
        recvline[n] = 0;
        fputs(recvline, stdout);
    }
}

int main (int argc, char **argv) {
    int sockfd, n, i, id;
    char recvline[MAXLINE+1], status;
    struct sockaddr_in servaddr;
    if (strcmp(argv[2], "TCP") == 0) {
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
    } else {
        if((sockfd=socket(AF_INET, SOCK_STREAM,0)) < 0)
            printf("socket error\n");
        bzero(&servaddr,sizeof(servaddr)); /*reset address to zero */
        servaddr.sin_family=AF_INET; /*IPv4*/
        servaddr.sin_port=htons(PORT); /*Port: 13*/
        if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) 
            printf("inet_ption error for %s\n", argv[1]);
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        dg_cli(stdin, sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    }
    exit(0);
}
