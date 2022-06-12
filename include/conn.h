#pragma once

#define LIBSSH_STATIC 1

#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include <pthread.h>
#include "entry.h"
#include "list.h"
#include "rawtp/connection.h"
#include "rawtp/fileoperation.h"

#define RAW

#ifdef SFTP
typedef struct Connector
{
    ssh_session m_ssh;
    sftp_session m_sftp;
    pthread_mutex_t mutex;
} Connector;

typedef struct Authinfo
{
    char host[64];
    char username[64];
    char password[64];
} Authinfo;

typedef struct FileSession
{
    char* path;
    sftp_file fh;
} FileSession;

Connector* getConnector(char* configpath);

int connInit(Connector* connector, Authinfo* authinfo);
List* connReaddir(const char* path);
Attribute* connStat(const char* path);
FileSession* connOpen(const char* path, int flag);
int connRead(FileSession* file, off_t offset, void* buffer, int size);
int connWrite(FileSession* file, off_t offset, void* buffer, int size);
int connClose(FileSession* file);
int connStatus();
#endif

#ifdef RAW 
typedef struct Connector
{
    int sockfd;
    pthread_mutex_t mutex;
} Connector;

typedef struct Authinfo
{
    char host[64];
    short port;
} Authinfo;

typedef struct FileSession
{
    char* path;
    int fh;
} FileSession;

Connector* getConnector(char* configpath);

int connInit(Connector* connector, Authinfo* authinfo);
List* connReaddir(const char* path);
Attribute* connStat(const char* path);
FileSession* connOpen(const char* path, int flag);
int connRead(FileSession* file, off_t offset, void* buffer, int size);
int connWrite(FileSession* file, off_t offset, void* buffer, int size);
int connClose(FileSession* file);
int connStatus();
#endif

