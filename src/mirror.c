#include "list.h"
#include "map.h"
#include <sqlite3.h>
#include <string.h>

typedef struct Mirror {
    int permission;
    int sizesum;
    sqlite3* dbsession;
    void* config;
} Mirror;

typedef struct MirrorFile {
    char* path;
    int size;
    int mtime;
    int atime;
    int ref_cnt;
} MirrorFile;

void showMirrorFile(MirrorFile* file){
    if(file != NULL){
        printf("{path = %s, size = %d, mtime = %d, atime = %d, ref_cnt = %d}\n",
               file->path, file->size, file->mtime, file->atime, file->ref_cnt);
    }
}

void freeMirrorFile(MirrorFile* file){
    if(file != NULL){
        free((void*)(file->path));
        free(file);
    }
}

/*DBセッションを開始*/
int initDbSession(const char *filename, sqlite3 **ppDb){
    int rc;
    rc = sqlite3_open(filename, ppDb);
    if(rc){
        sqlite3_close(*ppDb);
        return -1;
    }
    return 0;
}

/*DBの状態を確認。現在はただバージョンを返す。*/
int getDbStatus(sqlite3* pdb){
    int rc;
    sqlite3_stmt *res;

    rc = sqlite3_prepare_v2(pdb, "SELECT SQLITE_VERSION()", -1, &res, 0);
    if(rc != SQLITE_OK){
        printf("db stmt failed.\n");
        return -1;
    }
    
    rc = sqlite3_step(res);
    if(rc == SQLITE_ROW){
        printf("%s\n", sqlite3_column_text(res, 0));
    }

    sqlite3_finalize(res);
    return 0;
}

/*ミラー用のDBテーブルを作成*/
int createMirrorTable(sqlite3* dbsession){
    int rc;
    sqlite3_stmt *stmt;

    if(dbsession == NULL){
        printf("createMirrorTable failed\n");
        return -1;
    }
    
    //sql text
    rc = sqlite3_prepare_v2(dbsession, 
            "CREATE TABLE IF NOT EXISTS Mirrors"
            "(path TEXT PRIMARY KEY, size INTEGER, mtime INTEGER"
            ", atime INTEGER, ref_cnt INTEGER);",
            -1, &stmt, 0);
    if(rc != SQLITE_OK){
        printf("invalid sql\n");
        return -1;
    }

    //exectute
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        printf("createMirrorTable failed\n");
        sqlite3_finalize(stmt);
    }

    //end
    sqlite3_finalize(stmt);
    return 0;
}

/*ミラーファイルの挿入*/
int insertMirrorFileToDB(sqlite3* dbsession, MirrorFile* file){
    int rc;
    sqlite3_stmt* stmt;

    //sql text
    rc = sqlite3_prepare_v2(dbsession, 
            "REPLACE INTO Mirrors VALUES "
            "( ?, ?, ?, ?, ?);",
            -1, &stmt, 0);
    if(rc != SQLITE_OK){
        printf("invalid sql\n");
        return -1;
    }else{
        sqlite3_bind_text(stmt, 1, file->path, -1, 0);
        sqlite3_bind_int(stmt, 2, file->size);
        sqlite3_bind_int(stmt, 3, file->mtime);
        sqlite3_bind_int(stmt, 4, file->atime);
        sqlite3_bind_int(stmt, 5, file->ref_cnt);
    }

    //exectute
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        printf("insertMirrorFile fail.\n");
        return -1;
    }

    //end
    sqlite3_finalize(stmt);
    return 0;
}

/*文字列コピー*/
char* strdup2(const char* str){
    int n;
    char* dest = 0;

    n = strlen(str) + 1;
    dest = malloc(n + 1);
    strncpy(dest, str, n);

    return dest;
}

/*ミラーファイルの検索*/
MirrorFile* lookupMirrorFileFromDB(sqlite3* dbsession, char* path){
    int rc;
    sqlite3_stmt* stmt;
    MirrorFile* file;

    //sql text
    rc = sqlite3_prepare_v2(dbsession, 
            "SELECT * FROM Mirrors WHERE "
            "path = ?;",
            -1, &stmt, 0);
    if(rc != SQLITE_OK){
        printf("invalid sql\n");
        return NULL;
    }else{
        sqlite3_bind_text(stmt, 1, path, -1, 0);
    }
    //exectute
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_ROW){
        printf("insertMirrorFile fail.\n");
        file = NULL;
    }else{
        file = malloc(sizeof(MirrorFile));
        file->path = strdup((const char*)sqlite3_column_text(stmt, 0));
        file->size = sqlite3_column_int(stmt, 2);
        file->mtime = sqlite3_column_int(stmt, 3);
        file->atime = sqlite3_column_int(stmt, 4);
        file->ref_cnt = sqlite3_column_int(stmt, 5);
    }
    return file;
}

/*ミラーファイルの削除*/
int deleteMirrorFileFromDB(sqlite3* dbsession, MirrorFile* file){
    int rc;
    sqlite3_stmt* stmt;

    if(file == NULL){
        return -1;
    }

    //sql text
    rc = sqlite3_prepare_v2(dbsession, 
            "DELETE FROM Mirrors WHERE "
            "path = ?;",
            -1, &stmt, 0);
    if(rc != SQLITE_OK){
        printf("invalid sql\n");
        return -1;
    }else{
        sqlite3_bind_text(stmt, 1, file->path, -1, 0);
    }
    //exectute
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_DONE){
        printf("insertMirrorFile fail.\n");
        return -1;
    }
    return 0;
}

/*結果表示用関数*/
int showCallback(void* Notused, int argc, char** argv, char **azColName){
    Notused = 0;
    for(int i = 0; i < argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : NULL);
    }
    printf("\n");
    return 0;
}

/*自由な検索*/
int customQuery(sqlite3* dbsession, char* query){
    int rc;
    char* errmsg = 0;

    rc = sqlite3_exec(dbsession, query, showCallback, 0, &errmsg);
    if(rc != SQLITE_OK){
        printf("customQuery failed.\n");
        return -1;
    }
    return 0;
}

int main(){
    Mirror mirror;
    MirrorFile file;
    MirrorFile* pfile;
    int rc;

    //create session
    rc = initDbSession("sample.db", &(mirror.dbsession));
    if(rc < 0){
        return 1;
    }

    //check status
    rc = getDbStatus(mirror.dbsession);
    if(rc < 0){
        return 1;
    }

    //create table
    rc = createMirrorTable(mirror.dbsession);
    if(rc < 0){
        return 1;
    }

    //show tables
    rc = customQuery(mirror.dbsession, "SELECT * FROM sqlite_master;");
    if(rc < 0){
        return 1;
    }

    //insert file
    file.path = "testfile";
    file.size = 1; file.mtime = 2; file.atime = 3; file.ref_cnt = 4;
    rc = insertMirrorFileToDB(mirror.dbsession, &file);
    if(rc < 0){
        return 1;
    }

    //show tables
    rc = customQuery(mirror.dbsession, "SELECT * FROM Mirrors;");
    if(rc < 0){
        return 1;
    }

    //lookup file
    pfile = lookupMirrorFileFromDB(mirror.dbsession, "testfile");
    if(pfile == NULL){
        return 1;
    }else{
        showMirrorFile(pfile);
    }

    //delete file
    rc = deleteMirrorFileFromDB(mirror.dbsession, pfile);
    if(rc < 0){
        return 1;
    }
    freeMirrorFile(pfile);//free pfile

    //lookup file
    pfile = lookupMirrorFileFromDB(mirror.dbsession, "testfile");
    if(pfile != NULL){
        return 1;
    }else{
        printf("no file\n");
    }
    return 0;
}
