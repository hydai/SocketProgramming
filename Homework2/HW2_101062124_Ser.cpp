#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <iostream>
#include <sqlite3.h>
#include "const.h"
#include "hw2_lib.h"

int main(int argc, char **argv) {
    sqlite3 *db; // sqlite3 db struct
    char recvline[MAXLINE + 1];
    int n, port;
    int sockfd;
    fd_set rset, allset;
    struct sockaddr_in seraddr, cliaddr;
    if (argc < 2) {
        std::cerr << "Usage: ./HW2_101062124_Ser [port]\n";
        return 1;
    }
    sockfd = create_UDP_Server(seraddr, std::atoi(argv[1]));
    
    bool db_status = init_db(db);
    if (!db_status) {
        std::cerr << "Fatal error in init_db\n";
        return 1;
    }
    ser_echo(sockfd);
    close_db(db);
    return 0;
}
