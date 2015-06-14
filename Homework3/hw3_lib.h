#ifndef HW3_LIB_H
#define HW3_LIB_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <utility>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>
#include "const.h"

// Type
typedef std::vector< std::map<std::string, std::string> > result_set;
typedef std::vector<std::string> string_vector;
struct IP_INFO {
    std::string ip;
    int port;
};
struct FILE_STRUCT {
    int sub_no;
    int total_no;
    std::string filename;
    IP_INFO ip_info;
};
struct th_renew {
    std::string username;
    IP_INFO ip_info;
};

// Execute command
bool exec(std::string command);
std::string run_command_server(struct sockaddr_in addr, sqlite3* &db, std::string command, std::map<std::string, struct sockaddr_in> &online_user, std::map<std::string, std::set<std::string> > &online_file, int sockfd);
std::string run_command_client(std::string command, char *username, char *tid, IP_INFO ip_info, std::map<std::string, IP_INFO> &userlist, std::map<std::string, std::set<std::string> > &filelist);

// Socket
IP_INFO get_ip_info(struct sockaddr_in addr);
int create_udp_server(struct sockaddr_in addr, int port);
int create_udp_client(struct sockaddr_in *addr, std::string ip, int port);
void server_echo(int sockfd, sqlite3* &db);
int client_echo(int sockfd, struct sockaddr_in addr);
void send_data_to(int sockfd, struct sockaddr_in addr, int mode, std::string data);
int connectByAddr(struct sockaddr_in &addr, const char *ip, int port);
int listenByAddr(struct sockaddr_in &addr, int port);
void sendDataByIpInfo(IP_INFO ip_info, std::string data);

// Message
void show_welcome_message();
void show_lobby_message(std::string username);
void show_file_list();
void show_online_user(string_vector &ul);
void show_online_file(string_vector &ul);
void show_yell_message(string_vector &msg);
void show_tell_message(string_vector &msg);

// String
void logging(std::string msg);
std::string read_line();
string_vector string_split(std::string s);

// Sqlite
bool init_db(sqlite3* &db);
result_set exec_sql(sqlite3* &db, std::string sql, int &rc);
static int callback(void *vrs, int numOfCol, char **valueOfCol, char **nameOfCol);
void close_db(sqlite3* &db);

// Util
template<typename T>
T max(T a, T b) {
    return (a>b)?a:b;
}

#endif
