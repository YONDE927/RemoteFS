#pragma once

#include <sys/stat.h>
#include <stdlib.h>

typedef struct Attribute
{
    char path[256];
    struct stat st;
} Attribute;

Attribute* newAttr(int pathlen);
void freeAttr(void* attr);

void printdirstat(void* _dstat);

