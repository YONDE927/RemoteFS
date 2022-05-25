#include <stdio.h>
#include <sqlite3.h>
#include <time.h>

//DB接続初期化
int initDbSession(const char *filename, sqlite3 **ppDb){
    int rc;

    //sqlite3のスレッド対応
    rc = sqlite3_initialize();
    rc = sqlite3_config(SQLITE_CONFIG_SERIALIZED);

    rc = sqlite3_open(filename, ppDb);
    if(rc){
        sqlite3_close(*ppDb);
        return -1;
    }
    return 0;
}

/*DB接続を終了*/
int closeDbSession(sqlite3* pDb){
    int rc;
    sqlite3_close(pDb);
    return 0;
}

/*DBテーブルを作成*/
int createRecordTable(sqlite3* dbsession){
    int rc;
    sqlite3_stmt *stmt;

    if(dbsession == NULL){
        printf("createMirrorTable failed\n");
        return -1;
    }
    
    //sql text
    rc = sqlite3_prepare_v2(dbsession, 
            "CREATE TABLE IF NOT EXISTS Record"
            "(path TEXT, operation TEXT, time INTEGER);",
            -1, &stmt, 0);
    if(rc != SQLITE_OK){
        printf("invalid sql\n");
        return -1;
    }

    //exectute
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        printf("createRecordTable failed\n");
        sqlite3_finalize(stmt);
    }

    //end
    sqlite3_finalize(stmt);
    return 0;
}

typedef enum op_t {
    OPEN,
    CLOSE,
    READ,
    WRITE,
    GETATTR,
    READDIR
} op_t;

char* opname[] = {"OPEN","CLOSE","READ","WRITE","GETATTR","READDIR"};

//基本形
int recordOperation(sqlite3* dbsession, char* path, op_t op){
    int rc;
    sqlite3_stmt* stmt;
    time_t now;

    //time
    now = time(NULL);
    //sql text
    rc = sqlite3_prepare_v2(dbsession, 
            "REPLACE INTO Mirrors VALUES "
            "( ?, ?, ?, ?, ?);",
            -1, &stmt, 0);
    if(rc != SQLITE_OK){
        printf("invalid sql\n");
        return -1;
    }else{
        sqlite3_bind_text(stmt, 1, path, -1, 0);
        sqlite3_bind_text(stmt, 2, opname[op], -1, 0);
        sqlite3_bind_int(stmt, 3, (int)now);
    }

    //exectute
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        printf("insert record fail.\n");
        return -1;
    }

    //end
    sqlite3_finalize(stmt);
    return 0;
}
