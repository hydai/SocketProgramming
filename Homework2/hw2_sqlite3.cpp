#include "hw2_lib.h"
#include "const.h"

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
   int rc = 0;
   for (int i = 0; i < INIT_SQL_SIZE; i++) {
       exec_sql(db, std::string(init_sql[i]), rc);
   }
   
   return true;
}

void close_db(sqlite3* &db) {
    sqlite3_close(db);
}

result_set exec_sql(sqlite3* &db, std::string sql, int &rc) {
    char *errMsg = NULL;
    result_set rs;
    rc = sqlite3_exec(db, sql.c_str(), callback, (void *)&rs, &errMsg);
    if(rc != SQLITE_OK) {
        logging("SQL error: " + std::string(errMsg) + "\n");
        sqlite3_free(errMsg);
    } else {
        logging("SQL OK: Operation done successfully\n");
    }
    return rs;
}

static int callback(void *vrs, int numOfCol, char **valueOfCol, char **nameOfCol){
    std::map<std::string, std::string> rs;
    for (int i = 0; i < numOfCol; i++) {
        logging(std::string("Query: ") + nameOfCol[i] + " " + (valueOfCol[i]?valueOfCol[i]:"NULL"));
        rs.insert( std::pair<std::string, std::string>(nameOfCol[i], (valueOfCol[i]?valueOfCol[i]:"NULL")) );
    }
    ((result_set *)vrs)->push_back(rs);
    return 0;
}
