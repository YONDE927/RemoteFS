#include "remotefs.h"
#include "conn.h"
#include "list.h"
#include "map.h"
#include "entry.h"
#include <fuse3/fuse.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef struct FileHandler 
{
    off_t offset;
    FileSession* session;
} FileHandler;

typedef struct FsData
{
    IntMap* FhMap;
    char* RemoteRoot;
} FsData;

typedef struct Args
{
    char* RemoteRoot;
    char* SshConfig;
} Args;

Args* getArgs(char* RemoteRoot, char* SshConfig)
{
    static Args* args = NULL;
    int len;
    if(args == NULL)
    {
	args = malloc(sizeof(Args));
	if(RemoteRoot != NULL)
	{
	    len = strlen(RemoteRoot);
	    args->RemoteRoot = malloc(sizeof(char) * (len + 1)); 
	    strncpy(args->RemoteRoot, RemoteRoot, len);
	    args->RemoteRoot[len] = '\0';
	}
	if(SshConfig != NULL)
	{
	    len = strlen(SshConfig);
	    args->SshConfig = malloc(sizeof(char) * (len + 1)); 
	    strncpy(args->SshConfig, SshConfig, len);
	    args->SshConfig[len] = '\0';
	}
    }
    return args;
}

IntMap* getFhMap()
{
    FsData* fs = fuse_get_context()->private_data;
    return fs->FhMap;
}

char* getRoot()
{
    FsData* fs = (FsData*)fuse_get_context()->private_data;
    return fs->RemoteRoot;
}

char* patheditor(const char* path)
{
    int len1, len2;
    char* root,* buffer;
   
    root = getRoot(); 
    len1 = strlen(root);
    len2 = strlen(path);
    buffer = malloc(sizeof(char) * (len1 + len2 + 1));
    strncpy(buffer, root, len1);
    buffer[len1] = '\0';
    strncat(buffer, path, len2);
    buffer[len1 + len2] = '\0';
    return buffer;
}

void* fuseInit(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    int len;
    Args* args;
    FsData* fs;
    Connector* connecotor;

    args = getArgs(NULL, NULL);
    if(args == NULL)
    {
	printf("no args\n");
	exit(EXIT_FAILURE);
    }

    fs = malloc(sizeof(FsData));
    fs->FhMap = newIntMap();
    puts("aa");
    //SSH接続とSFTPセッションの確立 
    connecotor = getConnector(args->SshConfig);
    if(connecotor == NULL)
    {
	printf("SSH session is not established\n");
	exit(EXIT_FAILURE);
    }

    //FsDataにリモートサーバーのルートパスを設定
    len = strlen(args->RemoteRoot);
    fs->RemoteRoot = malloc(sizeof(char) * (len + 1)); 
    strncpy(fs->RemoteRoot, args->RemoteRoot, len);
    fs->RemoteRoot[len] = '\0';
    
    printf("Init %s\n", fs->RemoteRoot);
    return fs;
}

int fuseGetattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
    printf("getattr %s\n",path);
    Attribute* attr;
    char* RemotePath;

    RemotePath = patheditor(path);
    attr = connStat(RemotePath);
    if(attr == NULL)
    {
	printf("connStat error\n");
	free(RemotePath);
	return -ENOENT;
    }
    *stbuf = attr->st;
    free(RemotePath);
    return 0;
}

int fuseOpen(const char *path, struct fuse_file_info *fi)
{
    int map_size;
    int ind,fh;
    char* RemotePath;
    IntMap* FhMap;
    FileHandler file = {0};
    FileSession* session;

    //ファイルハンドラマップを取得
    FhMap = getFhMap();
    /* Passing 'const char *' to parameter of type 'char *' discards qualifiers* ファイルハンドラの生成 */
    //list opened file handers
    map_size = lenIntMap(FhMap);
    int* fhs = malloc(sizeof(int) * map_size);
    ind = 0;
    for(IntMapNode* node = FhMap->head; node != NULL; node = node->next)
    {
	fh = node->key;
	fhs[ind] = fh;
	ind++;
    }
    //search new file handler
    for(int i = 0; ; i++)
    {
	int isNew = 1;
	for(int j = 0; j < map_size; j++)
	{
	    if(i == fhs[j])
	    {
		isNew = 0;
	    }
	}
	if(isNew)
	{
	    fh = i;
	    break;
	}
    }
    //探索用ファイルハンドラリストの解放
    free(fhs);

    /* リモートファイルのオープン */
    RemotePath = patheditor(path);
    session = connOpen(path, fi->flags);
    if(session == NULL)
    {
	free(RemotePath);
	return -ENOENT;
    }
    file.session = session;
    free(RemotePath);

    /* ファイルハンドラの管理用マップ構造体へ登録 */
    insIntMap(FhMap, fh, &file, sizeof(FileHandler)); 
    fi->fh = fh;
    return 0;  
}

int fuseRead(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
    IntMap* FhMap; 
    FileHandler* fh;
    int rc;

    //ファイルハンドラマップを取得
    FhMap = getFhMap();

    fh = getIntMap(FhMap, fi->fh);
    if(fh == NULL)
    {
	return -EBADFD;
    }
   
    //オフセットの設定 
    fh->offset = offset;
    fh->session->offset = offset;

    //通信呼び出し
    rc = connRead(fh->session, buffer, size);
    if(rc < 0)
    {
	return -ENETDOWN;
    }

    //オフセットの設定 
    fh->offset += rc;
    fh->session->offset += rc;
    return 0;
}

int fuseWrite(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *fi)
{
    IntMap* FhMap;
    FileHandler* fh;
    int rc;

    //ファイルハンドラマップを取得
    FhMap = getFhMap();

    fh = getIntMap(FhMap, fi->fh);
    if(fh == NULL)
    {
	return -EBADFD;
    }
   
    //オフセットの設定 
    fh->offset = offset;
    fh->session->offset = offset;

    //通信呼び出し
    rc = connWrite(fh->session, (void*)buffer, size);
    if(rc < 0)
    {
	return -ENETDOWN;
    }

    //オフセットの設定 
    fh->offset += rc;
    fh->session->offset += rc;

    return 0;    
}

int fuseRelease(const char *path, struct fuse_file_info *fi)
{
    IntMap* FhMap;
    FileHandler* fh;
    int rc;

    //ファイルハンドラマップを取得
    FhMap = getFhMap();

    fh = getIntMap(FhMap, fi->fh);
    if(fh == NULL)
    {
	return -EBADFD;
    }
   
    //通信呼び出し
    rc = connClose(fh->session);
    if(rc < 0)
    {
	return -ENETDOWN;
    }

    //対象ファイルのFileHandlerをfhMapから削除して解放
    delIntMap(FhMap, fi->fh);

    return 0;    
}

int fuseReaddir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
    List* attrs;
    char* RemotePath;
    
    attrs = connReaddir(path); 
    if(attrs == NULL)
    {
	return -ENETDOWN;
    }
    for(Node* node = attrs->head; node != NULL; node = node->next)
    {
	Attribute* attr = node->data;
	filler(buf, attr->path, &(attr->st), 0, FUSE_FILL_DIR_PLUS);
    }
    freeList(attrs, freeAttr);

    return 0;    
}

void fuseDestory(void *private_data)
{
    FsData* fs;

    fs = private_data;
    freeIntMap(fs->FhMap);
    free(fs);
}

off_t fuseLseek(const char *path, off_t offset, int whence, struct fuse_file_info *fi)
{
    IntMap* FhMap;
    FileHandler* fh;
    int rc;

    //ファイルハンドラマップを取得
    FhMap = getFhMap();

    fh = getIntMap(FhMap, fi->fh);
    if(fh == NULL)
    {
	return -EBADFD;
    }
   
    fh->offset = offset;
    fh->session->offset = offset;
    return fh->offset;
}

struct fuse_operations fuseOper = 
{
    .getattr = fuseGetattr,
    .open = fuseOpen,
    .read = fuseRead,
    .write = fuseWrite,
    //.statfs = fuseStatfs,
    .release = fuseRelease,
    .readdir = fuseReaddir,
    .init = fuseInit,
    .destroy = fuseDestory,
    .lseek = fuseLseek
};

int main(int argc, char* argv[])
{
    Args* args;
    int i, new_argc;
    char* new_argv[10];

    if(argc < 4)
    {
	printf("RemoteFs [mountpoint] [remoteroot] [ssh.config]\n");
	exit(EXIT_FAILURE);
    }

    //Argsの取得
    args = getArgs(argv[2], argv[3]);

    for (i=0, new_argc=0; (i<argc) && (new_argc<10); i++)
    {
	if((i != 2) & (i != 3))
	{
	    printf("%s\n",argv[i]);
	    new_argv[new_argc++] = argv[i];
	}
    }

    return fuse_main(new_argc, new_argv, &fuseOper, NULL);
}
