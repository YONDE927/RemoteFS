#include "attribute.h"

Attribute* newAttr(int pathlen)
{
    Attribute* attr;
    attr = malloc(sizeof(Attribute));
    *attr = (Attribute){0};
    attr->path = malloc(sizeof(char) * pathlen);
    return attr;
}

void freeAttr(Attribute* attr)
{
    free(attr->path);
    free(attr);
}



