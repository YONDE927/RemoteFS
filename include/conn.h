#pragma once

#define LIBSSH_STATIC 1

#include <libssh/libssh.h>
#include <libssh/sftp.h>
#include "attribute.h"
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

Connector* getConnector(char* configpath);

int connInit(Connector* connector,Authinfo* authinfo);
List* connReaddir(char* path);
Attribute* connStat(char* path);
int connRead(char* path, void* buffer, long offset, int size);
int connWrite(char* path, void* buffer, long offset, int size);



