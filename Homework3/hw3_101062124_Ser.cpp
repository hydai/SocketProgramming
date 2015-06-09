#include "const.h"
#include "hw3_lib.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        logging("Usage: " + std::string(argv[0]) + " <port>");
        return 1;
    }
    struct sockaddr_in addr;
    int sockfd = create_udp_server(addr, std::atoi(argv[1]));
    mkdir("Upload", 0777);
    
    sqlite3 *db;
    bool db_status = init_db(db);
    if (!db_status) {
        logging("Fatal error in init_db");
        return 1;
    }
    server_echo(sockfd, db);
    close_db(db);
    return 0;
}
