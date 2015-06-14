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
    std::map<std::string, std::set<std::string> > online_file;
    online_file.clear();
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
        reply_string = run_command_server(addr, db, msg, online_user, online_file, sockfd);
        if (reply_string.size() > 0)
            sendto(sockfd, reply_string.c_str(), reply_string.length(), 0, (struct sockaddr *)&addr, len);
    }
}

std::string run_command_server(struct sockaddr_in addr, sqlite3* &db, std::string command, std::map<std::string, struct sockaddr_in> &online_user, std::map<std::string, std::set<std::string> > &online_file, int sockfd) {
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
    } else if (cmds.at(0) == "FL") {
        // Client sends file list
        logging("Client Sends file list");
        online_file[cmds.at(2)].insert(cmds.at(1));
        std::string updateFileCmd = "U_SF";
        for (auto iter = online_file.begin(); iter != online_file.end(); iter++) {
            updateFileCmd = updateFileCmd + " " + iter->first;
            updateFileCmd = updateFileCmd + " " + std::to_string((iter->second).size());
            for (auto iiter = (iter->second).begin(); iiter != (iter->second).end(); iiter++) {
                updateFileCmd = updateFileCmd + " " + *iiter;
            }
        }
        std::string updateUserCmd = "U_SU";
        for (auto iter = online_user.begin(); iter != online_user.end(); iter++) {
            updateUserCmd = updateUserCmd + " " + iter->first;
            IP_INFO ip_info = get_ip_info(iter->second);
            updateUserCmd = updateUserCmd + " " + ip_info.ip;
            updateUserCmd = updateUserCmd + " " + std::to_string(ip_info.port);
        }
        for (auto iter = online_user.begin(); iter != online_user.end(); iter++) {
            sendto(sockfd, updateUserCmd.c_str(), updateUserCmd.length(), 0, (struct sockaddr *)&(iter->second), sizeof(iter->second));
            sendto(sockfd, updateFileCmd.c_str(), updateFileCmd.length(), 0, (struct sockaddr *)&(iter->second), sizeof(iter->second));
        }
        ret = "";
    } else if (cmds.at(0) == "SF") {
        // Show files
        logging("Show files");
        ret = "R_SF";
        for (auto iter = online_file.begin(); iter != online_file.end(); iter++) {
            ret = ret + " " + iter->first;
            ret = ret + " " + std::to_string((iter->second).size());
            for (auto iiter = (iter->second).begin(); iiter != (iter->second).end(); iiter++) {
                ret = ret + " " + *iiter;
            }
        }
    } else if (cmds.at(0) == "SU") {
        // Show users
        logging("Show user");
        ret = "R_SU";
        for (auto iter = online_user.begin(); iter != online_user.end(); iter++) {
            ret = ret + " " + iter->first;
            IP_INFO ip_info = get_ip_info(iter->second);
            ret = ret + " " + ip_info.ip;
            ret = ret + " " + std::to_string(ip_info.port);
        }
    } else if (cmds.at(0) == "Y") {
        // Yell
        logging("Yell from " + cmds.at(1));
        ret = "R_Y " + cmds.at(1);
        for (int i = 2; i < cmds.size(); i++) {
            ret = ret + " " + cmds.at(i);
        }
        for (auto iter = online_user.begin(); iter != online_user.end(); iter++) {
            if (iter->first != cmds.at(1))
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
    } else if (cmds.at(0) == "UL") {
        char buf[MAXLINE+1];
        usleep(SLEEP_TIME_US);
        logging("Upload from " + cmds.at(1) + " File: " + cmds.at(2) + " Bytes: " + cmds.at(3));
        FILE *fptr = fopen(("Upload/" + cmds.at(2)).c_str(), "wb");
        int recivedSize = 0, fileSize = atoi(cmds.at(3).c_str());
        if (fptr != NULL) {
            logging("Server reciving file " + cmds.at(2) + " from " + cmds.at(1)
                    + " Bytes: " + cmds.at(3) + " ...");
            socklen_t len = sizeof(online_user[cmds.at(1)]);
            while (recivedSize < fileSize) {
                int n = recvfrom(sockfd, buf, MAXLINE, 0, (struct sockaddr*) &(online_user[cmds.at(1)]), &len);
                if (n < 0)
                    break;
                recivedSize += n;
                n = fwrite(buf, sizeof(char), n, fptr);
            }
            logging("Server reciving file " + cmds.at(2) + " from " + cmds.at(1)
                    + " Bytes: " + cmds.at(3) + " ...Done.");
            fclose(fptr);
        }
        if (recivedSize == fileSize) {
            std::string sql = "INSERT INTO filelist (account, filename, size) VALUES ('" + cmds.at(1) + "', '" + cmds.at(2) + "', '" + cmds.at(3) + "')";
            int rc;
            result_set rs = exec_sql(db, sql, rc);
        } else {
            exec("rm Upload/" + cmds.at(2));
        }
        ret = "R_UL";
        std::string sql = "SELECT * FROM filelist WHERE account='" + cmds.at(1) + "'";
        int rc;
        result_set rs = exec_sql(db, sql, rc);
        for (int i = 0; i < rs.size(); i++) {
            // R_UL uploader filename bytes
            ret = ret + " " + rs[i]["account"] + " " + rs[i]["filename"] + " " + rs[i]["size"];
        }
    } else if (cmds.at(0) == "DL") {
        logging("Download request from " + cmds.at(1) + " File: " + cmds.at(2));
        struct stat fileStat;
        FILE *fptr;
        if (lstat(("Upload/" + cmds.at(2)).c_str(), &fileStat) < 0
            || (fptr = fopen(("Upload/" + cmds.at(2)).c_str(), "rb")) == NULL) {
            logging("Download request from " + cmds.at(1) + " File: " + cmds.at(2) + " Failed due to DNE file.");
            ret = "R_DL 0";
        } else {
            char buf[MAXLINE+1];
            int fileSize = fileStat.st_size, sendSize = 0;
            logging("Server sending file " + cmds.at(2) + " to " + cmds.at(1)
                    + " Bytes: " + std::to_string(fileSize) + " ...");
            sendto(sockfd, std::to_string(fileSize).c_str(), std::to_string(fileSize).length(), 0, (struct sockaddr*)&(online_user[cmds.at(1)]),  sizeof(online_user[cmds.at(1)]));
            usleep(SLEEP_TIME_US);
            while (!feof(fptr)) {
                int n = fread(buf, sizeof(char), sizeof(buf), fptr);
                n = sendto(sockfd, buf, n, 0, (struct sockaddr*)&(online_user[cmds.at(1)]), sizeof(online_user[cmds.at(1)]));
                sendSize += n;
                usleep(SLEEP_TIME_US);
            }
            logging("Server sending file " + cmds.at(2) + " to " + cmds.at(1)
                    + " Bytes: " + std::to_string(fileSize) + " ...Done.");
            fclose(fptr);
        }
    }
    return ret;
}
