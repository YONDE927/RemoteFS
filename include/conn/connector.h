#include <libssh/sftp.h>
#include <libssh/libssh.h>
#include <pthread.h>

typedef struct RemoteConnector
{
    ssh_session ssh;
    sftp_session sftp;
    pthread_mutex_t mutex;
} RemoteConnector;
