#ifndef HW2_LIB_H
#define HW2_LIB_H
#include <sqlite3.h>
#include <map>
#include <string>
typedef std::map<std::string, std::string> resultSet;
void show_welcome_message();
void show_lobby_message();
void logging(std::string msg);
bool init_db(sqlite3* &db);
resultSet exec_sql(sqlite3* &db, std::string sql);
static int callback(void *vrs, int numOfCol, char **valueOfCol, char **nameOfCol);
void close_db(sqlite3* &db);

const int INIT_SQL_SIZE = 5;
const int MAX_SQL_LENGTH = 500;
const char init_sql[INIT_SQL_SIZE][MAX_SQL_LENGTH] = {
    "CREATE TABLE user (" \
    "account varchar(64) PRIMARY KEY NOT NULL, " \
    "password varchar(64));",

    "CREATE TABLE text (" \
    "tid integer PRIMARY KEY NOT NULL, " \
    "title varchar(64), " \
    "content varchar(300), " \
    "time timestamp DEFAULT CURRENT_TIMESTAMP, " \
    "account varchar(64), " \
    "ip varchar(64), " \
    "port integer, " \
    "hit integer DEFAULT 0)",

    ""
};
#endif
