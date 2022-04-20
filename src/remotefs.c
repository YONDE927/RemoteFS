#include "remotefs.h"
#include "conn.h"
#include "list.h"
#include "strmap.h"
#include "entry.h"
#include <fuse3/fuse.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

static List* filehandlers;

typedef struct OpenFile
{
    int fh;
    off_t offset;
}OpenFile;

int fuseGetattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
    Attribute* attr;
    attr = connStat(path);
    if(attr == NULL)
    {
	printf("connStat error\n");
	return -ENOENT;
    }
    *stbuf = attr->st;
    return 0;
}

int fuseOpen(const char *path, struct fuse_file_info *fi)
{
    int fhs_size;
    int ind,fh;
    OpenFile file;

    //list opened file handers
    fhs_size = length(filehandlers);
    int* fhs = malloc(sizeof(int) * fhs_size);
    ind = 0;
    for(Node* node = filehandlers->head; node != NULL; node = node->next)
    {
	fh = *(int*)(node->data);
	fhs[ind] = fh;
	ind++;
    }
    //search new file handler
    for(int i = 0; ; i++)
    {
	int isNew = 1;
	for(int j = 0; j < fhs_size; j++)
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
    //free
    free(fhs);
    //add file handler and file info to filehandlers
    file.offset = 0;
    file.fh = fh;
    push_back(filehandlers, &file, sizeof(OpenFile)); 
    return 0;
}

struct fuse_operations fuseOper = 
{
    .getattr = fuseGetattr,
    .open = fuseOpen,
    .read = fuseRead,
    .write = fuseWrite,
    .statfs = fuseStatfs,
    .release = fuseRelease,
    .readdir = fuseReaddir,
    .init = fuseInit,
    .destroy = fuseDestory,
    .lseek = fuseLseek
};

int main(int argc, char* argv[])
{
    return fuse_main(argc, argv, &fuseOper, NULL);
}
