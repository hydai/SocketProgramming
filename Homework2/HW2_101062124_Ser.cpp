#include "const.h"
#include "hw2_lib.h"

int main(int argc, char **argv) {
    sqlite3 *db;
    struct sockaddr_in seraddr, cliaddr;

    if (argc < 2) {
        logging("Usage: ./HW2_101062124_Ser [port]");
        return 1;
    }
    int sockfd = create_udp_server(seraddr, std::atoi(argv[1]));
    exec("mkdir Upload");
    
    bool db_status = init_db(db);
    if (!db_status) {
        logging("Fatal error in init_db");
        return 1;
    }
    server_echo(sockfd);
    close_db(db);
    return 0;
}
