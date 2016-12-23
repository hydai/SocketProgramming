#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <signal.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <poll.h>
#include <errno.h>
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
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        dg_cli(stdin, sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
        exit(0);
    }
}
