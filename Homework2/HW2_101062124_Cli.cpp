#include "const.h"
#include "hw2_lib.h"

void str_cli(FILE *fp, int sockfd);
void getFileName(char dst[], char src[]);
void show_prompt();

int main (int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;
    if (argc < 3) {
        printf("usage: %s <IPaddress>\n", argv[0]);
        goto END;
    }
    mkdir("Download", 0777);

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
