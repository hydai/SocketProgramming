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
    char tid[128] = "";
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
                std::string command = run_command_client(recv_string, username, tid);
                if (command != "") {
                    send_data_to(sockfd, addr, CLIENT_MODE, command);
                }
            }
        }
        if (FD_ISSET(istreamfd, &allset)) {
            std::string input = read_line();
            logging("Client input: " + input);
            std::string command = run_command_client(input, username, tid);
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
        if (rc == SQLITE_OK && rs.size() >= 1) {
            std::string acc = rs[0]["account"];
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
    } else if (cmds.at(0) == "A") {
        // Add Article
        logging("Add Article");
        std::string username = cmds.at(1);
        std::string title = cmds.at(2);
        std::string content = cmds.at(3);
        int rc = 0;
        result_set rs;
        {
            std::string sql = "INSERT INTO text (title, content, account, ip, port) VALUES ('"
                             + title + "', '" + content + "', '" + username + "', '"
                             + get_ip_info(addr).ip + "', '" + std::to_string(get_ip_info(addr).port) + "')";
            rs = exec_sql(db, sql, rc);
        }
        if (rc == SQLITE_OK) {
            logging("Add article " + cmds.at(2) + " accepted");
            ret = "R_A Accepted";
        } else {
            logging("Add article " + cmds.at(2) + " failed");
            ret = "R_A Failed";
        }
    } else if (cmds.at(0) == "SA") {
        // Show Article
        logging("Show Article");
        int rc = 0;
        result_set rs;
        {
            std::string sql = "SELECT * FROM text";
            rs = exec_sql(db, sql, rc);
        }
        if (rc == SQLITE_OK) {
            logging("SELECT text accepted");
            ret = "R_SA Accepted";
            ret = ret + " " + std::to_string(rs.size()) + " ";
            for (int i = 0; i < rs.size(); i++) {
                ret = ret + rs[i]["tid"] + " "
                  + rs[i]["title"] + " "
                  + rs[i]["account"] + " "
                  + rs[i]["ip"] + " "
                  + rs[i]["port"] + " "
                  + rs[i]["hit"] + " "
                  + rs[i]["content"] + " ";
            }
        } else {
            logging("SELECT text failed");
            ret = "R_SA Failed 0";
        }
    } else if (cmds.at(0) == "DA") {
        // Delete Article
        logging("Delete Article ID: " + cmds.at(2) + "by " + cmds.at(1));
        int rc = 0;
        result_set rs;
        {
            std::string sql = "SELECT * FROM text WHERE tid='" + cmds.at(2) + "' AND account='" + cmds.at(1) + "'";
            rs = exec_sql(db, sql, rc);
        }
        if (rc == SQLITE_OK && rs.size() > 0) {
            logging("DELETE text accepted");
            std::string sql = "DELETE FROM text WHERE tid='" + cmds.at(2) + "' AND account='" + cmds.at(1) + "'";
            rs = exec_sql(db, sql, rc);
            ret = run_command_server(addr, db, "SA", online_user, sockfd);
        } else {
            logging("DELETE text failed");
            ret = "R_DA";
        }
    } else if (cmds.at(0) == "RR") {
        // Reply Article
        logging("Reply Article ID: " + cmds.at(1) + " by " + cmds.at(2));
        int rc = 0;
        result_set rs, blk;
        {
            std::string sql = "INSERT INTO reply (tid, account, message, ip, port) VALUES ('"
                            + cmds.at(1) + "', '"
                            + cmds.at(2) + "', '"
                            + cmds.at(3) + "', '"
                            + get_ip_info(addr).ip + "', '"
                            + std::to_string(get_ip_info(addr).port) + "')";
            rs = exec_sql(db, sql, rc);
        }
        ret = run_command_server(addr, db, "E " + cmds.at(2) + " " + cmds.at(1), online_user, sockfd);
    } else if (cmds.at(0) == "E") {
        // Enter Article
        logging("Enter Article ID: " + cmds.at(2) + " by " + cmds.at(1));
        ret = "R_E";
        int rc = 0;
        result_set rs, blk;
        {
            std::string sql = "SELECT * FROM text WHERE tid='" + cmds.at(2) + "'";
            rs = exec_sql(db, sql, rc);
            sql = "UPDATE text SET hit='" + std::to_string(atoi(rs[0]["hit"].c_str()) + 1) + "' WHERE tid='" + cmds.at(2) + "'";
            exec_sql(db, sql, rc);
            sql = "SELECT * FROM text WHERE tid='" + cmds.at(2) + "'";
            rs = exec_sql(db, sql, rc);
        }
        std::string sql = "SELECT * from blacklist WHERE tid='" + cmds.at(2) + "'";
        blk = exec_sql(db, sql, rc);
        if (rs[0]["account"] == cmds.at(1)) {
            // Author
            ret = ret + " " + std::to_string(blk.size());
            logging("Blacklist size: " + std::to_string(blk.size()));
            for (int i = 0; i < blk.size(); i++) {
                ret = ret + " " + blk[i]["blackacc"];
            }
        } else {
            ret = ret + " -1";
        }
        bool isBlack = false;
        for (int i = 0; i < blk.size(); i++) {
            if (cmds.at(1) == blk[i]["blackacc"]) {
                isBlack = true;
                break;
            }
        }
        if (isBlack) {
            ret = ret + " 0 0";
        } else {
            ret = ret + " 1";
            ret = ret + " " + rs[0]["title"];
            ret = ret + " " + rs[0]["account"];
            ret = ret + " " + rs[0]["ip"];
            ret = ret + " " + rs[0]["port"];
            ret = ret + " " + rs[0]["hit"];
            ret = ret + " " + rs[0]["content"];
            // Reply
            sql = "SELECT * FROM reply WHERE tid='" + cmds.at(2) + "'";
            rs = exec_sql(db, sql, rc);
            ret = ret + " " + std::to_string(rs.size());
            int cs = rs.size();
            for (int i = 0; i < cs; i++) {
                ret = ret + " " + rs[i]["account"];
                ret = ret + " " + rs[i]["ip"];
                ret = ret + " " + rs[i]["port"];
                ret = ret + " " + rs[i]["message"];
            }
        }

    } else if (cmds.at(0) == "AB") {
        logging("AB from " + cmds.at(1) + " in tid " + cmds.at(2) + " bk " + cmds.at(3));
        int rc = 0;
        result_set rs;
        {
            std::string sql = "SELECT * FROM text WHERE tid='" + cmds.at(2) + "'";
            rs = exec_sql(db, sql, rc);
        }
        if (cmds.at(1) == rs[0]["account"]) {
            // Author
            std::string sql = "INSERT INTO blacklist (tid, blackacc) VALUES ('" + cmds.at(2) + "', '" + cmds.at(3) + "')";
            rs = exec_sql(db, sql, rc);
        }
        ret = run_command_server(addr, db, "E " + cmds.at(1) + " " + cmds.at(2), online_user, sockfd);
    } else if (cmds.at(0) == "DB") {
        logging("DB from " + cmds.at(1) + " in tid " + cmds.at(2) + " bk " + cmds.at(3));
        int rc = 0;
        result_set rs;
        {
            std::string sql = "SELECT * FROM text WHERE tid='" + cmds.at(2) + "'";
            rs = exec_sql(db, sql, rc);
        }
        if (cmds.at(1) == rs[0]["account"]) {
            // Author
            std::string sql = "DELETE FROM blacklist WHERE tid='" + cmds.at(2) + "' AND blackacc='" + cmds.at(3) + "'";
            logging("DB sql: " + sql);
            rs = exec_sql(db, sql, rc);
        }
        ret = run_command_server(addr, db, "E " + cmds.at(1) + " " + cmds.at(2), online_user, sockfd);
    }
    return ret;
}

std::string run_command_client(std::string command, char *username, char *tid) {
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
    } else if (cmds.at(0) == "A") {
        // Add Article
        ret = "A " + std::string(username) + " ";
        std::cout << "Title: ";
        ret = ret + read_line() + " ";
        std::cout << "Content: ";
        ret = ret + read_line();
    } else if (cmds.at(0) == "R_A") {
        // Server reply add article
        ret = "SA";
    } else if (cmds.at(0) == "DA") {
        // Delete Article
        ret = "DA " + std::string(username) + " ";
        std::cout << "Kill by id: ";
        ret = ret + read_line();
    } else if (cmds.at(0) == "R_DA") {
        // Server reply Delete Article
        std::cout << "Kill failed(permission deny)" << std::endl;
        std::cout << "[BL]Back to Article List" << std::endl;
        ret = "";
    } else if (cmds.at(0) == "BL") {
        // Back to Article List
        ret = "SA";
    } else if (cmds.at(0) == "SA") {
        // Show Article
        ret = "SA";
    } else if (cmds.at(0) == "R_SA") {
        // Server reply show article
        show_article_list();
        if (cmds.at(1) == "Accepted") {
            {
                std::stringstream ss;
                ss << cmds.at(2);
                int cs; ss >> cs;
                for (int i = 0; i < cs; i++) {
                    int j = i*7;
                    std::cout << "ID: " << cmds.at(3+j) << " | "
                              << "Title: " << cmds.at(3+j+1) << " | "
                              << "Author: " << cmds.at(3+j+2) << " | "
                              << "Ip: " << cmds.at(3+j+3) << ":" << cmds.at(3+j+4) << " | "
                              << "Hit: " << cmds.at(3+j+5) << " | "
                              << "Content: " << cmds.at(3+j+6) << std::endl;
                }
            }
        } else {
            std::cout << "Load Article Failed" << std::endl;
        }
    } else if (cmds.at(0) == "B") {
        show_lobby_message(std::string(username));
        ret = "";
    } else if (cmds.at(0) == "E") {
        // Enter Article
        ret = "E " + std::string(username) + " ";
        std::cout << "Article id: ";
        std::string ttid = read_line();
        strcpy(tid, ttid.c_str());
        ret = ret + ttid;
    } else if (cmds.at(0) == "RR") {
        // Reply Article
        ret = "RR " + std::string(tid) + " ";
        ret = ret + std::string(username) + " ";
        std::cout << "Reply something: ";
        ret = ret + read_line();
    } else if (cmds.at(0) == "R_E") {
        // Server reply Enter Article
        show_article_content();
        int cmdct = 1;
        if (cmds.at(cmdct++) != "-1") {
            // Author
            std::cout << "[AB]Add Blacklist [DB]Delete Blacklist" << std::endl;
            std::cout << "Blacklist: " << std::endl;
            cmdct--;
            int cs = atoi(cmds.at(cmdct++).c_str());
            for (int i = 0; i < cs; i++) {
                std::cout << cmds.at(cmdct++) << " ";
            }
            std::cout << std::endl;
        }
        if (cmds.at(cmdct++) == "0") {
            std::cout << "Permission deny" << std::endl;
        } else {
            std::cout
              << "Title: " << cmds.at(cmdct) << std::endl
              << "Author: " << cmds.at(cmdct+1) << std::endl
              << "Ip: " << cmds.at(cmdct+2) << ":" << cmds.at(cmdct+3) << std::endl
              << "Hit: " << cmds.at(cmdct+4) << std::endl
              << "Content: " << cmds.at(cmdct+5) << std::endl;
            cmdct += 6;
        }
        int cs = atoi(cmds.at(cmdct++).c_str());
        std::cout << "==============Reply==============" << std::endl;
        for (int i = 0; i < cs; i++) {
            std::cout
              << "Account: " << cmds.at(cmdct) << "("
              << "Ip: " << cmds.at(cmdct+1) << ":" << cmds.at(cmdct+2) << " => "
              << "Message: " << cmds.at(cmdct+3) << std::endl;
            cmdct += 4;
        }
    } else if (cmds.at(0) == "AB") {
        ret = "AB " + std::string(username) + " " + std::string(tid) + " ";
        std::cout << "Black someone: ";
        ret = ret + read_line();
    } else if (cmds.at(0) == "DB") {
        ret = "DB " + std::string(username) + " " + std::string(tid) + " ";
        std::cout << "De-Black someone: ";
        ret = ret + read_line();
    } else {
        logging("Unknown Command");
    }
    return ret;
}
