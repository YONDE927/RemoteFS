#pragma once

#include <sys/stat.h>
#include <stdlib.h>

typedef struct Attribute
{
    struct stat st;
    char* path;
} Attribute;

Attribute* newAttr(int pathlen);
void freeAttr(Attribute* attr);

