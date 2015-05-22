#ifndef CONST_H
#define CONST_H
const int MAXLINE = 4096;
const int LISTENQ = 1024;
const int POCKET_SIZE = 2048;
const int WAIT_TIME_OUT_US = 10000;
const int SLEEP_TIME_US = 1000;
const int SERVER_MODE = 1;
const int CLIENT_MODE = SERVER_MODE + 1;
const int OP_USER_REGISTE   = 1;
const int OP_USER_DELETE    = OP_USER_REGISTE + 1;
const int OP_USER_LOGIN     = OP_USER_DELETE + 1;
const int OP_USER_LOGOUT    = OP_USER_LOGIN + 1;
const int OP_TEXT_LIST      = OP_USER_LOGOUT + 1;
const int OP_TEXT_NEW       = OP_TEXT_LIST + 1;
const int OP_TEXT_BLACK_ADD = OP_TEXT_NEW + 1;
const int OP_TEXT_BLACK_DEL = OP_TEXT_BLACK_ADD + 1;
const int OP_TEXT_GET       = OP_TEXT_BLACK_DEL + 1;
const int OP_TEXT_UPLOAD    = OP_TEXT_GET + 1;
const int OP_TEXT_DOWNLOAD  = OP_TEXT_UPLOAD + 1;
const int OP_TEXT_DELETE    = OP_TEXT_DOWNLOAD + 1;
const int OP_CHAR_SHOW      = OP_TEXT_DELETE + 1;
const int OP_CHAR_BOARDCASE = OP_CHAR_SHOW + 1;
const int OP_CHAR_TO_USER   = OP_CHAR_BOARDCASE + 1;
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
