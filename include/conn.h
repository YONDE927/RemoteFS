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

typedef struct Remotefile
{
    char* path;
    off_t offset;
    sftp_file fh;
} Remotefile;

Connector* getConnector(char* configpath);

int connInit(Connector* connector,Authinfo* authinfo);
List* connReaddir(const char* path);
Attribute* connStat(const char* path);
int connRead(const char* path, void* buffer, long offset, int size);
int connWrite(const char* path, void* buffer, long offset, int size);



