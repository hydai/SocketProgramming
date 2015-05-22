#include "const.h"
#include "hw2_lib.h"

void str_cli(FILE *fp, int sockfd);
void getFileName(char dst[], char src[]);
void show_prompt();

int main (int argc, char **argv) {
    if (argc < 3) {
        printf("usage: %s <IPaddress> <Port>\n", argv[0]);
        goto END;
    }
    struct sockaddr_in addr;
    int sockfd = create_udp_client(addr, argv[1], std::atoi(argv[2]));
    mkdir("Download", 0777);
    int status = client_echo(sockfd);
    return status;
}
