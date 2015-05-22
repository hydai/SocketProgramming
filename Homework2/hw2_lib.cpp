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

void server_echo(int sockfd) {
    // Set socket timeout: http://stackoverflow.com/questions/13547721/udp-socket-set-timeout
    char msg[MAXLINE+1];
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
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
        reply_string = run_command(msg, SERVER_MODE);
        sendto(sockfd, reply_string.c_str(), reply_string.length(), 0, (struct sockaddr *)&addr, len);
    }
}

// Client side
int create_udp_client(struct sockaddr_in addr, std::string ip, int port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
    return sockfd;
}

int client_echo(int sockfd) {
    int istreamfd = fileno(stdin);
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
                run_command(recv_string, CLIENT_MODE);
            }
        }
        if (FD_ISSET(istreamfd, &allset)) {
            std::string input = read_line();
            logging("Client input: " + input);
            run_command(input, CLIENT_MODE);
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

std::string run_command(std::string command, int mode) {
    std::string ret = "";
    string_vector cmds = string_split(command);
    if (cmds.size() < 1) {
        ret = "Empty input";
        return ret;
    }
    if (mode == SERVER_MODE) {
        
    } else if (mode == CLIENT_MODE) {
        
    } else {
        // Unknown mode, do nothing
    }
    return ret;
}

