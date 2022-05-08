#pragma once

#define LIBSSH_STATIC 1

#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include "entry.h"
#include "list.h"

typedef struct Connector
{
    ssh_session m_ssh;
    sftp_session m_sftp;
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
    off_t offset;
    sftp_file fh;
} FileSession;

Connector* getConnector(char* configpath);

int connInit(Connector* connector,Authinfo* authinfo);
List* connReaddir(const char* path);
Attribute* connStat(const char* path);
FileSession* connOpen(const char* path, int flag);
int connRead(FileSession* file,off_t offset, void* buffer, int size);
int connWrite(FileSession* file,off_t offset, void* buffer, int size);
int connClose(FileSession* file);



