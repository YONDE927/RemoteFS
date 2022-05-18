#include "list.h"
#include "map.h"
#include "conn.h"
#include "entry.h"

#include <sqlite3.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>

#define BLOCK_SIZE 4048 //4KB
                        
/**********************/
/*共通のコンストラクタ*/
/**********************/

typedef struct Mirror {
    pthread_rwlock_t* task_rwlock;
    pthread_mutex_t* list_lock;
    pthread_cond_t* list_cond;
    int sizesum;
    int killswitch;
    sqlite3* dbsession;
    char* root;
    List* tasklist; //List of MirrorTask
    pthread_t taskthread;
} Mirror;

typedef struct MirrorFile {
    char* path;
    int size;
    int mtime;
    int atime;
    int ref_cnt;
} MirrorFile;

//コンストラクタはcreateTask
typedef struct MirrorTask {
    MirrorFile* file;
    char* path;
    int block_num;
    int iterator;
    struct stat st;
} MirrorTask;

int initDbSession(const char *filename, sqlite3 **ppDb);
int closeDbSession(sqlite3* pDb);
int createMirrorTable(sqlite3* dbsession);
int getMirrorUsedStorage(sqlite3* dbsession);
MirrorTask* createTask(const char* path);
void freeMirrorTask(void* task);

Mirror* constructMirror(char* dbname, char* root){
    int rc;
    Mirror* mirror;

    mirror = malloc(sizeof(Mirror));
    rc = initDbSession(dbname, &(mirror->dbsession));
    if(rc < 0){
        return NULL;
    }

    rc = createMirrorTable(mirror->dbsession);
    if(rc < 0){
        return NULL;
    }

    mirror->sizesum = getMirrorUsedStorage(mirror->dbsession);

    mirror->root = strdup(root);

    mirror->killswitch = 0;

    rc = pthread_rwlock_init(mirror->task_rwlock, NULL);
    if(rc < 0){
        return NULL;
    }

    rc = pthread_mutex_init(mirror->list_lock, NULL);
    if(rc < 0){
        return NULL;
    }

    rc = pthread_cond_init(mirror->list_cond, NULL);
    if(rc < 0){
        return NULL;
    }

    return mirror;
}

void freeMirror(Mirror* mirror){
    free(mirror->root);
    pthread_rwlock_destroy(mirror->task_rwlock);
    pthread_mutex_destroy(mirror->list_lock);
    pthread_cond_destroy(mirror->list_cond);
    closeDbSession(mirror->dbsession);
    freeList(mirror->tasklist, freeMirrorTask);
}

MirrorFile* constructMirrorFile(const char* path, struct stat st){
    MirrorFile* file;
    
    file = malloc(sizeof(MirrorFile));
    file->path = strdup(path);
    file->mtime = st.st_mtime;
    file->atime = st.st_atime;
    file->size = st.st_size;
    file->ref_cnt = 0;
    return file;
}


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

/******************************/
/*共通のコンストラクタここまで*/
/******************************/

/************/
/*文字列処理*/
/************/

/*文字列コピー*/
char* strdup2(const char* str){
    int n;
    char* dest = 0;

    n = strlen(str) + 1;
    dest = malloc(n + 1);
    strncpy(dest, str, n);

    return dest;
}

/*"/"を"%"に変える*/
char* convertFileName(const char* path){
    char* name;
    int size;

    name = strdup(path);
    size = strlen(name);
    for(int i = 0; i < size; i++){
        if(name[i] == '/'){
            name[i] = '%';
        }
    }
    return name;
}

/*実際のミラーファイルのパスを生成*/
char* getMirrorPath(Mirror* mirror, const char* path){
    int root_size, path_size;
    char* mirrorpath, *convertpath;

    convertpath = convertFileName(path);

    root_size = strlen(mirror->root);
    path_size = strlen(convertpath);

    mirrorpath = malloc(root_size + path_size);
    strncpy(mirrorpath, mirror->root, root_size + 1);
    strcat(mirrorpath, convertpath);

    free(convertpath);

    return mirrorpath;
} 

/********************/
/*文字列処理ここまで*/
/********************/

/**********************/
/* DBに関するコード群 */
/**********************/

/*DBセッションを開始*/
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

/*DBセッションを終了*/
int closeDbSession(sqlite3* pDb){
    int rc;
    sqlite3_close(pDb);
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

/*ミラーファイルの検索*/
MirrorFile* lookupMirrorFileFromDB(sqlite3* dbsession, const char* path){
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
        printf("lookupMirrorFileFromDB fail.\n");
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
        printf("delete MirrorFile fail.\n");
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

/*ミラーファイルの数*/
int getMirrorFileNum(sqlite3* dbsession){
    int rc;
    
    rc = customQuery(dbsession, "SELECT COUNT(*) FROM Mirrors;");
    return rc;
}

/*ミラーストレージの使用量*/
int getMirrorUsedStorage(sqlite3* dbsession){
    int rc;
    sqlite3_stmt* stmt;

    //sql text
    rc = sqlite3_prepare_v2(dbsession, 
            "SELECT SUM(SIZE) FROM Mirrors;",
            -1, &stmt, 0);
    if(rc != SQLITE_OK){
        printf("invalid sql\n");
        return -1;
    }
    //exectute
    rc = sqlite3_step(stmt);
    if(rc != SQLITE_ROW){
        printf("getMirrorUsedStorage fail.\n");
        return -1;
    }else{
        rc = sqlite3_column_int(stmt, 0);
    }
    return rc;
}

/****************************/
/*DBに関するコード群ここまで*/
/****************************/

/**********************/
/*通信に関するコード群*/
/**********************/

/*タスクの作成*/
MirrorTask* createTask(const char* path){
    MirrorTask* task;
    Attribute* attribute;

    //コネクタの取得
    Connector* connector = getConnector(NULL);
    if(connector == NULL){
        return NULL;
    }
   
    //Attributeの取得
    attribute = connStat(path);
    if(attribute == NULL){
        return NULL;
    }

    //MirrorTaskの作成
    task = malloc(sizeof(MirrorTask));
    task->file = constructMirrorFile(path, attribute->st);
    task->st = attribute->st;
    task->path = strdup(path);
    task->block_num = attribute->st.st_size / BLOCK_SIZE;
    task->iterator = 0;

    return task;
}

/*タスクの解放*/
void freeMirrorTask(void* pointer){
    MirrorTask* task = pointer;
    free(task->path);
    freeMirrorFile(task->file);
    free(task);
}

/*タスクの実行*/
int execTask(Mirror* mirror, MirrorTask* task){
    int rc, block_num, offset, size;
    FileSession* filesession;
    Attribute* attribute;
    MirrorFile* file;
    FILE* fp; 
    char* mirrorpath;
    char buffer[4048];

    const char* path = task->path;
    file = task->file;

    Connector* connector = getConnector(NULL);
    if(connector == NULL){
        return -1;
    }
   
    filesession = connOpen(path, 0);
    if(filesession == NULL){
        return -1;
    }

    //ストレージ上のファイル作成とオープン
    mirrorpath = getMirrorPath(mirror, path);
    fp = fopen(mirrorpath, "w+");
    if(fp == NULL){
        free(mirrorpath);
        return -1;
    }

    //ブロックごとのダウンロードループ
    for(task->iterator = 0; task->iterator < task->block_num; task->iterator++){
        pthread_rwlock_wrlock(mirror->task_rwlock);
        rc = connRead(filesession, (off_t)(task->iterator * BLOCK_SIZE), buffer, 4048);
        if(rc < 0){
            break;
        }
        pthread_rwlock_unlock(mirror->task_rwlock);
        fseek(fp, offset, SEEK_SET);
        rc = fwrite(buffer, 1, rc, fp);
    }
    fclose(fp);

    //DBへ登録
    rc = insertMirrorFileToDB(mirror->dbsession, file);
    if(rc < 0){
        free(mirrorpath);
        return -1;
    }

    //メモリ解放
    free(mirrorpath);
    return 0;
}

/*タスクの追加*/
int appendTask(Mirror* mirror, const char* path){
    MirrorTask* task;

    task = createTask(path);
    if(task == NULL){
        return -1;
    }

    push_back(mirror->tasklist, task, sizeof(MirrorTask));
    return 0;
}

/*タスクの削除*/
void deleteTask(Mirror* mirror, const char* path){
    Node* tmpnode, *node, *prenode;
    MirrorTask* task;

    node = mirror->tasklist->head;
    prenode = NULL;
    for(;node != NULL;){
        tmpnode = node;
        task = (MirrorTask*)(node->data);

        if(strcmp(task->path, path) == 0){
            //リストの先頭の時
            if(prenode != NULL){
                mirror->tasklist->head = node->next;
            }else{
                prenode->next = node->next;
            }
            //先にノードを進めておく
            node = node->next;
            //nodeの解放
            freeMirrorTask(task);
            free(tmpnode);
        }else{
            node = node->next;
            prenode = node;
        }
    }
}

/*総合タスク管理*/
/*ミラー通信のロック方針はミラー側はブロック単位でファイルシステム側はオペレーション単位でロックする。*/
void* loopTask(void* pmirror){
    int rc;
    Node* node;
    MirrorTask* task;
    Mirror* mirror;

    mirror = pmirror;
    while(1){
        if(mirror->killswitch == 1){
            break;
        }
        node = get_front(mirror->tasklist);

        pthread_mutex_lock(mirror->list_lock);

        while(node == NULL){
            pthread_cond_wait(mirror->list_cond, mirror->list_lock);
        }
        task = node->data;
        rc = execTask(mirror, task);
        pop_front(mirror->tasklist, freeMirrorTask);

        pthread_mutex_unlock(mirror->list_lock);
    }
    return NULL;
}

/*ミラーリングのスレッドを開始*/
int startMirroring(Mirror* mirror){
    int rc;

    mirror->killswitch = 0;
    rc = pthread_create(&(mirror->taskthread), NULL, loopTask, mirror);
    if(rc != 0){
        printf("create thread fail\n");
        return -1;
    }
    return 0;
}
/******************************/
/*通信に関するコード群ここまで*/
/******************************/

/**********************/
/*ミラーファイルの管理*/
/**********************/

/*ミラーファイルの削除*/
int deleteMirrorFile(const char* path){
    return 0;
}

/******************************/
/*ミラーファイルの管理ここまで*/
/******************************/

/********************************/
/*インターフェースに関するコード*/
/********************************/
void request_mirror(Mirror* mirror, const char* path){
    int rc;
    MirrorTask* task;

    rc = appendTask(mirror, path);
    pthread_cond_signal(mirror->list_cond);
}

MirrorFile* search_mirror(Mirror* mirror, const char* path){
    return lookupMirrorFileFromDB(mirror->dbsession, path);
}


void check_mirror(Mirror* mirror, const char* path){
    int rc;
    
    rc = getMirrorFileNum(mirror->dbsession);
}

int openMirrorFile(MirrorFile* file);
int readMirrorFile(MirrorFile* file, off_t offset, size_t size, char* buf);
int writeMirrorFile(MirrorFile* file, off_t offset, size_t size, char* buf);
int closeMirrorFile(MirrorFile* file);

/****************************************/
/*インターフェースに関するコードここまで*/
/****************************************/

/*検証用*/
int main(int argc, char** argv){
    Mirror* mirror;
    MirrorFile file;
    MirrorFile* pfile;
    Connector* conn;
    int rc;
    char* sshconfig;

    if(argc < 2){
        printf("mirror [sshconfig]\n");
        exit(EXIT_FAILURE);
    }
    sshconfig = argv[1];
    conn = getConnector(sshconfig);

    mirror = constructMirror("sample.db", "mirrordata");
    if(mirror == NULL){
        printf("constructMirror fail\n");
        exit(EXIT_FAILURE);
    }

    //check status
    rc = getDbStatus(mirror->dbsession);
    if(rc < 0){
        return 1;
    }

    //create table
    rc = createMirrorTable(mirror->dbsession);
    if(rc < 0){
        return 1;
    }

    //show tables
    rc = customQuery(mirror->dbsession, "SELECT * FROM sqlite_master;");
    if(rc < 0){
        return 1;
    }

    //insert file
    file.path = "testfile";
    file.size = 1; file.mtime = 2; file.atime = 3; file.ref_cnt = 4;
    rc = insertMirrorFileToDB(mirror->dbsession, &file);
    if(rc < 0){
        return 1;
    }

    //show tables
    rc = customQuery(mirror->dbsession, "SELECT * FROM Mirrors;");
    if(rc < 0){
        return 1;
    }

    //lookup file
    pfile = lookupMirrorFileFromDB(mirror->dbsession, "testfile");
    if(pfile == NULL){
        return 1;
    }else{
        showMirrorFile(pfile);
    }

    //delete file
    rc = deleteMirrorFileFromDB(mirror->dbsession, pfile);
    if(rc < 0){
        return 1;
    }
    freeMirrorFile(pfile);//free pfile

    //lookup file
    pfile = lookupMirrorFileFromDB(mirror->dbsession, "testfile");
    if(pfile != NULL){
        return 1;
    }else{
        printf("no file\n");
    }

    //connection test and task test
    MirrorTask* task;
    task = createTask("/home/yonde/Documents/RemoteFS/src/mirror.c");
    if(task == NULL){
        printf("crate task fail.\n");
        exit(EXIT_FAILURE);
    }
     
    return 0;
}
