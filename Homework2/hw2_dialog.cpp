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
              << "[E]Enter Article [D]Delete Article\n"
              << std::endl;
}

void show_article_content() {
    system("clear");
    std::cout << "**********Article**********\n"
              << "[U]Upload [D]Download [D]Delete Article\n"
              << std::endl;
}

void show_online_user(string_vector &sl) {
    std::cout << "**********Online User**********\n";
    auto iter = sl.begin(); iter++;
    for (; iter != sl.end(); iter++) {
        std::cout << *iter << std::endl;
    }
    std::cout << "*******************************\n";
}
