#include <iostream>
#include <cstdlib>
#include "hw2_lib.h"

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

void logging(std::string msg) {
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );
    std::cerr << now->tm_hour << ':'
              << now->tm_min << ':'
              << now->tm_sec
              << msg
              << std::endl;
}

// sqlite3
// Input: pointer of sqlite3 struct
// Output:
//      True -> Succeed
//      False -> Failed
bool init_db(sqlite3* &db) {
   if (sqlite3_open_v2(
         "hw2_101062124_db.db",
         &db,
         SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
         NULL)) {
       logging(("Can't open db: " + std::string(sqlite3_errmsg(db)) + "\n").c_str());
       close_db(db);
       return false;
   }
   for (int i = 0; i < INIT_SQL_SIZE; i++) {
       exec_sql(db, std::string(init_sql[i]));
   }
   
   return true;
}

void close_db(sqlite3* &db) {
    sqlite3_close(db);
}

resultSet exec_sql(sqlite3* &db, std::string sql) {
    char *errMsg = NULL;
    int rc = 0;
    resultSet rs;
    rs.clear();
    rc = sqlite3_exec(db, sql.c_str(), callback, (void *)&rs, &errMsg);
    if(rc != SQLITE_OK) {
        logging("SQL error: " + std::string(errMsg) + "\n");
        sqlite3_free(errMsg);
    } else {
        logging("Operation done successfully\n");
    }
    return rs;
}

static int callback(void *vrs, int numOfCol, char **valueOfCol, char **nameOfCol){
    resultSet rs = *(resultSet *)vrs;
    for (int i = 0; i < numOfCol; i++) {
        rs.insert( std::pair<std::string, std::string>(nameOfCol[i], (valueOfCol[i]?valueOfCol[i]:"NULL")) );
    }
    return 0;
}
