#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include "const.h"

void str_cli(FILE *fp, int sockfd);
void getFileName(char dst[], char src[]);
void show_prompt();

int main (int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;
    if (argc != 2) {
        printf("usage: %s <IPaddress>\n", argv[0]);
        goto END;
    }
    mkdir("Download", 0777);
    if ((sockfd=socket(AF_INET, SOCK_STREAM,0)) < 0) {
        printf("socket error\n");
        goto END;
    }

    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(PORT);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        printf("inet_ption error for %s\n", argv[1]);
        goto END;
    }
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        printf("connect error\n");
        goto END;
    }
    str_cli(stdin, sockfd);
END:
    exit(0);
}

void str_cli(FILE *fp, int sockfd) {
    char sendline[MAXLINE+1];
    char recvline[MAXLINE+1];
    char data[POCKET_SIZE];
    char file_name[MAXLINE];
    FILE *fptr = NULL;
    int n, file_size;
    
    show_prompt();
    while (fgets(sendline, MAXLINE, fp) != NULL) {
        sendline[strlen(sendline)-1] = '\0'; // remove '\n'
        if (!strncmp(sendline, CD, strlen(CD))) {
            // Send CD signal
            write(sockfd, CD, sizeof(CD));
            // Get ACK
            read(sockfd, recvline, MAXLINE);
            // Send path
            sendline[strlen(sendline)] = '\0';
            getFileName(file_name, sendline);
            write(sockfd, file_name, strlen(file_name)+1);
            // Get ACK
            n = read(sockfd, recvline, MAXLINE);
            if (n == 0) {
                printf("Recive Error!!!\n");
                return ;
            }
            recvline[n] = '\0';
            printf("Change directory to %s\n", recvline);
            // Send ACK
            write(sockfd, ACK, strlen(ACK)+1);
        } else if (!strncmp(sendline, LS, strlen(LS))) {
            write(sockfd, LS, sizeof(LS));
            n = read(sockfd, recvline, MAXLINE);
            if (n == 0) {
                printf("Recive Error!!!\n");
                return ;
            }
            recvline[n] = '\0';

            printf("Files:\n- - - - - - - - - -\n%s\n", recvline);
            // Send ACK
            write(sockfd, ACK, strlen(ACK)+1);
        } else if (!strncmp(sendline, UPLOAD, strlen(UPLOAD))) {
            // send upload signal
            write(sockfd, UPLOAD, sizeof(UPLOAD));
            // Get ACK
            read(sockfd, recvline, MAXLINE);

            // Send file name
            sendline[strlen(sendline)] = '\0';
            getFileName(file_name, sendline);
            write(sockfd, file_name, strlen(file_name)+1);
            // Get ACK
            read(sockfd, recvline, MAXLINE);
            
            // Send file size
            fptr = fopen(file_name, "rb");
            if (!fptr) {
                printf("Cannot find the file: %s\n", file_name);
                file_size = 0;
                write(sockfd, &file_size, sizeof(file_size));
                continue;
            }
            fseek(fptr, 0, SEEK_END); // Go to end
            file_size = ftell(fptr);
            rewind(fptr);
            write(sockfd, &file_size, sizeof(file_size));
            // Get ACK
            read(sockfd, recvline, MAXLINE);
            
            // Send file content
            while (file_size > 0) {
                if (file_size > POCKET_SIZE) {
                    // Send a complex pocket
                    fread(data, sizeof(char), POCKET_SIZE, fptr);
                    write(sockfd, data, POCKET_SIZE);
                    // Get ACK
                    read(sockfd, recvline, MAXLINE);
                    file_size -= POCKET_SIZE;
                } else {
                    // Send a remain pocket
                    fread(data, sizeof(char), file_size, fptr);
                    write(sockfd, data, POCKET_SIZE);
                    // Get ACK
                    read(sockfd, recvline, MAXLINE);
                    file_size = 0;
                }
            }
            printf("File upload complete.\n");
            // Send ACK
            write(sockfd, ACK, strlen(ACK)+1);
            fclose(fptr);
        } else if (!strncmp(sendline, DOWNLOAD, strlen(DOWNLOAD))) {
            // send download signal
            write(sockfd, DOWNLOAD, sizeof(DOWNLOAD));
            // Get ACK
            read(sockfd, recvline, MAXLINE);

            // Send file name
            sendline[strlen(sendline)] = '\0';
            getFileName(file_name, sendline);
            write(sockfd, file_name, strlen(file_name));
            // Get file size
            read(sockfd, &file_size, sizeof(file_size));
            if (file_size == 0) {
                printf("Error: %s is not avaliable\n", file_name);
                // Send ACK
                write(sockfd, ACK, strlen(ACK)+1);
                continue;
            }

            // Create file in local
            char buf_name[2*MAXLINE];
            strcpy(buf_name, "Download/");
            strcat(buf_name, file_name);
            fptr = fopen(buf_name, "wb");
            if (!fptr) {
                printf("Error: %s cannot be created\n", buf_name);
                // Send ACK
                write(sockfd, ACK, strlen(ACK)+1);
                continue;
            }
            
            // Get file content
            while (file_size > 0) {
                if (file_size > POCKET_SIZE) {
                    // Get a complex pocket
                    read(sockfd, data, POCKET_SIZE);
                    fwrite(data, sizeof(char), POCKET_SIZE, fptr);
                    // Send ACK
                    write(sockfd, ACK, strlen(ACK)+1);
                    file_size -= POCKET_SIZE;
                } else {
                    // Get a complex pocket
                    read(sockfd, data, file_size);
                    fwrite(data, sizeof(char), file_size, fptr);
                    // Send ACK
                    write(sockfd, ACK, strlen(ACK)+1);
                    file_size = 0;
                }
            }
            printf("File download complete.\n");
            // send ACK
            write(sockfd, ACK, strlen(ACK)+1);
            fclose(fptr);
        } else {
            // Exit
            return ;
        }
        show_prompt();
    }
}

void getFileName(char dst[], char src[]) {
    int flag = 0;
    int i = 0;
    while (src[i] != '\0') {
        if ((src[i] != ' ') && (flag == 0)) {
            i++;
        } else if (src[i] == ' ') {
            i++;
            flag = 1;
        } else {
            strcpy(dst, src+i);
            break;
        }
    }
}

void show_prompt() {
    printf("==========================================\n");
    printf("Support command:\n");
    printf("ls\n");
    printf("cd <path>\n");
    printf("put <file name>\n");
    printf("get <file name>\n");
    printf("exit\n");
    printf("$ ");
}
