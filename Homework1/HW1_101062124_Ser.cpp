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
#include "const.h"
#include <sstream>
#include <string>
#include <map>

void sig_chld(int signo);
void str_echo(int sockfd);
std::map<int, std::string> M;
std::string getIP(int pid);
void setIP(int pid, std::string str);

int main(int argc, char **argv) {
    int listenfd, connfd, status;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;

    mkdir("Upload", 0777);

    listenfd=socket(AF_INET, SOCK_STREAM,0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
    servaddr.sin_port=htons(PORT);
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    M.clear();
    signal(SIGCHLD, sig_chld);

    for( ; ; ) {
        clilen = sizeof(cliaddr);
        connfd=accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        printf("connection from %s, port %d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
        childpid = fork();

        if (childpid == 0) {
            close(listenfd);
            str_echo(connfd);
            exit(0);
        } else {
            setIP(childpid, (std::string(inet_ntoa(cliaddr.sin_addr))+":"+std::to_string(ntohs(cliaddr.sin_port))));
        }
        close(connfd);
    }
}

void setIP(int pid, std::string str) {
    M[pid] = str;
}

std::string getIP(int pid) {
    return M[pid];
}

void sig_chld(int signo) {
    pid_t pid;
    int stat;
    while((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        printf("child pid = %d, ip = %s terminated\n", pid, getIP(pid).c_str());
    }
    return;
}

void str_echo(int sockfd) {
    int n;

again:
    {
        DIR *dir;
        struct dirent *entry;
        char buf[MAXLINE+1];
        char data[POCKET_SIZE];
        char *tmp_buf = NULL;
        char *ls = NULL;
        int ls_len = 0;
        int file_size;
        FILE *fptr = NULL;
        while ( (n = read(sockfd, buf, MAXLINE)) > 0) {
            buf[strlen(buf)] = '\0'; // End of line
            if (!strncmp(buf, CD, strlen(CD))) {
                // reply CD signal by ACK
                write(sockfd, ACK, strlen(ACK)+1);
                // Get path
                n = read(sockfd, buf, MAXLINE);
                buf[strlen(buf)] = '\0';
                chdir(buf);
                getcwd(buf, MAXLINE);
                // Send pwd
                write(sockfd, buf, MAXLINE);
                // Get ACK
                read(sockfd, buf, MAXLINE);
            } else if (!strncmp(buf, LS, strlen(LS))) {
                dir = opendir(".");
                std::stringstream ss;
                while ((entry = readdir(dir)) != NULL) {
                    ss << (entry->d_name) << "\n";
                }
                closedir(dir);
                ls_len = ss.str().length()+1;
                ls = (char *)calloc(ls_len, sizeof(char));
                strcpy(ls, ss.str().c_str());
                write(sockfd, ls, ls_len);
                // Get ACK
                read(sockfd, buf, MAXLINE);
            } else if (!strncmp(buf, UPLOAD, strlen(UPLOAD))) {
                // reply UPLOAD signal by ACK
                write(sockfd, ACK, strlen(ACK)+1);
                // Get file name
                read(sockfd, buf, MAXLINE);
                // Send ACK
                write(sockfd, ACK, strlen(ACK)+1);
                // Get file size
                read(sockfd, &file_size, sizeof(file_size));
                printf("Client upload file %s (size = %d) to server\n", buf, file_size);
                // Send ACK
                write(sockfd, ACK, strlen(ACK)+1);
                fptr = fopen(buf, "wb");
                while (file_size > 0) {
                    if (file_size > POCKET_SIZE) {
                        // Write a complex pocket
                        read(sockfd, data, POCKET_SIZE);
                        fwrite(data, sizeof(char), POCKET_SIZE, fptr);
                        // Send ACK
                        write(sockfd, ACK, strlen(ACK)+1);
                        file_size -= POCKET_SIZE;
                    } else {
                        // Write a complex pocket
                        read(sockfd, data, file_size);
                        fwrite(data, sizeof(char), file_size, fptr);
                        // Send ACK
                        write(sockfd, ACK, strlen(ACK)+1);
                        file_size = 0;
                    }
                }
                fclose(fptr);
                printf("File upload complete.\n");
                // Get ACK
                read(sockfd, buf, MAXLINE);
            } else if (!strncmp(buf, DOWNLOAD, strlen(DOWNLOAD))) {
                char file_name[2*MAXLINE] = {0};
                // Write ACK
                write(sockfd, ACK, strlen(ACK)+1);

                // Get file name
                n = read(sockfd, buf, MAXLINE);
                buf[n] = '\0';
                strcpy(file_name, buf);
                getcwd(buf, MAXLINE);
                fptr = fopen(file_name, "rb");
                if (!fptr) {
                    printf("File %s is not avaliable\n", file_name);
                    file_size = 0;
                    write(sockfd, &file_size, sizeof(file_size));
                    // Get ACK
                    read(sockfd, buf, MAXLINE);
                    continue;
                }
                // Send file size
                fseek(fptr, 0, SEEK_END); // Go to end
                file_size = ftell(fptr);
                rewind(fptr);
                write(sockfd, &file_size, sizeof(file_size));

                printf("Client download file %s (size = %d) to server\n", file_name, file_size);
                // Send file content
                while (file_size > 0) {
                    if (file_size > POCKET_SIZE) {
                        // Send a complex pocket
                        fread(data, sizeof(char), POCKET_SIZE, fptr);
                        write(sockfd, data, POCKET_SIZE);
                        // Get ACK
                        read(sockfd, buf, MAXLINE);
                        file_size -= POCKET_SIZE;
                    } else {
                        // Send a remain pocket
                        fread(data, sizeof(char), file_size, fptr);
                        write(sockfd, data, file_size);
                        // Get ACK
                        read(sockfd, buf, MAXLINE);
                        file_size = 0;
                    }
                }
                printf("File download complete.\n");
                // Get ACK
                read(sockfd, buf, MAXLINE);
                fclose(fptr);
            } else {
                // Exit
                exit(0);
            }
        }
    }

    if (n < 0 && errno == EINTR)
        goto again;
    else if (n < 0)
        printf("str_echo: read error");
}
