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
#include <sys/wait.h>
#include "const.h"
int main(int argc, char **argv) {
    int listenid, connfd, status;
    struct sockaddr_in servaddr;
    struct sockaddr_in client;
    socklen_t addr_size;
    char buff[MAXLINE];
    time_t ticks;
    listenid=socket(AF_INET, SOCK_STREAM,0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(PORT);
    bind(listenid, (SA *)&servaddr, sizeof(servaddr));
    listen(listenid, LISTENQ);
    for( ;;) {
        addr_size = sizeof(client);
        connfd=accept(listenid, (SA *)&client, &addr_size);

        printf("connection from %s, port %d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        pid_t pid = fork();
        if (pid == 0) {
            /* Child process. */
            printf("execute netstat:\n");
            system("netstat");
            exit(0);
        } else if (pid < 0) {
            /* fork failed. */
            status = -1;
        } else {
            waitpid(pid, &status, 0);
            printf("netstat finished!!\n");
        }
        ticks=time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks)); write(connfd, buff, strlen(buff));
        close(connfd);
    }
}
