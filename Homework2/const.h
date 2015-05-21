#ifndef CONST_H
#define CONST_H
const int MAXLINE = 4096;
const int LISTENQ = 1024;
const int POCKET_SIZE = 2048;
const int WAIT_TIME_OUT_US = 10000;
const int SLEEP_TIME_US = 1000;
const int OP_User_Registe   = 1;
const int OP_User_Delete    = OP_User_Registe + 1;
const int OP_User_Login     = OP_User_Delete + 1;
const int OP_User_Logout    = OP_User_Login + 1;
const int OP_Text_List      = OP_User_Logout + 1;
const int OP_Text_New       = OP_Text_List + 1;
const int OP_Text_Black_Add = OP_Text_New + 1;
const int OP_Text_Black_Del = OP_Text_Black_Add + 1;
const int OP_Text_Get       = OP_Text_Black_Del + 1;
const int OP_Text_Upload    = OP_Text_Get + 1;
const int OP_Text_Download  = OP_Text_Upload + 1;
const int OP_Text_Delete    = OP_Text_Download + 1;
const int OP_Char_Show      = OP_Text_Delete + 1;
const int OP_Char_Boardcase = OP_Char_Show + 1;
const int OP_Char_To_User   = OP_Char_Boardcase + 1;
const char ACK[] = "ACK";
#endif
