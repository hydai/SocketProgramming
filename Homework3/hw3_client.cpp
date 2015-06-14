#include "hw3_lib.h"
#include "const.h"

// Client side



void send_file(FILE_STRUCT fs) {
    logging("Send file");
    struct sockaddr_in addr, cliaddr;
    char buf[MAXLINE+1] = {0};
    FILE *fptr;
    fptr = fopen(("Upload/" + fs.filename).c_str(), "rb");
    long long fileSize = 0;
    {
        struct stat fileStat;
        lstat(("Upload/" + fs.filename).c_str(), &fileStat);
        fileSize = fileStat.st_size;
    }
    fs.ip_info.port += fs.sub_no+2;
    int listenfd = listenByAddr(addr, fs.ip_info.port);
    socklen_t len = sizeof(cliaddr);
    int connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &len);
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(connfd, SOL_SOCKET, SO_REUSEADDR, &tv, sizeof(tv));

    long long fileOffset = 0;
    if (fs.total_no > 1) {
        fileOffset = (fileSize * (fs.sub_no-1)) / fs.total_no;
        fileSize = (fileSize * fs.sub_no) / fs.total_no - fileOffset;
    }
    fseek(fptr , fileOffset , SEEK_SET);

    sprintf(buf, "%lld", fileSize);
    logging("Ready to send bytes:" + std::to_string(fileSize));
    write(connfd, buf, strlen(buf));
    read(connfd, buf, MAXLINE);

    logging("Upload " + fs.filename + " started.");
    while (fileSize > 0) {
        int n = std::min((long long int)MAXLINE, fileSize);
        fread(buf, sizeof(char), n, fptr);
        fileOffset += n;
        fileSize -= n;
        n = write(connfd, buf, n);
        memset(buf, 0, sizeof(buf));
        usleep(SLEEP_TIME_US);
    }

    close(connfd);
    close(listenfd);
    logging("Upload " + fs.filename + " finished.");
}

void get_file(FILE_STRUCT fs) {
    logging("Download file: " + fs.filename);
    usleep(250000);
    char buf[MAXLINE+1] = {0};
    struct sockaddr_in addr;
    FILE *fptr;
    fptr = fopen(("Download/" + fs.filename).c_str(), "wb");
    fs.ip_info.port += fs.sub_no + 2;
    int connfd = connectByAddr(addr, fs.ip_info.ip.c_str(), fs.ip_info.port);
    read(connfd, buf, MAXLINE);
    write(connfd, buf, strlen(buf));
    long long fileSize = atol(buf);
    logging("Download " + fs.filename + " started.");

    while (fileSize > 0) {
        int n = read(connfd, buf, sizeof(buf));
        fwrite(buf, sizeof(char), n, fptr);
        memset(buf, 0, sizeof(buf));
        fileSize -= n;
    }

    fclose(fptr);
    close(connfd);
    logging("Download " + fs.filename + " finished.");
}

void auto_renew_filelist(std::string username, IP_INFO server_ip) {
    DIR *dir;
    struct dirent *entry;
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(FILE_RENEW_TIME));
        dir = opendir("Upload/");
        while ((entry = readdir(dir)) != NULL) {
            std::string filename(entry->d_name);
            logging("Get " + filename);
            std::string sendstr = "FL " + username + " " + filename;
            sendDataByIpInfo(server_ip, sendstr);
        }
        closedir(dir);
    }
}

void sendDataByIpInfo(IP_INFO ip_info, std::string data) {
    struct sockaddr_in addr;
    int fd = create_udp_client(&addr, ip_info.ip, ip_info.port);
    sendto(fd, data.c_str(), data.length(), 0, (struct sockaddr*)&addr, sizeof(addr));
    close(fd);
}

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
    IP_INFO ip_info = get_ip_info(addr);
    std::map<std::string, IP_INFO> userlist;
    userlist.clear();
    std::map<std::string, std::set<std::string> > filelist;
    filelist.clear();
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
                std::string command = run_command_client(recv_string, username, tid, ip_info, userlist, filelist);
                if (command != "") {
                    send_data_to(sockfd, addr, CLIENT_MODE, command);
                }
            }
        }
        if (FD_ISSET(istreamfd, &allset)) {
            std::string input = read_line();
            logging("Client input: " + input);
            std::string command = run_command_client(input, username, tid, ip_info, userlist, filelist);
            if (command != "") {
                send_data_to(sockfd, addr, CLIENT_MODE, command);
            }
        }
    }
    return 0;
}

std::string run_command_client(std::string command, char *username, char *tid, IP_INFO ip_info, std::map<std::string, IP_INFO> userlist, std::map<std::string, std::set<std::string> > filelist) {
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
            std::thread renew_fl_thread(auto_renew_filelist, username, ip_info);
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
    } else if (cmds.at(0) == "SF") {
        // Show file list
        ret = "SF";
    } else if (cmds.at(0) == "R_SF") {
        // Server reply show file list
        show_online_file(cmds);
        filelist.clear();
        std::string fn;
        int i = 1;
        fn = cmds.at(i); i++;
        for (; i < cmds.size(); ) {
            int fno = atoi(cmds.at(i).c_str());
            i++;
            for (; fno > 0 ; fno--) {
                filelist[fn].insert(cmds.at(i)); i++;
            }
            if (i < cmds.size()) {
                fn = cmds.at(i);
                i++;
            }
        }
    } else if (cmds.at(0) == "U_SF") {
        // Server reply show file list, Update
        filelist.clear();
        std::string fn;
        int i = 1;
        fn = cmds.at(i); i++;
        for (; i < cmds.size(); ) {
            int fno = atoi(cmds.at(i).c_str());
            i++;
            for (; fno > 0 ; fno--) {
                filelist[fn].insert(cmds.at(i)); i++;
            }
            if (i < cmds.size()) {
                fn = cmds.at(i);
                i++;
            }
        }
    } else if (cmds.at(0) == "SU") {
        // Show user list
        ret = "SU";
    } else if (cmds.at(0) == "U_SU") {
        // Server reply show user list, Update
        userlist.clear();
        IP_INFO tmp_ip;
        for (int i = 1; i < cmds.size(); i += 3) {
            tmp_ip.ip = cmds.at(i+1);
            tmp_ip.port = atoi(cmds.at(i+2).c_str());
            userlist[cmds.at(i)] = tmp_ip;
        }
    } else if (cmds.at(0) == "R_SU") {
        // Server reply show user list
        show_online_user(cmds);
        userlist.clear();
        IP_INFO tmp_ip;
        for (int i = 1; i < cmds.size(); i += 3) {
            tmp_ip.ip = cmds.at(i+1);
            tmp_ip.port = atoi(cmds.at(i+2).c_str());
            userlist[cmds.at(i)] = tmp_ip;
        }
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
        ret = "R_T " + std::string(username) + " ";
        std::cout << "Tell someone: ";
        std::string to_user = read_line();
        ret = ret + to_user;
        std::cout << "Say something: ";
        ret = ret + " " + read_line();
        if (userlist.find(to_user) != userlist.end()) {
            sendDataByIpInfo(userlist[to_user], ret);
        }
        ret = "";
    } else if (cmds.at(0) == "R_T") {
        // Server someone tell to you
        show_tell_message(cmds);
    } else if (cmds.at(0) == "MDL") {
        std::vector<std::thread> threads;
        std::vector<FILE_STRUCT> fss;
        std::string filename = cmds.at(1);
        int total_no = 1;
        std::vector<std::string> ownerList;
        for (auto &owner : filelist[filename]) {
            if (owner != username)
                ownerList.push_back(owner);
        }
        if (ownerList.size() < 1) {
            logging("Error: No such file");
            return "";
        }
        total_no = ownerList.size();
        for (int i = 1; i <= total_no; i++) {
            ret = "S_D " + std::string(username) + " " + filename;
            ret = ret + " " + std::to_string(i);
            ret = ret + " " + std::to_string(total_no);
            sendDataByIpInfo(userlist[ownerList.at(i-1)], ret);
            usleep(50000);
            logging("Download " + filename + " "
                    + std::to_string(i) + "/"
                    + std::to_string(total_no)
                    + " from " + ownerList.at(i-1));
            FILE_STRUCT fs;
            fs.filename = filename + "." + std::to_string(i);
            fs.sub_no = i;
            fs.total_no = total_no;
            fs.ip_info.ip = ip_info.ip;
            fs.ip_info.port = 37899;
            threads.push_back(std::thread(get_file, fs));
        }
        ret = "";
        FILE *fptr = fopen(("Download/" + filename).c_str(), "wb");
        char buf[MAXLINE+1];
        for (int i = 1; i <= total_no; i++) {
            std::string tmp_filename = "Download/" + filename + "." + std::to_string(i);
            threads.at(i-1).join();
            FILE *sub_fptr = fopen(tmp_filename.c_str(), "rb");
            while (!feof(sub_fptr)) {
                int n = fread(buf, sizeof(char), MAXLINE, sub_fptr);
                fwrite(buf, sizeof(char), n, fptr);
            }
            fclose(sub_fptr);
            exec("rm " + filename);
        }
        fclose(fptr);
    } else if (cmds.at(0) == "DL") {
        // Ask file from others
        ret = "S_D " + std::string(username) + " " + cmds.at(1);
        if (userlist.find(cmds.at(2)) != userlist.end()) {
            sendDataByIpInfo(userlist[cmds.at(2)], ret);
            FILE_STRUCT fs;
            fs.ip_info.ip = userlist[cmds.at(2)].ip;
            fs.ip_info.port = 37899;
            fs.filename = cmds.at(1);
            std::thread dl_thread(get_file, fs);
        }
        ret = "";
    } else if (cmds.at(0) == "S_D") {
        // Send file to others
        logging("Send file to others");
        if (userlist.find(cmds.at(1)) != userlist.end()) {
            FILE_STRUCT fs;
            fs.ip_info.ip = userlist[cmds.at(1)].ip;
            fs.ip_info.port = 37899;
            fs.sub_no = 0;
            fs.total_no = 0;
            if (cmds.size() >= 4) {
                fs.sub_no = atoi(cmds.at(3).c_str());
                fs.total_no = atoi(cmds.at(4).c_str());
            }
            fs.filename = cmds.at(2);
            std::thread s_d_thread(send_file, fs);
        }
    } else if (cmds.at(0) == "B") {
        show_lobby_message(std::string(username));
        ret = "";
    } else {
        logging("Unknown Command");
    }
    return ret;
}
