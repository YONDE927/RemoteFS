#include "entry.h"
#include <stdio.h>

Attribute* newAttr(int pathlen)
{
    Attribute* attr;
    attr = malloc(sizeof(Attribute));
    *attr = (Attribute){0};
    return attr;
}

void freeAttr(void* attr)
{
    free((Attribute*)attr);
}

void printAttr(void* _dstat){
    if(_dstat == NULL){
        return;
    }
    struct Attribute* dstat = _dstat;
    printf("%s %ld %ld %ld \n", dstat->path, dstat->st.st_size, dstat->st.st_mtime, dstat->st.st_ctime);
}



