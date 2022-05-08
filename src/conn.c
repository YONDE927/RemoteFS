#include <libssh/sftp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "conn.h"

#define CHUNK_SIZE 16384

int loadoption(char* path,Authinfo* authinfo)
{
    char ch;
    FILE* file;
    int count = 0;
    file = fopen(path, "r");
    if(file == NULL)
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
    static Authinfo* authinfo = NULL;
    if(connector==NULL)
    {
    connector = (Connector*)malloc(sizeof(Connector));
    authinfo = (Authinfo*)malloc(sizeof(Authinfo));
    if(loadoption(configpath, authinfo) < 0)
    {
        printf("loadoption failed.\n");
        free(connector);
        free(authinfo);
        connector = NULL;
        authinfo = NULL;
        return NULL;
    }
    if(connInit(connector, authinfo) < 0)
    {
        printf("connection cannot be established\n");
        free(connector);
        free(authinfo);
        connector = NULL;
        authinfo = NULL;
        return NULL;
    }
    }
    if(ssh_is_connected(connector->m_ssh))
    {
    return connector;
    }
    else
    {
    if(connInit(connector, authinfo) < 0)
    {
        printf("connection cannot be established\n");
        free(connector);
        free(authinfo);
        return NULL;
    }
    else
    {
        return connector;
    }
    }
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

/* connReaddirはpathを受け取り、AttributeのList構造体を領域確保ともにポインタを返却*/
List* connReaddir(const char* path)
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

/* connStatはAttributeのポインタを受け取りその領域を予約して取得した属性をコピーする。 */
Attribute* connStat(const char* path)
{
    printf("connStat %s\n",path);
    Attribute* attr = NULL;
    sftp_attributes sfstat;
    Connector* connector = getConnector(NULL);
    if(connector == NULL)
    {
    return NULL;
    }
    sfstat = sftp_stat(connector->m_sftp, path);
    if(sfstat != NULL)
    {
        attr = newAttr(strlen(path) + 1);
        strncpy(attr->path, path, strlen(path) + 1);
        attr->st.st_size  = sfstat->size;
        attr->st.st_atime = sfstat->atime;
        attr->st.st_mtime = sfstat->mtime;
        attr->st.st_ctime = sfstat->createtime;
        if(sfstat->type==1){
            attr->st.st_mode = S_IFREG | 0755;
            attr->st.st_nlink = 1;
        }else if(sfstat->type==2){
            attr->st.st_mode = S_IFDIR | 0755;
        attr->st.st_nlink = 2;
        }
    }
    return attr;
}

FileSession* connOpen(const char* path,int flag)
{
    FileSession* rf;
    sftp_file file;
    int path_size;
    Connector* connector = getConnector(NULL);
    file = sftp_open(connector->m_sftp, path, O_RDWR, 0);
    if(file == NULL)
    {
    printf("error %d\n",sftp_get_error(connector->m_sftp));
    return NULL;
    }

    path_size = strlen(path);
    rf = malloc(sizeof(FileSession));
    rf->path = malloc(sizeof(char)*(path_size + 1));
    strncpy(rf->path, path, path_size);
    rf->path[path_size] = '\0';
    rf->fh = file;
    rf->offset = 0;
    return rf;
}

int connRead(FileSession* file,off_t offset, void* buffer, int size)
{
    /* charを予約して、sftp_readしてコピーする。 */
    int read_sum = 0;
    int read_size = 0;
    int nbytes = 1;

    if(file == NULL){
        printf("Remotefile* file not exist\n");
        return -1;
    }

    //読み込み
    if(sftp_seek(file->fh, offset) < 0){
    return -1;
    }

    for(; (nbytes != 0) & (size > 0) ;){
        if( size > CHUNK_SIZE ){
            read_size = CHUNK_SIZE;
        }
        else{
            read_size = size;
        }
        nbytes = sftp_read(file->fh, buffer, read_size);
        if(nbytes < 0){
            return -1;
        }
        //次のreadのためにbufferのオフセットを更新して読み込み分サイズを減らす。
        buffer += nbytes;
        size -= nbytes;
        //総読み込みサイズの計算
        read_sum += nbytes;
    }
    return read_sum;
}

int connWrite(FileSession* file,off_t offset, void* buffer, int size)
{
    /* charを予約して、sftp_writeしてコピーする。 */
    int write_sum = 0;
    int write_size = 0;
    int nbytes = 1;
    
    if(file == NULL)
    {
        printf("Remotefile* file not exist\n");
        return -1;
    }

    //書き込み
    if(sftp_seek(file->fh, file->offset) < 0)
    {
        return -1;
    }

    for(; (nbytes != 0) & (size > 0) ;)
    {
        if( size > CHUNK_SIZE )
        {
            write_size = CHUNK_SIZE;
        }
        else
        {
            write_size = size;
        }
        nbytes = sftp_write(file->fh, buffer, write_size);
        if(nbytes < 0)
        {
            return -1;
        }
        //次のreadのためにbufferのオフセットを更新して読み込み分サイズを減らす。
        buffer += nbytes;
        size -= nbytes;
        //総読み込みサイズの計算
        write_sum += nbytes;
    }
    return write_sum;
}

int connClose(FileSession* file)
{
    if(file == NULL)
    {
    printf("Remotefile* file not exist\n");
    return -1;
    }

    if(sftp_close(file->fh) == SSH_ERROR)
    {
    printf("sftp_close error\n");
    return -1;
    }
    free(file->path);
    free(file);
    file = NULL;
    return 0;
}
