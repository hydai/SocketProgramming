#ifndef CONST_H
#define CONST_H
const int MAXLINE = 4096;
const int LISTENQ = 1024;
const int POCKET_SIZE = 2048;
const int WAIT_TIME_OUT_US = 10000;
const int SLEEP_TIME_US = 1000;
const int SERVER_MODE = 1;
const int CLIENT_MODE = SERVER_MODE + 1;
const char ACK[] = "ACK";
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

    "CREATE TABLE reply (" \
    "tid integer NOT NULL, " \
    "time timestamp DEFAULT CURRENT_TIMESTAMP, " \
    "account varchar(64), " \
    "ip varchar(64), " \
    "port integer, " \
    "message varchar(128))",

    "CREATE TABLE blacklist (" \
    "tid integer NOT NULL, " \
    "blackacc varchar(64))",

    "CREATE TABLE fileintext (" \
    "tid integer NOT NULL, " \
    "account varchar(64), " \
    "filename varchar(64))"
};
#endif
