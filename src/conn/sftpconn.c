#include <libssh/sftp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "conn/sftpconn.h"
#include "fs/attribute.h"
#include "utils/list.h"

int loadoption(char* path,Authinfo* authinfo)
{
    char ch;
    FILE* file;
    int count = 0;
    file = fopen(path, "r");
    if(file < 0)
    {
	return -1;
    }
    ch = getc(file);
    while((ch != EOF) && (ch != '\n'))
    {
	authinfo->host[count] = ch;	
	ch = getc(file);
	count ++;
    }
    authinfo->host[count] = '\0';

    ch = getc(file);
    count = 0;
    while((ch != EOF) && (ch != '\n'))
    {
	authinfo->username[count] = ch;	
	ch = getc(file);
	count ++;
    }
    authinfo->username[count] = '\0';

    ch = getc(file);
    count = 0;
    while((ch != EOF) && (ch != '\n'))
    {
	authinfo->password[count] = ch;	
	ch = getc(file);
	count ++;
    }
    authinfo->password[count] = '\0';
    return 0;
}

Connector* getConnector(char* configpath)
{
    static Connector* connector = NULL;
    Authinfo* authinfo;
    if(connector==NULL)
    {
	connector = (Connector*)malloc(sizeof(Connector));
	authinfo = (Authinfo*)malloc(sizeof(Authinfo));
	if(loadoption(configpath, authinfo) < 0)
	{
	    printf("loadoption failed.\n");
	    free(connector);
	    free(authinfo);
	    return NULL;
	}
	if(connInit(connector, authinfo) < 0)
	{
	    printf("connection is not established\n");
	    free(connector);
	    free(authinfo);
	    return NULL;
	}
	free(authinfo);
    }
    if(ssh_is_connected(connector->m_ssh))
    {
	return connector;
    }
    return NULL;
}

int connInit(Connector* connector,Authinfo* authinfo)
{
    connector->m_ssh = ssh_new();
    ssh_options_set(connector->m_ssh, SSH_OPTIONS_HOST, authinfo->host);
    if(ssh_connect(connector->m_ssh)!=SSH_OK){
        ssh_free(connector->m_ssh);
	return -1;
    }
    if(ssh_userauth_password(connector->m_ssh,authinfo->username,authinfo->password)!=SSH_AUTH_SUCCESS){
	ssh_free(connector->m_ssh);
	return -1;
    }
    connector->m_sftp = sftp_new(connector->m_ssh);
    if(sftp_init(connector->m_sftp)!=SSH_OK){
	sftp_free(connector->m_sftp);
        ssh_free(connector->m_ssh);
	return -1;
    }
    return 0;
}

List* connReaddir(char* path)
{
    Connector* connector = getConnector(NULL);
    List* list = newList();
    Attribute attr;
    sftp_attributes sfstat;
    sftp_dir dir;
    int pathlen;

    //open direcotry
    if(connector==NULL)
    {
	printf("Connector is down.\n");
	return list;
    }
    dir = sftp_opendir(connector->m_sftp, path);
    if(dir==NULL)
    {
	printf("error open dir.\n");
	return list;
    }
    sfstat = sftp_readdir(connector->m_sftp, dir);
    while(sfstat!=NULL)
    {
	pathlen = strlen(sfstat->name) + 1;
	attr.path = malloc(sizeof(char) * pathlen);
	strncpy(attr.path, sfstat->name, pathlen);
	attr.st.st_size = sfstat->size;
	attr.st.st_atime = sfstat->atime;
	attr.st.st_mtime = sfstat->mtime;
	attr.st.st_nlink = 1;
	push_front(list, &attr, sizeof(Attribute));
	sfstat = sftp_readdir(connector->m_sftp, dir);
    }
    return list;
}

int connStat(char* path, Attribute* attr)
{
    return 0;
}

int connOpen(char* path)
{
    return 0;
}

int connRead(char* path, void* buffer, int size)
{
    return 0;
}

int connWrite(char* path, void* buffer, int size)
{
    return 0;
}

int connClose(char* path)
{
    return 0;
}

int connDownload(char* src, char* dest)
{
    return 0;
}

int main()
{
    char* configpath = "./ssh.config";
    Connector* connector = getConnector(configpath);
    Connector* con2 = getConnector(NULL);
    if(con2 != NULL)
    {
	printf("static conn succeed\n");
    }
    List* list = connReaddir("/home/yonde/Documents/RemoteFS");
}
