#include "hw3_lib.h"
#include "const.h"

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
