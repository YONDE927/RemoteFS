#include "list.h"
#include "map.h"
#include <sqlite3.h>
#include <stdio.h>

typedef struct Mirror {
    int permission;
    int sizesum;
    sqlite3* dbsession;
    void* config;
} Mirror;

int initDbSession(const char *filename, sqlite3 **ppDb){
    int rc;
    rc = sqlite3_open(filename, ppDb);
    if(rc){
        sqlite3_close(*ppDb);
        return -1;
    }
    return 0;
}

int getDbStatus(sqlite3* pdb){
    int rc;
    sqlite3_stmt *res;

    rc = sqlite3_prepare_v2(pdb, "SELECT SQLITE_VERSION()", -1, &res, 0);
    if(rc != SQLITE_OK){
        printf("db error\n");
        return -1;
    }

    rc = sqlite3_step(res);
    if(rc == SQLITE_ROW){
        printf("%s\n", sqlite3_column_text(res, 0));
    }
    sqlite3_finalize(res);
    return 0;
}

int main(){
    Mirror mirror;
    char* db_file = "sample.db";
    int rc;

    rc = initDbSession(db_file,&(mirror.dbsession));
    if(rc < 0){
        return 1;
    }

    rc = getDbStatus(mirror.dbsession);
    if(rc < 0){
        return 1;
    }

    return 0;
}
