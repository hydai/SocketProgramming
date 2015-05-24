#include "hw2_lib.h"
#include "const.h"

// Message part
void show_welcome_message() {
    system("clear");
    std::cout << "**********Welcome to hyBBS**********\n"
              << "[R]Registe [LI]Login\n";
}

void show_lobby_message(std::string &username) {
    system("clear");
    std::cout << "**********Hello, "
              << username
              << "**********\n"
              << "[SU]Show user [SA]Show Article [A]Add Article\n"
              << "[Y]Yell [T]Tell [LO]Logout [DA]Delete Account\n"
              << std::endl;
}

void show_article_list() {
    system("clear");
    std::cout << "**********Article List**********\n"
              << "[E]Enter Article [D]Delete Article [B]Back\n"
              << std::endl;
}

void show_article_content() {
    system("clear");
    std::cout << "**********Article**********\n"
              << "[UL]Upload [DL]Download [DD]Delete [BL]Back to List\n"
              << std::endl;
}

void show_online_user(string_vector &sl) {
    system("clear");
    std::cout << "**********Online User**********\n";
    auto iter = sl.begin(); iter++;
    for (; iter != sl.end(); iter++) {
        std::cout << *iter << std::endl;
    }
    std::cout << "*******************************\n";
}

void show_yell_message(string_vector &msg) {
    std::cout << "\n**********"
              << msg.at(1)
              << " Yell**********\n";
    auto iter = msg.begin(); iter++; iter++;
    for (; iter != msg.end(); iter++) {
        std::cout << *iter << " ";
    }
    std::cout << std::endl;
    std::cout << "*******************************\n\n";
}

void show_tell_message(string_vector &msg) {
    std::cout << "\n**********"
              << msg.at(1)
              << " Talk to You("
              << msg.at(2)
              << ")**********\n";
    auto iter = msg.begin(); iter++; iter++; iter++;
    for (; iter != msg.end(); iter++) {
        std::cout << *iter << " ";
    }
    std::cout << std::endl;
    std::cout << "*******************************\n\n";
}
