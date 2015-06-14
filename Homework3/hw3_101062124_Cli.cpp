#include "const.h"
#include "hw3_lib.h"

int main (int argc, char **argv) {
    if (argc < 3) {
        logging("usage: " + std::string(argv[0]) + " <IPaddress> <Port>");
        return 1;
    }
    struct sockaddr_in addr;
    int sockfd = create_udp_client(&addr, argv[1], std::atoi(argv[2]));
    mkdir("Download", 0777);
    mkdir("Upload", 0777);
    int status = client_echo(sockfd, addr);
    return status;
}
