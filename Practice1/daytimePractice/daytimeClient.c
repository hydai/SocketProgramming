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

int main (int argc, char **argv) {
    int sockfd, n;
    char recvline[MAXLINE+1];
    struct sockaddr_in servaddr; /*declare server address*/
    if(argc!=2)
        /*deal with error message*/ 
        printf("usage: a.out <IPaddress>\n");
    if((sockfd=socket(AF_INET, SOCK_STREAM,0)) < 0) printf("socket error\n");
    /*create an Internet(AF_INET) stream(SOCK_STREAM) socket*/
    bzero(&servaddr,sizeof(servaddr)); /*reset address to zero */
    servaddr.sin_family=AF_INET; /*IPv4*/
    servaddr.sin_port=htons(PORT); /*Port: 13*/
    if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) 
        printf("inet_ption error for %s\n", argv[1]);
    if(connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0) 
        printf("connect error\n");
    /* SA=struct soketaddr */
    while ((n=read(sockfd, recvline, MAXLINE)) >0) { 
        recvline[n]=0; /* null terminate */
        if (fputs(recvline, stdout) == EOF) 
            printf("fputs error\n"); 
    }
    if (n<0)
        printf("read error\n");
    exit(0);
}
