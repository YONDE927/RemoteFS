#include "attribute.h"

Attribute* newAttr(int pathlen)
{
    Attribute* attr;
    attr = malloc(sizeof(Attribute));
    *attr = (Attribute){0};
    attr->path = malloc(sizeof(char) * pathlen);
    return attr;
}

void freeAttr(void* attr)
{
    free(((Attribute*)attr)->path);
    free((Attribute*)attr);
}



