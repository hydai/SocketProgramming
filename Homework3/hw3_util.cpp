#include "hw3_lib.h"
#include "const.h"

// Util
void logging(std::string msg) {
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    std::cerr << now->tm_hour << ':'
              << now->tm_min << ':'
              << now->tm_sec << " "
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

IP_INFO get_ip_info(struct sockaddr_in addr) {
    IP_INFO ret;
    ret.ip = inet_ntoa(addr.sin_addr);
    ret.port = ntohs(addr.sin_port);
    return ret;
}
