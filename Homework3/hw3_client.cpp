#include "hw3_lib.h"
#include "const.h"

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
    } else if (cmds.at(0) == "BL") {
        // Back to Article List
        ret = "SA";
    } else if (cmds.at(0) == "SA") {
        // Show Article
        ret = "SA";
    } else if (cmds.at(0) == "R_SA") {
        // Server reply show article
        show_file_list();
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
    } else if (cmds.at(0) == "R_E") {
        // Server reply Enter Article
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
    } else {
        logging("Unknown Command");
    }
    return ret;
}
