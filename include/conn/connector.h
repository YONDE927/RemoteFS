#include <libssh/sftp.h>
#include <libssh/libssh.h>
#include <pthread.h>
#include <sys/stat.h>

typedef struct RemoteConnector
{
    ssh_session ssh;
    sftp_session sftp;
    pthread_mutex_t mutex;
} RemoteConnector;

typedef enum FileType
{
    RegularFile,
    Directory,
    Other
} FileType;

typedef struct RemoteStat
{
    char path[256];
    struct stat;
    FileType filetype;
} RemoteStat;

RemoteConnector* GetRemoteConnector();

int conn_stat(char* path,RemoteStat* remotestat);
int conn_readdir(char* path);
