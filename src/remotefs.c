#include "remotefs.h"
#include "conn.h"
#include "list.h"
#include "strmap.h"
#include "attribute.h"
#include <stdio.h>
#include <fcntl.h>



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
