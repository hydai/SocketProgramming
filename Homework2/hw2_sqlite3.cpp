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
   for (int i = 0; i < INIT_SQL_SIZE; i++) {
       exec_sql(db, std::string(init_sql[i]));
   }
   
   return true;
}

void close_db(sqlite3* &db) {
    sqlite3_close(db);
}

result_set exec_sql(sqlite3* &db, std::string sql) {
    char *errMsg = NULL;
    int rc = 0;
    result_set rs;
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
    result_set rs = *(result_set *)vrs;
    for (int i = 0; i < numOfCol; i++) {
        rs.insert( std::pair<std::string, std::string>(nameOfCol[i], (valueOfCol[i]?valueOfCol[i]:"NULL")) );
    }
    return 0;
}
