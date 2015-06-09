#include "hw3_lib.h"
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
    }
    return ret;
}
