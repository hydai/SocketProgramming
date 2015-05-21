#include "hw2_lib.h"
#include "const.h"

int create_udp_server(struct sockaddr_in addr, int port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(PORT);
    bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    return sockfd;
}

void server_echo(int sockfd) {
    // Set socket timeout: http://stackoverflow.com/questions/13547721/udp-socket-set-timeout
    char msg[MAXLINE+1];
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        logging("setsocketopt failed");
    }
    while (true) {
        int n = recvfrom(sockfd, msg, MAXLINE, 0, (struct sockaddr *)&addr, &len);
        if (n < 0) {
            // Skip garbage data
            continue;
        }
        sendto(sockfd, msg, n, 0, (struct sockaddr *)&addr, len);
    }
}

IP_INFO get_ip_info(struct sockaddr_in addr) {
    IP_INFO ret;
    ret.ip = inet_ntoa(addr.sin_addr);
    ret.port = ntohs(addr.sin_port);
    return ret;
}

void show_welcome_message() {
    system("clear");
    std::cout << "**********Welcome to hyBBS**********\n"
              << "[R]Registe [LI]Login\n";
}

void show_lobby_message(std::string &user_name) {
    system("clear");
    std::cout << "**********Hello, "
              << user_name
              << "**********\n"
              << "[SU]Show user [SA]Show Article [A]Add Article [E]Enter Article\n"
              << "[Y]Yell [T]Tell [LO]Logout\n"
              << std::endl;
}

void show_article_list() {
    system("clear");
    std::cout << "**********Article List**********\n"
              << "[E]Enter Article [D]Delete Article\n"
              << std::endl;
}

void show_article_content() {
    system("clear");
    std::cout << "**********Article**********\n"
              << "[U]Upload [D]Download [D]Delete Article\n"
              << std::endl;
}
void logging(std::string msg) {
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    std::cerr << now->tm_hour << ':'
              << now->tm_min << ':'
              << now->tm_sec
              << msg
              << std::endl;
}

string_vector string_split(std::string s) {
    string_vector sp;
    std::stringstream ss;
    std::string tmp;
    ss << s;
    while (ss >> tmp) {
        sp.push_back(tmp);
    }
    return sp;
}

std::string read_line() {
    std::string ret;
    std::getline(std::cin, ret);
    return ret;
}

bool exec(std::string command) {
    int status = system(command.c_str());
    return (status == 0)?true:false;
}

// sqlite3
// Input: pointer of sqlite3 struct
// Output:
//      True -> Succeed
//      False -> Failed
bool init_db(sqlite3* &db) {
   if (sqlite3_open_v2(
         "hw2_101062124_db.db",
         &db,
         SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
         NULL)) {
       logging(("Can't open db: " + std::string(sqlite3_errmsg(db)) + "\n").c_str());
       close_db(db);
       return false;
   }
   for (int i = 0; i < INIT_SQL_SIZE; i++) {
       exec_sql(db, std::string(init_sql[i]));
   }
   
   return true;
}

void close_db(sqlite3* &db) {
    sqlite3_close(db);
}

result_set exec_sql(sqlite3* &db, std::string sql) {
    char *errMsg = NULL;
    int rc = 0;
    result_set rs;
    rs.clear();
    rc = sqlite3_exec(db, sql.c_str(), callback, (void *)&rs, &errMsg);
    if(rc != SQLITE_OK) {
        logging("SQL error: " + std::string(errMsg) + "\n");
        sqlite3_free(errMsg);
    } else {
        logging("Operation done successfully\n");
    }
    return rs;
}

static int callback(void *vrs, int numOfCol, char **valueOfCol, char **nameOfCol){
    result_set rs = *(result_set *)vrs;
    for (int i = 0; i < numOfCol; i++) {
        rs.insert( std::pair<std::string, std::string>(nameOfCol[i], (valueOfCol[i]?valueOfCol[i]:"NULL")) );
    }
    return 0;
}
