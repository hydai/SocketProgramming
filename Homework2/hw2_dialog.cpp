#include "hw2_lib.h"
#include "const.h"

// Message part
void show_welcome_message() {
    system("clear");
    std::cout << "**********Welcome to hyBBS**********\n"
              << "[R]Registe [LI]Login\n";
}

void show_lobby_message(std::string &user_name) {
    system("clear");
    std::cout << "**********Hello, "
              << user_name
              << "**********\n"
              << "[SU]Show user [SA]Show Article [A]Add Article [E]Enter Article\n"
              << "[Y]Yell [T]Tell [LO]Logout\n"
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
