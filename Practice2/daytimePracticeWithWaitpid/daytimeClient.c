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

void str_cli(FILE *fp, int sockfd) {
    char sendline[MAXLINE], recvline[MAXLINE];
    while (~fscanf(fp, "%s", sendline)) {
        printf("Get fs %s\n", sendline);
        write(sockfd, sendline, strlen(sendline)+1);
        if (read(sockfd, recvline, MAXLINE) == 0) {
            printf("str_cli: server terminated prematurely\n");
            exit(0);
        }
        printf("Recive: %s\n", recvline);
    }
}

int main (int argc, char **argv) {
    int sockfd, n, i, id;
    char recvline[MAXLINE+1];
    struct sockaddr_in servaddr;
    if(argc!=2)
        printf("usage: a.out <IPaddress>\n");
    for (i = 0; i < 5; i++) {
        if((sockfd=socket(AF_INET, SOCK_STREAM,0)) < 0)
            printf("socket error\n");
        bzero(&servaddr,sizeof(servaddr)); /*reset address to zero */
        servaddr.sin_family=AF_INET; /*IPv4*/
        servaddr.sin_port=htons(PORT); /*Port: 13*/
        if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) 
            printf("inet_ption error for %s\n", argv[1]);
        if(connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0) 
            printf("connect error\n");
    }
    printf("> ");
    while (~scanf("%d", &id)) {
        printf("sockid = %d\n", id+2);
        str_cli(stdin, id+2);
        printf("> ");
    }
    exit(0);
}
