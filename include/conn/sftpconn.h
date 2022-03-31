#pragma once

#define LIBSSH_STATIC 1

#include <libssh/libssh.h>
#include <libssh/sftp.h>

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

