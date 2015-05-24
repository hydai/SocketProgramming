#include "hw2_lib.h"
#include "const.h"

// Server side
int create_udp_server(struct sockaddr_in addr, int port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    return sockfd;
}

void server_echo(int sockfd, sqlite3* &db) {
    // Set socket timeout: http://stackoverflow.com/questions/13547721/udp-socket-set-timeout
    char msg[MAXLINE+1];
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    std::map<std::string, struct sockaddr_in> online_user;
    online_user.clear();
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = WAIT_TIME_OUT_US;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        logging("setsocketopt failed");
    }
    while (true) {
        int n = recvfrom(sockfd, msg, MAXLINE, 0, (struct sockaddr *)&addr, &len);
        if (n < 0) {
            // Skip garbage data
            continue;
        }
        msg[n] = '\0';
        logging("From ip: "
                + get_ip_info(addr).ip
                + ":"
                + std::to_string(get_ip_info(addr).port)
                + " - \n"
                + std::string(msg));
        // Reply ack
        std::string reply_string = ACK;
        sendto(sockfd, reply_string.c_str(), reply_string.length(), 0, (struct sockaddr *)&addr, len);
        // Two packets in very short time, sometime it will be broken...
        usleep(SLEEP_TIME_US);
        reply_string = run_command_server(addr, db, msg, online_user, sockfd);
        sendto(sockfd, reply_string.c_str(), reply_string.length(), 0, (struct sockaddr *)&addr, len);
    }
}

// Client side
int create_udp_client(struct sockaddr_in *addr, std::string ip, int port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(addr, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &(addr->sin_addr));
    return sockfd;
}

int client_echo(int sockfd, struct sockaddr_in addr) {
    int istreamfd = fileno(stdin);
    char username[128] = "";
    fd_set allset;
    FD_ZERO(&allset);
    show_welcome_message();
    while (true) {
        FD_SET(sockfd, &allset);
        FD_SET(istreamfd, &allset);
        int maxfd = max(sockfd, istreamfd) + 1;
        select(maxfd, &allset, NULL, NULL, 0);
        if (FD_ISSET(sockfd, &allset)) {
            char recv_string[MAXLINE + 1];
            int n = recvfrom(sockfd, recv_string, MAXLINE, 0, NULL, NULL);
            if (n < 0) {
                // Error occurs
                logging("Error: receive from server has something wrong");
            } else if (n == 0) {
                // Server down
                logging("Error: server terminated");
                return 1;
            } else {
                recv_string[n] = '\0';
                logging("Server reply: " + std::string(recv_string));
                std::string command = run_command_client(recv_string, username);
                if (command != "") {
                    send_data_to(sockfd, addr, CLIENT_MODE, command);
                }
            }
        }
        if (FD_ISSET(istreamfd, &allset)) {
            std::string input = read_line();
            logging("Client input: " + input);
            std::string command = run_command_client(input, username);
            if (command != "") {
                send_data_to(sockfd, addr, CLIENT_MODE, command);
            }
        }
    }
    return 0;
}

// Both side
void send_data_to(int sockfd, struct sockaddr_in addr, int mode, std::string data) {
    if (mode == SERVER_MODE) {
        // Server send data to client, without packet loss
        sendto(sockfd, data.c_str(), data.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
        logging("Send data to client succeed: " + data);
    } else if (mode == CLIENT_MODE) {
        // Client send data to server, with packet loss (very evil TA)
        sendto(sockfd, data.c_str(), data.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = WAIT_TIME_OUT_US;
        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            logging("setsocketopt failed");
        }
        char recv_string[MAXLINE + 1];
        while(true) {
            // Get ack
            int n = recvfrom(sockfd, recv_string, MAXLINE, 0, NULL, NULL);
            if(n < 0) {
                // Send data failed, resend again
                sendto(sockfd, data.c_str(), data.length(), 0, (struct sockaddr *)&addr, sizeof(addr));
                logging("Resend data: " + data);
            } else {
                break;
            }
        }
        logging("Send data to server succeed: " + data);
    } else {
        // Unknown mode, do nothing
    }
}

std::string run_command_server(struct sockaddr_in addr, sqlite3* &db, std::string command, std::map<std::string, struct sockaddr_in> &online_user, int sockfd) {
    std::string ret = "";
    string_vector cmds = string_split(command);
    if (cmds.size() < 1) {
        ret = "Empty input";
        return ret;
    }
    if (cmds.at(0) == "R") {
        // Register
        logging("Register account: " + cmds.at(1) + " password: " + cmds.at(2));
        int rc = 0;
        result_set rs;
        {
            std::string sql = "INSERT INTO user VALUES ('" + cmds.at(1) + "', '" + cmds.at(2) + "')";
            rs = exec_sql(db, sql, rc);
        }
        if (rc == SQLITE_OK) {
            ret = "R_R Accepted";
            logging("Register accepted");
        } else {
            ret = "R_R Failed";
            logging("Register failed");
        }
    } else if (cmds.at(0) == "LI") {
        // Login
        logging("Login account: " + cmds.at(1) + " password: " + cmds.at(2));
        int rc = 0;
        result_set rs;
        {
            std::string sql = "SELECT * FROM user WHERE account='" + cmds.at(1) + "' AND password='" + cmds.at(2) + "'";
            rs = exec_sql(db, sql, rc);
        }
        logging("Size: " + std::to_string(rs.size()));
        if (rc == SQLITE_OK && rs.size() >= 1) {
            std::string acc = rs["account"];
            online_user[acc] = addr;
            logging("Login " + acc + " accepted");
            ret = "R_LI Accepted " + acc;
        } else {
            logging("Login failed");
            ret = "R_LI Failed";
        }
    } else if (cmds.at(0) == "LO") {
        // Logout
        logging("Logout account: " + cmds.at(1));
        online_user.erase(cmds.at(1));
        ret = "R_LO Accepted";
    } else if (cmds.at(0) == "D") {
        // Delete
        logging("Delete account: " + cmds.at(1));
        int rc = 0;
        result_set rs;
        {
            std::string sql = "DELETE FROM user WHERE account='" + cmds.at(1) + "'";
            rs = exec_sql(db, sql, rc);
        }
        if (rc == SQLITE_OK) {
            online_user.erase(cmds.at(1));
            logging("Delete " + cmds.at(1) + " accepted");
            ret = "R_D Accepted";
        } else {
            logging("Delete " + cmds.at(1) + "failed");
            ret = "R_D Failed";
        }
    } else if (cmds.at(0) == "SU") {
        // Show users
        logging("Show user");
        ret = "R_SU";
        for (auto iter = online_user.begin(); iter != online_user.end(); iter++) {
            ret = ret + " " + (iter->first);
        }
    } else if (cmds.at(0) == "Y") {
        // Yell
        logging("Yell from " + cmds.at(1));
        ret = "R_Y " + cmds.at(1);
        for (int i = 2; i < cmds.size(); i++) {
            ret = ret + " " + cmds.at(i);
        }
        for (auto iter = online_user.begin(); iter != online_user.end(); iter++) {
            sendto(sockfd, ret.c_str(), ret.length(), 0, (struct sockaddr *)&(iter->second), sizeof(iter->second));
        }
    } else if (cmds.at(0) == "T") {
        // Tell
        logging("Tell from " + cmds.at(1) + " to " + cmds.at(2));
        ret = "R_T " + cmds.at(1) + " " + cmds.at(2);
        for (int i = 3; i < cmds.size(); i++) {
            ret = ret + " " + cmds.at(i);
        }
        sendto(sockfd, ret.c_str(), ret.length(), 0, (struct sockaddr *)&(online_user[cmds.at(2)]), sizeof(online_user[cmds.at(2)]));
    }
        
    return ret;
}

std::string run_command_client(std::string command, char *username) {
    std::string t_username = "";
    std::string ret = "";
    string_vector cmds = string_split(command);
    if (cmds.size() < 1) {
        ret = "Empty input";
        return ret;
    }
    if (cmds.at(0) == "R") {
        // Register
        std::cout << "Account: ";
        t_username = read_line();
        ret = "R " + t_username;
        logging("GET " + std::string(username));
        std::cout << "Password: ";
        ret = ret + " " + read_line();
    } else if (cmds.at(0) == "R_R") {
        // Server reply register
        show_welcome_message();
        std::cout << "Register " + cmds.at(1) << std::endl;
    } else if (cmds.at(0) == "LI") {
        // Login
        std::cout << "Account: ";
        t_username = read_line();
        strcpy(username, t_username.c_str());
        ret = "LI " + t_username;
        std::cout << "Password: ";
        ret = ret + " " + read_line();
    } else if (cmds.at(0) == "R_LI") {
        // Server reply login
        if (cmds.at(1) == "Accepted") {
            show_lobby_message(cmds.at(2));
        } else {
            std::cout << "Login " + cmds.at(1) << std::endl;
        }
    } else if (cmds.at(0) == "LO") {
        // Logout
        ret = "LO " + std::string(username);
    } else if (cmds.at(0) == "R_LO") {
        // Server reply logout
        show_welcome_message();
        std::cout << "Logout\n";
    } else if (cmds.at(0) == "D") {
        // Delete account
        ret = "D " + std::string(username);
    } else if (cmds.at(0) == "R_D") {
        // Server reply delete account
        show_welcome_message();
        if (cmds.at(1) == std::string("Accepted")) {
            std::cout << "Delete Account\n";
        } else {
            std::cout << "Delete Failed\n" << std::endl;
        }
    } else if (cmds.at(0) == "SU") {
        // Show user list
        ret = "SU";
    } else if (cmds.at(0) == "R_SU") {
        // Server reply show user list
        show_online_user(cmds);
    } else if (cmds.at(0) == "Y") {
        // Yell
        ret = "Y " + std::string(username) + " ";
        std::cout << "Say something: ";
        ret = ret + read_line();
    } else if (cmds.at(0) == "R_Y") {
        // Server boardcase
        if (cmds.at(1) != std::string(username))
            show_yell_message(cmds);
    } else if (cmds.at(0) == "T") {
        // Tell
        ret = "T " + std::string(username) + " ";
        std::cout << "Tell someone: ";
        ret = ret + read_line();
        std::cout << "Say something: ";
        ret = ret + " " + read_line();
    } else if (cmds.at(0) == "R_T") {
        // Server someone tell to you
        if (cmds.at(2) == std::string(username))
            show_tell_message(cmds);
    } else {
        logging("Unknown Command");
    }
    return ret;
}
